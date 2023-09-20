
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <vector>

#include <ic4/ic4.h>

#include <console-helper.h>

static void device_lost_handler(ic4::Grabber& grabber)
{
	std::cout << "Device lost!" << std::endl;
}

int main()
{
	ic4::initLibrary();
	std::atexit(ic4::exitLibrary);

	auto device_list = ic4::DeviceEnum::getDevices();
	auto it = ic4_examples::console::select_from_list(device_list);
	if (it == device_list.end())
	{
		return -1;
	}

	ic4::Grabber grabber;
	grabber.deviceOpen(*it);

	auto token = grabber.eventAddDeviceLost(device_lost_handler);

	std::cout << "Opened device " << grabber.deviceInfo().getModelName() << " (" << grabber.deviceInfo().getSerial() << ")" << std::endl;
	std::cout << "Disconnect device to produce device-lost event" << std::endl;

	std::cout << "Press ENTER to exit program" << std::endl;
	std::cout << std::endl;

	int ch = std::getchar();

	// Only for completeness. Technically this is not necessary here, since the grabber is destroyed at the end of the function.
	grabber.eventRemoveDeviceLost(token);

	return 0;
}