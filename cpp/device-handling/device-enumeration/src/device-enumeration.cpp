
#include <iostream>
#include <cstdlib>
#include <string>

#include <ic4/ic4.h>

std::string format_device_info(const ic4::DeviceInfo& device_info)
{
	return "Model: " + device_info.modelName() + " Serial: " + device_info.serial() + " Version: " + device_info.version();
}

void print_device_list()
{
	std::cout << "Enumerating all attached video capture devices in a single list..." << std::endl;

	auto device_list = ic4::DeviceEnum::enumDevices();

	if (device_list.empty())
	{
		std::cout << "No devices found" << std::endl;
		return;
	}

	std::cout << "Found " << device_list.size() << " devices:" << std::endl;

	for (auto&& dev_info : device_list)
	{
		std::cout << "\t" <<  format_device_info(dev_info) << std::endl;
	}
	
	std::cout << std::endl;
}

inline const char* toString(ic4::TransportLayerType val) noexcept
{
	switch (val)
	{
	case ic4::TransportLayerType::Unknown:      return "Unknown";
	case ic4::TransportLayerType::GigEVision:   return "GigEVision";
	case ic4::TransportLayerType::USB3Vision:   return "USB3Vision";
	default:
		return "";
	}
}

void print_interface_device_tree()
{
	std::cout << "Enumerating video capture devices by interface..." << std::endl;

	auto interface_list = ic4::DeviceEnum::enumInterfaces();

	for (auto&& itf : interface_list)
	{
		std::cout << "Interface: " << itf.interfaceDisplayName() << std::endl;
		std::cout << "\tProvided by " << itf.transportLayerName() << " [TLType: " << toString(itf.transportLayerType()) << "]" << std::endl;

		auto device_list = itf.enumDevices();

		if (device_list.empty())
		{
			std::cout << "\tNo devices found" << std::endl;
			continue;
		}

		std::cout << "\tFound " << device_list.size() << " devices:" << std::endl;

		for (auto&& dev_info : device_list)
		{
			std::cout << "\t\t" << format_device_info(dev_info) << std::endl;
		}
	}

	std::cout << std::endl;
}

int main()
{
	ic4::initLibrary();
	std::atexit(ic4::exitLibrary);

	print_device_list();
	print_interface_device_tree();

	return 0;
}