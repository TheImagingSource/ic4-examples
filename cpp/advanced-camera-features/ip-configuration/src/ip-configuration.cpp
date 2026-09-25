
#include <ic4/ic4.h>

#include <iostream>
#include <exception>

// This program shows how to configure the IP Address and subnet mask of a GigE Vision camera.
// 
// Please note: There is a pre-compiled program ic4-ipconfig_cmd.exe that is distributed with the
// The Imaging Source GenTL Producer for GigE Vision Cameras.
// That utitily can do everything this example shows, but also recover devices that are misconfigured.
//
// The IP Configuration of the camera is accessed through the driver's property map obtained
// from Grabber::driverPropertyMap():
// - Setting GevDeviceIPConfigPersistentIPEnable to true sets the camera to a fixed IP address
// - Setting GevDeviceIPConfigDHCPEnable to true enables the camera to perform an IP address via DHCP
// - If both are not set, the camera uses Link-Local Auto-IP (LLA)
// If GevDeviceIPConfigPersistentIPEnable is set, the IP configuration is specified by 3 more parameters:
// - GevDeviceIPConfigPersistentIPAddress is the persistent IP address
// - GevDeviceIPConfigPersistentSubnetMask is the subnet mask
// - GevDeviceIPConfigPersistentGateway is the default gateway
// Writing to any of these features has no effect until GevDeviceIPConfigApply is executed.

// Print help on how to use this program
void print_usage()
{
	std::cout << "Usage:" << std::endl;
	std::cout << "ip-configuration <IDENTIFIER> [<IP> [<SUBNET> [GATEWAY]]]" << std::endl;

	std::cout << "\t<IDENTIFIER> is the identifier of a connected GigE Vision camera, e.g. its" << std::endl;
	std::cout << "\t\tSerial Number" << std::endl;
	std::cout << "\t\tIP Address" << std::endl;
	std::cout << "\t\tModel Name (if currently unique)" << std::endl;

	std::cout << "\t<IP> is either" << std::endl;
	std::cout << "\t\t'auto' to use Link-Local Auto-IP (LLA)" << std::endl;
	std::cout << "\t\t'dhcp' to enable DHCP" << std::endl;
	std::cout << "\t\tan IP Address in the format 192.168.0.27 that is configured to be used persistently" << std::endl;
	std::cout << "\t\tnot present -- in this case the program prints the current IP configuration of <IDENTIFIER>" << std::endl;

	std::cout << "\t<SUBNET> is the subnet mask to be used if <IP> is an IP Address. (Optional, default is 0.0.0.0)" << std::endl;
	std::cout << "\t<GATEWAY> is the default gateway to configure if <IP> is an IP Address. (Optional, default is 0.0.0.0)" << std::endl;
}

int main(int argc, char* argv[])
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
		if (argc < 2 || argc > 5)
		{
			print_usage();
		}
		else
		{
			ic4::Grabber grabber;

			// Open the device specified by <IDENTIFIER>
			grabber.deviceOpen(argv[1]);

			// Query device info to get access to later print model name and serial number
			auto info = grabber.deviceInfo();

			// IP Configuration is accessed via the driver's property map
			auto map = grabber.driverPropertyMap();

			if (argc == 2)
			{
				std::cout << "Current IP Configuration for device " << info.modelName() << " (" << info.serial() << ")" << std::endl;

				std::cout << "IP Address: " << map.getValueString("GevDeviceIPAddress") << std::endl;
				std::cout << "Subnet Mask: " << map.getValueString("GevDeviceSubnetMask") << std::endl;
				std::cout << "Default Gateway: " << map.getValueString("GevDeviceGateway") << std::endl;
			}
			else
			{
				std::string ip = argv[2];
				if (ip == "auto")
				{
					// Disable both PersistentIP and DHCP modes, so that LLA remains
					map.setValue("GevDeviceIPConfigPersistentIPEnable", false);
					map.setValue("GevDeviceIPConfigDHCPEnable", false);
				}
				else if (ip == "dhcp")
				{
					// Disable PersistentIP and enable DHCP
					map.setValue("GevDeviceIPConfigPersistentIPEnable", false);
					map.setValue("GevDeviceIPConfigDHCPEnable", true);
				}
				else
				{
					// Disable PersistentIP mode
					map.setValue("GevDeviceIPConfigPersistentIPEnable", true);

					// Set the IP address
					map.setValue("GevDeviceIPConfigPersistentIPAddress", ip);

					// Set the subnet mask
					if (argc > 3)
					{
						map.setValue("GevDeviceIPConfigPersistentSubnetMask", argv[3]);
					}
					else
					{
						// This default may be wrong, depending on the value of 'ip'.
						// Sanity checks and automated generation are omitted for clarity.
						map.setValue("GevDeviceIPConfigPersistentSubnetMask", "255.255.255.0");
					}

					// Set the default gateway
					if (argc > 4)
					{
						map.setValue("GevDeviceIPConfigPersistentGateway", argv[4]);
					}
					else
					{
						map.setValue("GevDeviceIPConfigPersistentGateway", "0.0.0.0");
					}
				}

				// Let the settings go active as configured
				map.executeCommand("GevDeviceIPConfigApply");

				// Print summary of what was configured
				std::cout << "Device " << info.modelName() << " (" << info.serial() << ") was reconfigured." << std::endl;
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}