
#include <ic4/ic4.h>

#include <console-helper.h>

#include <thread>
#include <chrono>
#include <regex>

/*
 * This example program demonstrates how to set up multicast image transmission using GigE Vision cameras
 * 
 * When operating in multicast mode, one client 'owns' the camera and can control it, while other clients
 * can subscribe to the multicast group and receive a copy of the image data stream.
 */

// Multicast only works with GigE Vision devices
std::vector<ic4::DeviceInfo> enum_gige_devices()
{
	std::vector<ic4::DeviceInfo> result;

	for (auto&& dev : ic4::DeviceEnum::enumDevices())
	{
		if (dev.getInterface().transportLayerType() != ic4::TransportLayerType::GigEVision)
		{
			continue;
		}

		auto serial = dev.serial();
		if (serial.substr(serial.size() - 8) == "-Monitor")
		{
			continue;
		}

		result.push_back(dev);
	}

	return result;
}

bool checkFirmwareVersion(ic4::Grabber& grabber)
{
	auto device_version = grabber.devicePropertyMap().getValueString(ic4::PropId::DeviceVersion);

	auto pattern = std::regex::basic_regex("Rev (\\d+)");
	std::smatch match;

	if (std::regex_search(device_version, match, pattern))
	{
		int revision = std::stoi(match[1].str());

		if (revision < 4000)
		{
			// GigE3 cameras need at least version 3173
			return revision >= 3173;
		}
		else
		{
			// GigE4 cameras need at least version 4380
			return revision >= 4380;
		}
	}

	std::cerr << "Failed to extract firmware revision from DeviceVersion '" << device_version << "'" << std::endl;

	return false;
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

#ifdef IC4_MULTICAST_MASTER
	std::cout << "Started in master mode." << std::endl;
#elif defined IC4_MULTICAST_MONITOR
	std::cout << "Started in monitor mode." << std::endl;
#else
#error Either IC4_MULTICAST_MASTER or IC4_MULTICAST_MONITOR has to be defined
#endif

	auto device_list = enum_gige_devices();
	auto it = ic4_examples::console::select_from_list(device_list);
	if (it == device_list.end())
	{
		return -1;
	}

	// Remember the selected device for later
	ic4::DeviceInfo dev = *it;

	ic4::Grabber grabber;

	auto print_frames_sink = ic4::QueueSink::create(
		[](ic4::QueueSink& qs)
		{
			auto buffer = qs.popOutputBuffer();

			std::cout << "Received frame " << buffer->metaData().device_frame_number << std::endl;
		}
	);

	try
	{
#ifdef IC4_MULTICAST_MASTER
		grabber.deviceOpen(dev);

		// Make sure the device supports multicast
		if (!checkFirmwareVersion(grabber))
		{
			std::cerr << "Firmware version does not support multicast operation. Please contact support." << std::endl;
			return -2;
		}

		// For multicast operation, the master has to manually configure stream options
		//´on both the device's and driver's property maps:
		auto driver_map = grabber.driverPropertyMap();
		auto device_map = grabber.devicePropertyMap();

		// Disable packet-size auto-negotiation and set it to fixed 1500
		driver_map.setValue("GevStreamNegotiatePacketSize", false);
		device_map.setValue("GevSCPSPacketSize", 1500);

		// Override destination address:port data with multicast address
		driver_map.setValue("GevStreamDestinationOverride", true);
		driver_map.setValue("GevStreamDestinationAddress", "225.0.0.1");
		driver_map.setValue("GevStreamDestinationPort", "50002");

		std::cout << "Starting master stream. Press ENTER to quit." << std::endl;

		// Start a stream, printing information about received images
		grabber.streamSetup(print_frames_sink);

		// Wait for ENTER key
		std::cin.get();

#elif defined IC4_MULTICAST_MONITOR

		// Activate generation monitor device entries on the interface of the selected device
		auto itf = dev.getInterface();
		auto itf_map = itf.interfacePropertyMap();
		itf_map.setValue("SpawnMonitorDevices", true);

		try
		{
			// The serial number of the monitor device instance has "-Monitor" appended to it
			auto monitor_serial = dev.serial() + "-Monitor";

			// Open the monitor device
			grabber.deviceOpen(monitor_serial);

			std::cout << "Starting monitor stream. Press ENTER to quit." << std::endl;

			// Start a stream, printing information about received images
			grabber.streamSetup(print_frames_sink);

			// Wait for ENTER key
			std::cin.get();

			// Disable monitor devices again
			itf_map.setValue("SpawnMonitorDevices", false);
		}
		catch(const ic4::IC4Exception&)
		{
			// Disable monitor devices again even if there is an exception
			itf_map.setValue("SpawnMonitorDevices", false);

			throw;
		}
#endif
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}