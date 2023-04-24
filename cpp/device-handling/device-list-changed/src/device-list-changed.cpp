
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>>

#include <ic4/ic4.h>

void device_list_changed_handler(ic4::DeviceEnum& enumerator)
{
	auto device_list = ic4::DeviceEnum::getDevices();

	auto new_device_count = device_list.size();

	std::cout << "Device list has changed!" << std::endl;
	std::cout << "Found " << new_device_count << " devices" << std::endl;
	std::cout << std::endl;
}

int main()
{
	ic4::InitLibrary();
	std::atexit(ic4::ExitLibrary);

	ic4::DeviceEnum enumerator;
	auto token = enumerator.eventAddDeviceListChanged(device_list_changed_handler);

	auto initial_device_count = ic4::DeviceEnum::getDevices().size();

	std::cout << "Press ENTER to exit program" << std::endl;
	std::cout << initial_device_count << " devices connected initially." << std::endl;
	std::cout << std::endl;

	(void)std::getchar();

	// This is technically not necessary, since the DeviceEnum object is destroyed at the end of this function.
	enumerator.eventRemoveDeviceListChanged(token);

	return 0;
}