
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

#include <ic4/ic4.h>

#include <console-helper.h>

/*
 * This example program shows how to utilize the device-lost event on the Grabber object to
 * receive a notification in case the connection to the currently opened video capture device
 * is interrupted.
 * 
 * It then re-enumerates the connected devices until the previously opened camera is found
 * and opens it again.
 */

std::atomic<bool> g_abort_reconnect(false);
std::string g_device_unique_name;

static void device_lost_handler(ic4::Grabber& grabber)
{
	std::cout << "Device lost!" << std::endl;
	std::cout << "Waiting for device to be reconnected..." << std::endl;

	// We need to formally close the lost device before trying to open it again
	grabber.deviceClose();

	while (!g_abort_reconnect)
	{
		// Re-enumerate devices
		auto devices = ic4::DeviceEnum::enumDevices();

		// Check whether one of them matches the unique name of the device we are looking for
		auto it = std::find_if(devices.begin(), devices.end(),
			[](auto& deviceInfo)
			{
				return deviceInfo.uniqueName() == g_device_unique_name;
			}
		);

		if (it != devices.end())
		{
			// If it was found, try to open it
			ic4::Error err;
			if (grabber.deviceOpen(*it, err))
			{
				std::cout << "Device re-opened successfully." << std::endl << std::endl;
				return;
			}
			else
			{
				std::cout << "Failed to re-open device: " << err.message() << std::endl;
				return;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

int main()
{
	ic4::initLibrary();
	std::atexit(ic4::exitLibrary);

	// Let the user select a connected device
	auto device_list = ic4::DeviceEnum::enumDevices();
	auto it = ic4_examples::console::select_from_list(device_list);
	if (it == device_list.end())
	{
		return -1;
	}

	// Create a Grabber object opening the selected device
	ic4::Grabber grabber(*it);

	// Register device-lost event handler
	auto token = grabber.eventAddDeviceLost(device_lost_handler);

	// Query information about the opened device
	auto device_info = grabber.deviceInfo();

	// Remember device unique name to reopen later
	g_device_unique_name = device_info.uniqueName();

	std::cout << "Opened device " << device_info.modelName() << " (" << device_info.serial() << ")" << std::endl;
	std::cout << "Disconnect device to produce device-lost event" << std::endl;

	std::cout << "Press ENTER to exit program" << std::endl;
	std::cout << std::endl;

	// Wait for ENTER key
	int ch = std::getchar();

	// Let the device-lost handler know to stop looking
	g_abort_reconnect = true;

	// Only for completeness. Technically this is not necessary here, since the grabber is destroyed at the end of the function.
	grabber.eventRemoveDeviceLost(token);

	return 0;
}
