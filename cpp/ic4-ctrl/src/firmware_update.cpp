
#include "firmware_update.h"
#include "ic4-ctrl-helper.h"
#include "ic4_enum_to_string.h"

#include "print_property.h"
#include <fmt/core.h>
#include <iostream>
#include <condition_variable>

using namespace ic4_helper;

namespace
{

bool user_confirms_action()
{
    fmt::println("!!! Attention !!!");
    fmt::println("This action could break your camera.\n");

    fmt::println("Do you really want to proceed? [y/N] ");

    while (true)
    {
        std::string s;
        std::getline(std::cin, s);
        if (s.compare("y") == 0 || s.compare("Y") == 0)
        {
            return true;
        }
        else if (s.empty() || s.compare("n") == 0 || s.compare("N") == 0)
        {
            fmt::println("Aborting...");
            return false;
        }
        else
        {
            fmt::println("Please answer yes or no.");
        }
    }
}


bool set_user_model_overwrite(ic4::PropertyMap& prop_map, const std::string& model_overwrite)
{
    ic4::Error err;
    auto prop_model_selector = prop_map.find("FirmwarePackageModelsSelector").asInteger(err);

    bool is_valid_model = false;

    for (int index = prop_model_selector.minimum(err);
         index <= prop_model_selector.maximum(err);
         index += prop_model_selector.increment(err))
    {
        prop_model_selector.setValue(index, err);
        std::string models = prop_map.getValueString("FirmwarePackageModels", err);

        if (models == model_overwrite)
        {
            is_valid_model = true;
            break;
        }
    }
    
    if (is_valid_model)
    {
        if (!prop_map.setValue("FirmwareUpdateModelOverride", model_overwrite, err))
        {
            fmt::println("Error while setting model overwrite: {}", err.message());
            return false;
        }
        return true;
    }
    else
    {
        fmt::println("The given model override does not exist in the firmware file: \"{}\"", model_overwrite);
        return false;
    }

}


// convenience helper
// helps us remember serial <-> selector association
struct dev_info
{
    ic4::DeviceInfo dev;
    int selector;
};

std::vector<dev_info> verify_devices(std::vector<ic4::DeviceInfo>& device_list)
{
    ic4::Error err;
    std::vector<dev_info> devices;
    for (auto &dev : device_list)
    {
        ic4::Interface itf = dev.getInterface(err);

        if (!itf.is_valid())
        {
            fmt::print("Interface for device \"{}\" is not valid.", dev.serial());
            return {};
        }

        auto prop_map = itf.interfacePropertyMap();

        auto selector = prop_map.find("DeviceSelector").asInteger(err);

        bool device_found = false;

        for(int index = selector.minimum(err);
            index <= selector.maximum(err);
            index += selector.increment(err))
        {
            selector.setValue(index, err);

            if (prop_map.getValueString("DeviceSerialNumber") == dev.serial())
            {
                // device is valid
                devices.push_back({dev, index});
                device_found = true;
                break;
            }
        }
        if (!device_found)
        {
            fmt::println("Unable to find device {}", dev.serial());
            return {};
        }
    }
    return devices;
}


} // namespace



void ic4ctrl::write_firmware_update(const std::string& firmware_file,
                                    const std::string& model_overwrite,
                                    bool assume_yes,
                                    std::vector<ic4::DeviceInfo>& device_list)
{
    // Find all devices before writing anything
    // Don't stop in the middle of 10 devices because no5 does not exist

    std::vector<dev_info> devices = verify_devices(device_list);
    if (devices.empty())
    {
        return;
    }

    for (const auto& d : devices)
    {
        fmt::println("Writing firmware to device {} ...", d.dev.serial());
        ic4::Error err;

        ic4::Interface itf = d.dev.getInterface(err);
        auto prop_map = itf.interfacePropertyMap(err);

        if (!assume_yes)
        {
            if (!user_confirms_action())
            {
                return;
            }
        }

        prop_map.setValue("DeviceSelector", d.selector, err);

        if (!prop_map.setValue("FirmwareUpdatePackageFile", firmware_file, err))
        {
            fmt::println("Unable to use firmware file: {}", err.message());
            return;
        }

        if (!model_overwrite.empty())
        {
            if (!set_user_model_overwrite(prop_map, model_overwrite))
            {
                return;
            }
        }

        std::mutex mtx;
        std::condition_variable wait_var;
		bool done = false;

        // register callbacks for what the firmware update is doing

        auto cb_percent = [&] (ic4::Property& prop) -> void
        {
            int value = prop.asInteger().getValue();

            // lock prints in case multiple callbacks 
            // come in on different threads
            std::lock_guard guard(mtx);
            // Reuse percentage lines
            fmt::print("{} %\r", value);
            fflush(stdout);

        };
        auto percent_prop = prop_map.findInteger("FirmwareUpdateProgressPercent");
        auto percent_token = percent_prop.eventAddNotification(cb_percent, err);


        auto status_prop = prop_map.findString("FirmwareUpdateProgress");
        std::string last_progress;
        auto cb = [&] (ic4::Property& prop) -> void
        {
            std::string value = prop.asString().getValue();

            if (value == last_progress)
            {
                return;
            }

            last_progress = value;
            std::lock_guard guard(mtx);
            fmt::println("{}", value);
        };
        auto status_token = status_prop.eventAddNotification(cb);

        auto state_prop = prop_map.find("FirmwareUpdateState");
        std::string last_state = "NotStarted";
        auto state_cb = [&] (ic4::Property& prop) -> void
        {
            auto state = prop.asEnumeration().getValue();

            if (state == last_state)
            {
                return;
            }
            last_state = state;
            std::lock_guard guard(mtx);
            if (state == "NotStarted") {}	// not yet started
            else if (state == "Running") {}	// currently running
			else if (state == "Success")
			{
				fmt::println("Firmware update was a success!");
				fmt::println("Please reconnect the camera.");

				done = true;
				wait_var.notify_all();
			}
            else
            {
				fmt::println("Firmware update encountered an error!");
				if (state == "ErrorUnsupportedFirmwarePackage")
				{
					fmt::println("The device driver does not support the supplied firmware package.");
				}
				else if (state == "ErrorModelNotSupportedByPackage")
				{
					fmt::println("Supplied firmware package does not contain firmware for this device/model.");
				}
				else
				{
					fmt::println("  Error: {}.", prop.asEnumeration().selectedEntry().description());
					fmt::println("The camera may be in an undefined state!");
					fmt::println("Please try again or contact support!");
				}

				done = true;
				wait_var.notify_all();
            }
        };
        auto state_token = state_prop.eventAddNotification(state_cb);

        auto upload_cmd = prop_map.find("FirmwareUpdateUpload").asCommand();

        if (!upload_cmd.execute(err))
        {
            fmt::println("Unable to start firmware update: {}", err.message());
            return;
        }

		{
			std::unique_lock lock(mtx);
			// wait until firmware update is done
			wait_var.wait(lock, [&done] { return done; });
		}

        // cleanup callbacks
        state_prop.eventRemoveNotification(state_token, err);
        status_prop.eventRemoveNotification(status_token, err);
        percent_prop.eventRemoveNotification(percent_token, err);
    }

}
