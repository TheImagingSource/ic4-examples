
#include <iostream>
#include <cstdio>

#include <ic4/ic4.h>

#include <console-helper.h>

int main()
{
	ic4::initLibrary();
	std::atexit(ic4::exitLibrary);

	auto device_list = ic4::DeviceEnum::getDevices();
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

	// #TODO: Insert format configuration

	auto sink = ic4::SnapSink::create(err);
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

	for (int i = 0; i < 10; ++i)
	{
		std::cout << "Press any key to snap and save a jpeg image" << std::endl;
		(void)std::getchar();

		auto image_buffer = sink->snapSingle(1000, err);
		if (!image_buffer)
		{
			std::cerr << "Failed to snap image: " << err.message() << std::endl;
			continue;
		}

		auto file_name = "image_" + std::to_string(i) + ".jpg";

		ic4::SaveJpegOptions options =
		{
			90	// .quality_pct =
		};
		if (!ic4::imageBufferSaveAsJpeg(*image_buffer, file_name, options, err))
		{
			std::cerr << "Failed to save image file: " << err.message() << std::endl;
			continue;
		}

		std::cout << "Saved image file " << file_name << std::endl;
		std::cout << std::endl;
	}

	// Only for completeness. Technically this is not necessary here, since the grabber is destroyed at the end of the function.
	grabber.streamStop();
	grabber.deviceClose();

	return 0;
}