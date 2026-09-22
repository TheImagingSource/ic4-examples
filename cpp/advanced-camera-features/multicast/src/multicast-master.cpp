
#include <ic4/ic4.h>

#include <console-helper.h>

#include <regex>

/*
 * These example program demonstrates how to set up multicast image transmission using GigE Vision cameras
 * - multicast-master is started first opens the camera and configures it for multicast operation.
 *   It owns the camera and is the only program that can configure camera parameters.
 *   The master can be operated both with a data stream activated or in headless mode (without receiving image data)
 * - multicast-monitor is started second and subscribes to the multicast group to (also) receive the camera's image data.
 * 
 * This file contains the multicast-master program.
 * There are two optional features that are currently disabled behind a if(false) block:
 * - MTU auto-negotiation can be disabled, if the computer running the monitor program has a lower possible MTU than the
 *   computer running the master program
 * - The master can be configured to only control the camera but not actually receive data for itself.
 * 
 * Please note:
 * It is possible to run master and monitor programs on the same computer.
 */

// Multicast only works with GigE Vision devices
std::vector<ic4::DeviceInfo> enum_gige_devices()
{
	std::vector<ic4::DeviceInfo> result;

	for (auto&& dev : ic4::DeviceEnum::enumDevices())
	{
		// Skip non-GigE devices
		if (dev.getInterface().transportLayerType() != ic4::TransportLayerType::GigEVision)
		{
			continue;
		}

		// Skip "-Monitor" entries and only show the regular device entry
		auto serial = dev.serial();
		if (serial.substr(serial.size() - 8) == "-Monitor")
		{
			continue;
		}

		result.push_back(dev);
	}

	return result;
}

// Make sure the camera firmware supports multicast operation
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

	std::cout << "Started in master mode." << std::endl;

	auto device_list = enum_gige_devices();
	auto it = ic4_examples::console::select_from_list(device_list);
	if (it == device_list.end())
	{
		return -1;
	}

	// Define a simple queue sink that prints frame numbers of received images
	auto print_frames_sink = ic4::QueueSink::create(
		[](ic4::QueueSink& qs)
		{
			auto buffer = qs.popOutputBuffer();

			std::cout << "Received frame " << buffer->metaData().device_frame_number << std::endl;
		}
	);

	try
	{
		// Open the monitor device in a new Grabber
		ic4::Grabber grabber(*it);

		// Make sure the device supports multicast
		if (!checkFirmwareVersion(grabber))
		{
			std::cerr << "Firmware version does not support multicast operation. Please contact support." << std::endl;
			return -2;
		}

		// For multicast operation, the master has to manually configure stream options
		// on both the device's and driver's property maps:
		auto driver_map = grabber.driverPropertyMap();
		auto device_map = grabber.devicePropertyMap();

		// Optional:
		// If the monitor(s) have limited network access, it might be necessary to
		// limit MTU to a conservative value
		if (false)
		{
			// Disable packet-size auto-negotiation and set it to fixed 1500
			driver_map.setValue("GevStreamNegotiatePacketSize", false);
			device_map.setValue("GevSCPSPacketSize", 1500);
		}

		// Override destination address:port data with multicast address
		driver_map.setValue("GevStreamDestinationOverride", true);
		driver_map.setValue("GevStreamDestinationAddress", "225.0.0.1");
		driver_map.setValue("GevStreamDestinationPort", "50002");

		std::cout << "Starting master stream. Press ENTER to quit." << std::endl;

		// Optional:
		// Enable headless mode, where master controls the camera,
		// but does not receive image data
		if (false)
		{
			// Write destination address:port into the camera
			driver_map.executeCommand("GevStreamWriteDestinationSetup");

			// Mark Transport-Layer as locked, allowing access to AcquisitionStart
			device_map.setValue(ic4::PropId::TLParamsLocked, true);

			// Let the camera start acquisition
			device_map.executeCommand("AcquisitionStart");
		}
		else
		{
			// Start a stream, printing information about received images
			grabber.streamSetup(print_frames_sink);
		}

		// Wait for ENTER key
		std::cin.get();
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}