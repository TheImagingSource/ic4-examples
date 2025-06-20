
#include <iostream>
#include <cstdio>

#include <ic4/ic4.h>

#include <console-helper.h>

#include <thread>

// This example shows how to use the QueueSink to measure the frame rate

int main()
{
	ic4::initLibrary();
	std::atexit(ic4::exitLibrary);

	auto device_list = ic4::DeviceEnum::enumDevices();
	auto it = ic4_examples::console::select_from_list(device_list);
	if (it == device_list.end())
	{
		return -1;
	}

	ic4::Error err;
	ic4::Grabber grabber;
	if (!grabber.deviceOpen(*it, err))
	{
		std::cerr << "Failed to open device: " << err.message() << std::endl;
		return -2;
	}

	auto& map = grabber.devicePropertyMap();

	// Try loading default UserSet to reset device to defaults
	// This reverses settings like TriggerMode=On, which would prevent this demo from running as expected
	map.setValue(ic4::PropId::UserSetSelector, "Default", ic4::Error::Ignore());
	map.executeCommand(ic4::PropId::UserSetLoad, ic4::Error::Ignore());

	std::chrono::steady_clock::time_point first_frame_timepoint;	// Arrival-time of the first frame
	std::chrono::steady_clock::time_point last_frame_timepoint;		// Arrival-time of the last frame
	int64_t frame_count = 0;										// Count of frames received
	auto callback_function = [&](ic4::QueueSink& sink)
	{
		if (frame_count == 0)	// Save the time of the first frame
		{
			first_frame_timepoint = std::chrono::steady_clock::now();
		}

		last_frame_timepoint = std::chrono::steady_clock::now();

		++frame_count;

		auto buf = sink.popOutputBuffer();	// We just drop the buffer here, so that it can re-queue itself in the queue
	};

	auto sink = ic4::QueueSink::create(callback_function, err);
	if (!sink)
	{
		std::cerr << "Failed to create sink: " << err.message() << std::endl;
		return -3;
	}

	if (!grabber.streamSetup(sink, ic4::StreamSetupOption::AcquisitionStart, err))
	{
		std::cerr << "Failed to setup stream: " << err.message() << std::endl;
		return -4;
	}

	// Wait for 3 seconds
	for (int i = 0; i < 3; ++i)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "." << std::endl;
	}

	grabber.streamStop();

	auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(last_frame_timepoint - first_frame_timepoint).count();
	auto fps = (frame_count - 1) / static_cast<double>(total_time) * 1'000'000.;

	std::cout << "fps: " << fps << "\ntotal-time: " << total_time << "us\nframes-received: " << frame_count << "\n";
	std::cout << std::endl;


	grabber.deviceClose();

	return 0;
}