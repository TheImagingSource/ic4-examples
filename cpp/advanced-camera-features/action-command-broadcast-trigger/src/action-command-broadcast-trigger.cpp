
#include <vector>
#include <thread>
#include <chrono>

#include <ic4/ic4.h>

#include <console-helper.h>


int main()
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

	// Filter interfaces for GigEVision, since Action Command broadcasts are only supported by GigEVision devices/interfaces.
	auto all_interfaces = ic4::DeviceEnum::enumInterfaces();
	std::vector<ic4::Interface> gige_interfaces;
	for (auto&& itf : all_interfaces)
	{
		if (itf.transportLayerType(ic4::Error::Ignore()) == ic4::TransportLayerType::GigEVision)
			gige_interfaces.push_back(itf);
	}

	// Let the user select a network interface.
	auto it = ic4_examples::console::select_from_list(gige_interfaces);
	if (it == gige_interfaces.end())
	{
		return -1;
	}

	try
	{
		auto itf = *it;

		// Check whether there are any video capture devices on the selected network interface.
		auto devices = itf.enumDevices();
		if (devices.empty())
		{
			std::cerr << "No devices found!" << std::endl;
			return -2;
		}

		// Both camera configuration and Action Command broadcast packets contain 3 values to identify
		// which devices should respond to a broadcast packet.
		// The ActionDeviceKey value in the broadcast packet has to match the ActionDeviceKey setting of the camera.
		// The ActionGroupKey value in the broadcast packet has to match the ActionGroupKey setting of the camera.
		// The ActionGroupMask value in the broadcast packet has to have all bits set that are present in the ActionGroupMask setting of the camera.
		// Using a clever combination of device key, group key and group masks, it is possible to create
		// arbitrary groups of cameras that respond to a broadcast packet.

		// For testing, we use the same device key, group key, and group mask for both the broadcast packet and
		// all cameras.
		const long DEVICE_KEY = 0x00000123;
		const long GROUP_KEY = 0x00000456;
		const long GROUP_MASK = 0x00000001;

		/// Configure the broadcast packet that will be sent by the driver.
		itf.interfacePropertyMap().setValue(ic4::PropId::ActionDeviceKey, DEVICE_KEY);
		itf.interfacePropertyMap().setValue(ic4::PropId::ActionGroupKey, GROUP_KEY);
		itf.interfacePropertyMap().setValue(ic4::PropId::ActionGroupMask, GROUP_MASK);

		// Disable ActionScheduledTimeEnable, we want the actions to be executed immediately.
		itf.interfacePropertyMap().setValue("ActionScheduledTimeEnable", false);

		// Open all devices present on the selected network interface.
		std::vector<ic4::Grabber> grabbers;
		grabbers.resize(devices.size());

		for (size_t i = 0; i < devices.size(); ++i)
		{
			grabbers[i].deviceOpen(devices[i]);

			auto map = grabbers[i].devicePropertyMap();

			// Configure device for maximum resolution, maximum frame rate.
			map.setValue(ic4::PropId::Width, map[ic4::PropId::Width].maximum());
			map.setValue(ic4::PropId::Height, map[ic4::PropId::Height].maximum());
			map.setValue(ic4::PropId::AcquisitionFrameRate, map[ic4::PropId::AcquisitionFrameRate].maximum());

			// Configure Action0 to the same device key, group key and group mask as the broadcast packet prepared above.
			map.setValue(ic4::PropId::ActionSelector, 0);
			map.setValue(ic4::PropId::ActionDeviceKey, DEVICE_KEY);
			map.setValue(ic4::PropId::ActionGroupKey, GROUP_KEY);
			map.setValue(ic4::PropId::ActionGroupMask, GROUP_MASK);

			// Enable trigger mode, with trigger source set to Action0.
			map.setValue(ic4::PropId::TriggerMode, "On");
			map.setValue(ic4::PropId::TriggerSource, "Action0");

			// Define a QueueSinkListener to print information about received images
			class PrintFrameReceived : public ic4::QueueSinkListener
			{
				size_t _deviceIndex;
			public:
				PrintFrameReceived(size_t deviceIndex)
					: _deviceIndex(deviceIndex)
				{
				}
				void framesQueued(ic4::QueueSink& sink) override
				{
					auto buffer = sink.popOutputBuffer();

					std::cout << "Image on device " << _deviceIndex;
					std::cout << ", FrameID = " << buffer->metaData().device_frame_number;
					std::cout << ", Timestamp = " << buffer->metaData().device_timestamp_ns;
					std::cout << std::endl;
				}
			};

			// Create a sink to receive images.
			auto sink = ic4::QueueSink::create(std::make_shared<PrintFrameReceived>(i));

			// Set up stream for this camera.
			grabbers[i].streamSetup(sink);
		}

		// Capture 10 images
		for (int i = 0; i < 10; ++i)
		{
			// Instruct the network interface to send one broadcast Action Command.
			itf.interfacePropertyMap().executeCommand("ActionCommand");

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		return 0;
	}
	catch (const ic4::IC4Exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}