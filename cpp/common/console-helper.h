	
#include <vector>
#include <iostream>
#include <cstdlib>
#include <string>

#include <ic4/ic4.h>

namespace ic4_examples
{
	namespace console
	{
		inline std::vector<ic4::DeviceInfo>::const_iterator select_from_list(const std::vector<ic4::DeviceInfo>& device_list)
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
	}
}
