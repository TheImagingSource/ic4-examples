
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <vector>
#include <chrono>
#include <thread>

#include <ic4/ic4.h>

#include <console-helper.h>


struct PrintChunkExposureTimeListener : ic4::QueueSinkListener
{
	ic4::PropertyMap map_;
	ic4::PropFloat chunkExposureTime_;

	PrintChunkExposureTimeListener(ic4::PropertyMap m)
		: map_(m)
	{
		ic4::Error err;

		chunkExposureTime_ = map_.find(ic4::PropId::ChunkExposureTime, err);
		if (err.isError())
		{
			throw std::runtime_error("Float property ChunkExposureTime is not supported: " + err.message());
		}
	}

	void framesQueued(ic4::QueueSink& sink) override
	{
		// Do not throw from callback function, capture and log errors instead
		ic4::Error err;

		auto buffer = sink.popOutputBuffer(err);
		if (err.isError())
		{
			std::cerr << "popOutputBuffer failed: " << err.message() << std::endl;
			return;
		}

		// Use the image buffer as backend for read operations on chunk properties
		if (!map_.connectChunkData(buffer, err))
		{
			std::cerr << "connectChunkData failed: " << err.message() << std::endl;
			return;
		}
		
		// Read chunk property from image buffer
		auto val = chunkExposureTime_.getValue(err);
		if( err.isError() )
		{
			std::cerr << "ChunkExposureTime getValue failed: " << err.message() << std::endl;
			return;
		}

		// Disconnecting is not strictly necessary, but will release the buffer for reuse earlier
		map_.connectChunkData(nullptr, ic4::Error::Ignore());

		std::cout << " > ChunkExposureTime = " << val << std::endl;
	}
};

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

	try
	{
		auto device_list = ic4::DeviceEnum::enumDevices();
		auto it = ic4_examples::console::select_from_list(device_list);
		if (it == device_list.end())
		{
			return -1;
		}

		ic4::Grabber grabber;
		grabber.deviceOpen(*it);

		auto map = grabber.devicePropertyMap();

		PrintChunkExposureTimeListener listener(map);
		auto sink = ic4::QueueSink::create(listener);

		// Try loading default UserSet to reset device to defaults
		// This reverses settings like TriggerMode=On, which would prevent this demo from running as expected
		map.setValue(ic4::PropId::UserSetSelector, "Default", ic4::Error::Ignore());
		map.executeCommand(ic4::PropId::UserSetLoad, ic4::Error::Ignore());

		// Chunkdata-related properties have to be configured before streamSetup,
		// because enabling them increases the payload size
		map.setValue(ic4::PropId::ChunkModeActive, true);
		map.setValue(ic4::PropId::ChunkSelector, "ExposureTime");
		// Ignore possible error, since the device might have ChunkEnable[ExposureTime] locked to true
		map.setValue(ic4::PropId::ChunkEnable, true, ic4::Error::Ignore());

		std::cout << "Configure resolution 640x480" << std::endl;
		map.setValue(ic4::PropId::Width, 640, ic4::Error::Ignore());
		map.setValue(ic4::PropId::Height, 480, ic4::Error::Ignore());

		std::cout << "Set AcquisitionFrameRate to 5" << std::endl;
		map.setValue(ic4::PropId::AcquisitionFrameRate, 5, ic4::Error::Ignore());

		std::cout << "Set ExposureAuto to Off" << std::endl;
		map.setValue(ic4::PropId::ExposureAuto, "Off");

		std::cout << "Set ExposureTime to 2 ms" << std::endl;
		map.setValue(ic4::PropId::ExposureTime, 2000);

		grabber.streamSetup(sink);

		std::cout << "Stream for 3 seconds" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::cout << "Set ExposureTime to 8 ms" << std::endl;
		map.setValue(ic4::PropId::ExposureTime, 8000);

		std::cout << "Continue streaming for 3 seconds" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::cout << "Set ExposureTime to 32 ms" << std::endl;
		map.setValue(ic4::PropId::ExposureTime, 32000);

		std::cout << "Continue streaming for 3 seconds" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		// Stopping the stream is mandatory because we have the listener defined as a stack variable.
		// Just having everything go out of scope, the listener would be destroyed first and lead to undefined behavior.
		grabber.streamStop();

		return 0;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An exception occurred:" << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}