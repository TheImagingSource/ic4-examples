
#include <opencv2/opencv.hpp>

#include <ic4/ic4.h>
#include <ic4-interop/interop-OpenCV.h>

#include <console-helper.h>

#include <iostream>

void example_imagebuffer_opencv_snap()
{
	// Let the user select a device
	auto devices = ic4::DeviceEnum::enumDevices();
	auto it = ic4_examples::console::select_from_list(devices);
	if( it == devices.end() )
		return;

	// Create a new Grabber and open the user-selected device
	ic4::Grabber grabber;
	grabber.deviceOpen(*it);

	// Create an OpenCV display window
	cv::namedWindow("display");

	// Create a sink that converts the data to something that OpenCV can work with (e.g. BGR8)
	auto sink = ic4::SnapSink::create(ic4::PixelFormat::BGR8);
	grabber.streamSetup(sink);

	for (int i = 0; i < 5; ++i)
	{
		std::cout << "Focus display window and press any key to continue..." << std::endl;
		cv::waitKey(0);

		// Snap image from running data stream
		auto buffer = sink->snapSingle(1000);

		// Create a cv::Mat pointing into the BGR8 buffer
		auto mat = ic4interop::OpenCV::wrap(*buffer);

		std::cout << "Displaying captured image" << std::endl;
		cv::imshow("display", mat);

		std::cout << "Focus display window and press any key to continue..." << std::endl;;
		cv::waitKey(0);

		// Blur the image in place (this still uses the ImageBuffer's memory)
		cv::blur(mat, mat, cv::Size(75, 75));

		std::cout << "Displaying blurred image" << std::endl;
		cv::imshow("display", mat);
	}

	grabber.streamStop();
}

int main()
{
	ic4::initLibrary();

	example_imagebuffer_opencv_snap();

	ic4::exitLibrary();
}