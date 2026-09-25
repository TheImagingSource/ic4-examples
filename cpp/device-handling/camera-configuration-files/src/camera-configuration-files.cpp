
#include <ic4/ic4.h>

#include <console-helper.h>

#include <iostream>
#include <cstdlib>
#include <cstdint>

/* This program demonstrate how to save and load device settings to and from configuration files and memory buffers.
 * 
 * The example is split into 2 sections, demonstrating different modes of device settings serialization:
 * 
 * Section 1 demonstrates how to store the Grabber's device state and reopen the device from the state file:
 * - Save the Grabber's device state to a settings file
 * - Modify a device property (Width)
 * - Close the device
 * - Reopen the device using the settings file, restoring the original Width value
 * 
 * The settings file contains both a device identifier and the device settings, so that the device can be reopened and its
 * settings restored.
 * There are function overloads to store the Grabber's device state into a memory buffer instead.
 * 
 * In section 2, only the settings inside the device's PropertyMap are saved and restored:
 * - Save the device's PropertyMap into a memory buffer
 * - Modify a device property (Width)
 * - Restore the device PropertyMap's state from the memory buffer
 * 
 * The settings buffer only contains the PropertyMap's settings.
 * Therefore, this approach can be used to transfer camera settings from one device to another.
 * There are function overloads to store the PropertyMap's state into a file instead.
 */

// Helper function to print info about the device opened in the passed Grabber
void print_device_info(const char* label, ic4::Grabber& grabber)
{
	if (grabber.isDeviceValid())
	{
		auto info = grabber.deviceInfo();
		auto map = grabber.devicePropertyMap();

		auto model_name = info.modelName();
		auto width = map.getValueInt64(ic4::PropId::Width);

		std::cout << label << " -- " << model_name << " -- Width: " << width << std::endl;
	}
	else
	{
		std::cout << label << " -- No Device Opened" << std::endl;
	}
}

int main()
{
	// Initialize the library with sensible defaults:
	// - Throw exceptions on errors
	// - Log errors and warnings from API calls
	// - Log to stdout and Windows debug log
	ic4::InitLibraryConfig libraryConfig =
	{
		ic4::ErrorHandlerBehavior::Throw,
		ic4::LogLevel::Warning,
		ic4::LogLevel::Off,
		ic4::LogTarget::StdOut | ic4::LogTarget::WinDebug
	};
	ic4::initLibrary(libraryConfig);
	std::atexit(ic4::exitLibrary);

	// Let the user select a camea
	auto device_list = ic4::DeviceEnum::enumDevices();
	auto it = ic4_examples::console::select_from_list(device_list);
	if (it == device_list.end())
	{
		return -1;
	}

	try
	{
		ic4::Grabber grabber;
		print_device_info("Before opening device", grabber);

		// Open the selected device
		grabber.deviceOpen(*it);
		print_device_info("After opening device", grabber);

		// Reset camera settings to default by executing UserSetLoad[Default]
		auto map = grabber.devicePropertyMap();
		map.setValue(ic4::PropId::UserSetSelector, "Default");
		map.executeCommand(ic4::PropId::UserSetLoad);
		print_device_info("After resetting device to defaults", grabber);


		// Section 1: Save Grabber's device state into a file and restore it
		const char* DEVICE_STATE_FILE_NAME = "settings.json";

		// Save device state
		grabber.deviceSaveState(DEVICE_STATE_FILE_NAME);
		std::cout << ">>> Saved device state into " << DEVICE_STATE_FILE_NAME << std::endl;

		// Modify a property for demonstration
		map.setValueMinimum(ic4::PropId::Width);
		print_device_info("After setting Width to minimum", grabber);

		// Close the device
		grabber.deviceClose();
		print_device_info("After closing device", grabber);

		// Reopen using device state file
		grabber.deviceOpenFromState(DEVICE_STATE_FILE_NAME);
		print_device_info("After opening device from file", grabber);


		// Section 2: Save PropertyMap's state into a memory buffer and restore it

		// map has become invalid after the device was closed above
		map = grabber.devicePropertyMap();

		// Save PropertyMap's state into a buffer
		std::vector<uint8_t> buffer;
		map.serialize(buffer);
		std::cout << ">>> Saved property map state into memory buffer" << std::endl;

		// Modify a property for demonstration
		map.setValueMinimum(ic4::PropId::Width);
		print_device_info("After setting Width to minimum", grabber);

		// Restore PropertyMap state from buffer
		map.deSerialize(buffer);
		std::cout << ">>> Restored property map state from memory buffer" << std::endl;
		print_device_info("After deserializing property map state", grabber);

		return 0;
	}
	catch (const ic4::IC4Exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}
