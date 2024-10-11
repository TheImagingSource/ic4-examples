#include <ic4/ic4.h>

#include <console-helper.h>

#include <iostream>


// Define QueueSinkListener-derived class that saves all received frames in bitmap files
class SaveAsBmpListener : public ic4::QueueSinkListener
{
private:
	std::string path_base_;
	int counter_;

public:
	SaveAsBmpListener(std::string path_base)
		: path_base_(std::move(path_base))
		, counter_(0)
	{
	}

	// Inherited via QueueSinkListener, called when there are frames available in the sink's output queue
	void framesQueued(ic4::QueueSink& sink) override
	{
		ic4::Error err;

		while (true)
		{
			// Remove a buffer from the sink's output queue
			auto buffer = sink.popOutputBuffer(err);
			if (buffer == nullptr)
			{
				// No more buffers available, return
				return;
			}

			// Generate a file name for the bitmap file
			auto file_name = path_base_ + std::to_string(counter_) + ".bmp";

			// Save the image buffer in the bitmap file
			if (!ic4::imageBufferSaveAsBitmap(*buffer, file_name, {}, err))
			{
				std::cerr << "Failed save buffer: " << err.message() << std::endl;
			}
			else
			{
				std::cout << "Saved image " << file_name << std::endl;

				counter_ += 1;
			}
		}
	}
};

int main()
{
	ic4::initLibrary();
	std::atexit(ic4::exitLibrary);

	// Let the user select a device
	auto device_list = ic4::DeviceEnum::enumDevices();
	auto it = ic4_examples::console::select_from_list(device_list);
	if (it == device_list.end())
	{
		return -1;
	}

	// Open the selected device
	ic4::Error err;
	ic4::Grabber grabber;
	if (!grabber.deviceOpen(*it, err))
	{
		std::cerr << "Failed to open device: " << err.message() << std::endl;
		return -2;
	}

	auto map = grabber.devicePropertyMap(err);
	if (err.isError())
	{
		std::cerr << "Failed to query device property map: " << err.message() << std::endl;
		return -3;
	}

	// Reset all device settings to default
	// Not all devices support this, so ignore possible errors
	map.setValue(ic4::PropId::UserSetSelector, "Default", ic4::Error::Ignore());
	map.executeCommand(ic4::PropId::UserSetLoad, ic4::Error::Ignore());

	// Select FrameStart trigger (for cameras that support this)
	map.setValue(ic4::PropId::TriggerSelector, "FrameStart", ic4::Error::Ignore());

	// Enable trigger mode
	if (!map.setValue(ic4::PropId::TriggerMode, "On", err))
	{
		std::cerr << "Failed to enable trigger mode: " << err.message() << std::endl;
		return -4;
	}
	
	// Create an instance of the listener type defined above, specifying a partial file name
	std::string path_base = "./test_image";
	SaveAsBmpListener listener(path_base);

	// Create a QueueSink to capture all images arriving from the video capture device
	auto sink = ic4::QueueSink::create(listener, err);
	if (!sink)
	{
		std::cerr << "Failed to create sink: " << err.message() << std::endl;
		return -3;
	}

	// Start the video stream into the sink
	if (!grabber.streamSetup(sink, ic4::StreamSetupOption::AcquisitionStart, err))
	{
		std::cerr << "Failed to setup stream: " << err.message() << std::endl;
		return -4;
	}

	std::cout << "Stream started." << std::endl;
	std::cout << "Waiting for triggers" << std::endl;
	std::cout << "All images will be saved as " << path_base << "*.bmp" << std::endl;
	std::cout << std::endl;

	std::cout << "Input hardware triggers, or press ENTER to issue a software trigger" << std::endl;
	std::cout << "Press q + ENTER to quit" << std::endl;

	while( true )
	{
		int ch = std::getchar();
		if (ch == 'q')
			break;

		// Execute software trigger
		if (!map.executeCommand(ic4::PropId::TriggerSoftware, err))
		{
			std::cerr << "Failed to perform software trigger: " << err.message() << std::endl;
			continue;
		}
	}

	// We have to call streamStop before exiting the function, because we have the listener defined as a stack variable.
	grabber.streamStop();
	grabber.deviceClose();

	return 0;
}