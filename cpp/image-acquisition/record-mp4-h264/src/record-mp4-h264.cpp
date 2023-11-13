
#include <ic4/ic4.h>

#include <console-helper.h>

#include <iostream>
#include <cstdio>
#include <atomic>

// Define QueueSinkListener-derived class that can add the received frames into a video writer
class AddFrameListener : public ic4::QueueSinkListener
{
private:
	ic4::VideoWriter& writer_;
	std::atomic<bool> do_write_frames_;
	std::atomic<int> num_frames_written_;

public:
	AddFrameListener(ic4::VideoWriter& writer)
		: writer_(writer)
		, do_write_frames_(false)
		, num_frames_written_(0)
	{
	}

	// Inherited via QueueSinkListener, called when there are frames available in the sink's output queue
	void framesQueued(ic4::QueueSink& sink) override
	{
		ic4::Error err;

		// Remove a buffer from the sink's output queue
		// We have to remove buffers from the queue even if not recording; otherwise the device will not have
		// buffers to write new video data into
		auto buffer = sink.popOutputBuffer(err);
		if (buffer == nullptr)
		{
			std::cerr << "Failed to get frame from sink: " << err.message() << std::endl;
			return;
		}

		if (do_write_frames_)
		{
			// Pass the image buffer to the video writer
			if (!writer_.addFrame(buffer, err))
			{
				std::cerr << "Failed to add frame to video file: " << err.message() << std::endl;
			}

			num_frames_written_ += 1;
		}
	}

	void enable_recording(bool enable)
	{
		if (enable)
		{
			num_frames_written_ = 0;
		}
		do_write_frames_ = enable;
	}

	int num_frames_written()
	{
		return num_frames_written_;
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

	// #TODO: Insert format configuration

	// Create a video writer for H264-compressed MP4 files
	ic4::VideoWriter writer(ic4::VideoWriterType::MP4_H264);

	// Create an instance of the listener type defined above
	AddFrameListener listener(writer);

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

	// Query the sink's output image type
	// The image type is required when starting the video recording
	auto image_type = sink->outputImageType(err);
	if (err.isError())
	{
		std::cerr << "Failed to query sink output frame type: " << err.message() << std::endl;
		return -5;
	}

	// Query the device's configured frame rate.
	// The frame rate is later set as the video file's playback rate.
	auto frame_rate = grabber.devicePropertyMap().find(ic4::PropId::AcquisitionFrameRate).getValue(err);
	if (err.isError())
	{
		std::cerr << "Failed to query acquisition frame rate: " << err.message() << std::endl;
		return -6;
	}

	std::cout << "Stream started." << std::endl;
	std::cout << "ImageType: " << ic4::to_string(image_type) << std::endl;
	std::cout << "AcquisitionFrameRate: " << frame_rate << std::endl;
	std::cout << std::endl;

	for (int i = 0; i < 3; ++i)
	{
		std::cout << "Press any key to begin recording a video file" << std::endl;
		(void)std::getchar();

		std::string file_name = "video" + std::to_string(i) + ".mp4";

		// Begin writing a video file with a name, image type and playback rate
		if (!writer.beginFile(file_name.c_str(), image_type, frame_rate, err))
		{
			std::cerr << "Failed to begin recording: " << err.message() << std::endl;
			continue;
		}

		// Instruct our QueueSinkListener to write frames into the video writer
		listener.enable_recording(true);

		std::cout << "Recording started. Press any key to stop" << std::endl;
		(void)std::getchar();

		// Stop writing frames into the video writer
		listener.enable_recording(false);

		// Finalize the currently opened video file
		if (!writer.finishFile(err))
		{
			std::cerr << "Failed to finish recording: " << err.message() << std::endl;
			continue;
		}

		std::cout << "Saved video file " << file_name << std::endl;
		std::cout << "Wrote " << listener.num_frames_written() << " frames." << std::endl;
		std::cout << std::endl;
	}

	// We have to call streamStop before exiting the function, because we have the listener defined as a stack variable.
	grabber.streamStop();
	grabber.deviceClose();

	return 0;
}