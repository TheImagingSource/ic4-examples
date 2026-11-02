
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
 * This file contains the multicast-monitor program.
 * 
 * Connecting to a device in monitor mode takes these steps:
 * - Set the SpawnMonitorDevices flag on the GenTL representation of the selected device's network interface
 *   This will add an additional entry to the list of enumerated devices with a "-Monitor" suffix on the model name and serial number.
 * - Open the "-Monitor" device in a Grabber as usual
 * - Use Grabber::streamStart just with a normal camera
 * 
 * There are a few limitations when using a monitor device's property map:
 * - The device cannot be controlled
 * - There are only a few properties providing information about the stream and image format.
 * - Controls for image transformation performed by the driver are availble.
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
		if (serial.size() >= 8 && serial.substr(serial.size() - 8) == "-Monitor")
		{
			continue;
		}

		result.push_back(dev);
	}

	return result;
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

	std::cout << "Started in monitor mode." << std::endl;

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
		ic4::DeviceInfo dev = *it;

		// Activate generation monitor device entries on the interface of the selected device
		auto itf = dev.getInterface();
		auto itf_map = itf.interfacePropertyMap();
		itf_map.setValue("SpawnMonitorDevices", true);
		auto disable_monitor_devices = ic4_examples::console::make_scope_exit(
			[&itf_map]()
			{
				itf_map.setValue("SpawnMonitorDevices", false);
			}
		);

		// The serial number of the monitor device instance has "-Monitor" appended to it
		auto monitor_serial = dev.serial() + "-Monitor";

		// Open the monitor device in a new Grabber
		ic4::Grabber grabber(monitor_serial);

		std::cout << "Starting monitor stream. Press ENTER to quit." << std::endl;

		// Start a stream, printing information about received images
		grabber.streamSetup(print_frames_sink);

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