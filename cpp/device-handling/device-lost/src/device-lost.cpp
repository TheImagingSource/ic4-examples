
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <vector>

#include <ic4/ic4.h>

void device_lost_handler(ic4::Grabber& grabber)
{
	std::cout << "Device lost!" << std::endl;
}

auto select_from_list(const std::vector<ic4::DeviceInfo>& device_list)
{
	if (device_list.empty())
	{
		std::cout << "No devices found" << std::endl;
		return device_list.end();
	}

	std::cout << "Select device:" << std::endl;

	for (size_t i = 0; i < device_list.size(); ++i)
	{
		auto model_name = device_list[i].getModelName();
		auto serial = device_list[i].getSerial();
		auto itf = device_list[i].getInterface();
		auto transport_layer_name = itf.getTransportLayerName();

		std::cout << "[" << i << "] ";
		std::cout << model_name << " (" << serial << ", " << transport_layer_name << ")";
		std::cout << std::endl;
	}

	std::cout << "Your choice: ";
	std::string input;
	std::getline(std::cin, input);

	try
	{
		size_t index = std::stoull(input);

		if (index >= device_list.size())
		{
			std::cout << "Invalid index" << std::endl;
			return device_list.end();
		}

		return std::next(device_list.begin(), index);
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return device_list.end();
	}
}

int main()
{
	ic4::InitLibrary();
	std::atexit(ic4::ExitLibrary);

	auto device_list = ic4::DeviceEnum::getDevices();
	auto it = select_from_list(device_list);
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