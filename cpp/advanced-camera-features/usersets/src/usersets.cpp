
#include <ic4/ic4.h>

#include <console-helper.h>

#include <thread>
#include <chrono>
#include <cassert>

/*
 * This example program demonstrates the use of the UserSet features present in most modern cameras.
 * 
 * UserSets provide a mechanism to store the camera configuration in the camera's non-volatile memory.
 * 
 * Usually, a camera provides at least two 'slots' to save data into, named 'UserSet1' and 'UserSet2'.
 * 
 * To save the camera configuration into one of the slots, first set UserSetSelector to the identifier of the slot to operate on.
 * Then, execute the UserSetSave command to store the current camera configuration into that slot.
 * 
 * To later restore the camera configuration, first set UserSetSelector to the identifier of the slot to read from.
 * Then, execute the UserSetLoad command.
 * 
 * There is a special 'Default' slot that contains the camera's default configuration that can be used to
 * restore the device into a known-good state. The UserSetSave operation is not available for the 'Default' slot.
 * 
 * The UserSetDefault feature can be used to specify a slot that is loaded automatically when the camera is powered on
 * (or rebooted using DeviceReset).
 */


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
	// Automatically call exitLibrary when returning from main
	std::atexit(ic4::exitLibrary);

	try
	{
		// Let the user select a camera
		auto device_list = ic4::DeviceEnum::enumDevices();
		auto it = ic4_examples::console::select_from_list(device_list);
		if (it == device_list.end())
		{
			return -1;
		}

		// Open the device in a new Grabber instance
		ic4::Grabber grabber(*it);

		// Access the device's property map
		auto map = grabber.devicePropertyMap();

		// Make sure the camera supports the UserSet features
		ic4::Error err;
		auto selector = map.find(ic4::PropId::UserSetSelector, err);
		if (err.isError())
		{
			std::cerr << "Failed to find UserSetSelector property. The device likely does not have UserSet support.";
			return -2;
		}

		// Execute UserSetLoad[Default] to reset the camera to its default state
		map.setValue(ic4::PropId::UserSetSelector, "Default");
		map.executeCommand(ic4::PropId::UserSetLoad);

		// Query some property info for demonstration purposes
		auto default_height = map[ic4::PropId::Height].getValue();
		auto min_height = map[ic4::PropId::Height].minimum();

		// Modify a property for demonstration purposes
		map.setValue(ic4::PropId::Height, min_height);

		// Verify Height was changed to min_height
		assert(map.getValueInt64(ic4::PropId::Height) == min_height);

		// Execute UserSetSave[UserSet1] to store the camera configuration where Height==min_height
		map.setValue(ic4::PropId::UserSetSelector, "UserSet1");
		map.executeCommand(ic4::PropId::UserSetSave);

		// Load default UserSet again
		map.setValue(ic4::PropId::UserSetSelector, "Default");
		map.executeCommand(ic4::PropId::UserSetLoad);

		// Verify Height was restored to default_height
		assert(map.getValueInt64(ic4::PropId::Height) == default_height);

		// Load UserSet1 again
		map.setValue(ic4::PropId::UserSetSelector, "UserSet1");
		map.executeCommand(ic4::PropId::UserSetLoad);

		// Verify Height was restored to min_height
		assert(map.getValueInt64(ic4::PropId::Height) == min_height);

		// Set UserSetDefault so that the camera always loads UserSet1 at startup
		// Please note: Applications (such as ic4-demoapp and IC Capture 4) can have
		// their own mechanism to restore camera state, which can hide the effect of UserSetDefault.
		map.setValue(ic4::PropId::UserSetDefault, "UserSet1");

		// Reboot the camera
		map.executeCommand(ic4::PropId::DeviceReset);

		// We have to call deviceClose before deviceOpen
		grabber.deviceClose();

		// Try to re-discover the device after reset
		// Please note: We cannot use grabber.deviceOpen(*it) here, because *it refers
		// to the previously-discovered device instance.
		std::cout << "Waiting for the camera to re-appear after DeviceReset..." << std::endl;
		using TClock = std::chrono::steady_clock;
		auto wait_timeout = TClock::now() + std::chrono::seconds(60);
		while (!grabber.deviceOpen(it->uniqueName(), err))
		{
			if (TClock::now() > wait_timeout)
			{
				std::cerr << "Failed to discover device after DeviceReset" << std::endl;
				return -3;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		// Access the property map for the re-opened device
		map = grabber.devicePropertyMap();

		// Verify Height was initialized to min_height
		assert(map.getValueInt64(ic4::PropId::Height) == min_height);

		// Revert camera startup behavior back to default
		map.setValue(ic4::PropId::UserSetDefault, "Default");
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}