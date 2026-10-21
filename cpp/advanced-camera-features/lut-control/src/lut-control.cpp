
#include <ic4/ic4.h>

#include <console-helper.h>

#include <thread>
#include <chrono>
#include <cmath>

/*
 * This example program demonstrates how to use the cameras LUT (lookup table) feature to modify the tone curve of the captured images.
 */

// Helper function to let the grabber stream for a bit to let auto functions reach a useful state
void calibrateBrightness(ic4::Grabber& grabber)
{
	auto sink = ic4::SnapSink::create();
	grabber.streamSetup(sink);

	std::this_thread::sleep_for(std::chrono::seconds(2));

	grabber.streamStop();
}

// Helper function to setup a datastream, and save a single image
void saveSingleImage(ic4::Grabber& grabber, const char* path)
{
	// Setup a sink and data stream to snap an image
	auto sink = ic4::SnapSink::create();
	grabber.streamSetup(sink);

	auto buffer = sink->snapSingle(1000);

	ic4::imageBufferSaveAsJpeg(*buffer, path);

	grabber.streamStop();

	std::cout << "Saved " << path << std::endl;
}

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
		// Let the user select a camera
		auto device_list = ic4::DeviceEnum::enumDevices();
		auto it = ic4_examples::console::select_from_list(device_list);
		if (it == device_list.end())
		{
			return -1;
		}

		// Open the device in a new Grabber instance
		ic4::Grabber grabber(*it);

		// Access the device's property map
		auto map = grabber.devicePropertyMap();

		// Make sure the camera supports the LUT feature
		ic4::Error err;
		auto lut_enable = map.find(ic4::PropId::LUTEnable, err);
		if (err.isError())
		{
			std::cerr << "Failed to find LUTEnable property. The device likely does not have LUT support.";
			return -2;
		}

		// To get consistent behavior, first reset all camera properties to default by loading the 'Default' user set
		map.setValue(ic4::PropId::UserSetSelector, "Default");
		map.executeCommand(ic4::PropId::UserSetLoad);

		// Disable the camera's color preset if available, since LUT overrides gamma settings anyway
		// Linear settings provide a better result in this demo
		if (map.setValue(ic4::PropId::UserSetSelector, "ColorLinear", ic4::Error::Ignore()))
		{
			map.executeCommand(ic4::PropId::UserSetLoad);
		}

		// Let the device run with auto functions enabled for a few seconds to let them settle on useful values
		calibrateBrightness(grabber);

		// First, we need to select which type of lookup table the camera shall use.
		// For this example, we are going to use and modify the 'Luminance' lookup table only.
		// Color cameras also have the option to use one lookup table per color channel ('Red', 'Green', 'Blue').
		// Please note that using per-channel and luminance lookup tables can be mutually exclusive.
		// Some cameras might not even have a LUTSelector -- in this case the error is ignored.
		map.setValue(ic4::PropId::LUTSelector, "Luminance", ic4::Error::Ignore());

		// The lookup table is accessed by first writing the index of the table into LUTIndex,
		// and then writing the table entry using LUTValue.
		auto lut_index = map.find(ic4::PropId::LUTIndex);
		auto lut_value = map.find(ic4::PropId::LUTValue);

		// Gather information about the ranges of both properties
		int index_max = lut_index.maximum();
		int value_max = lut_value.maximum();

		// Minimum for both index and value is always 0
		assert(lut_index.minimum() == 0);
		assert(lut_value.minimum() == 0);

		// Please note that both index and value do not necessarily have to align with the camera's current pixel format's bit depth.
		// The camera internally interpolates between values when required.
		std::cout << "Luminance LUT index has range 0.." << index_max << std::endl;
		std::cout << "Luminance LUT value has range 0.." << value_max << std::endl;

		// First, save a reference image with LUT disabled
		map.setValue(ic4::PropId::LUTEnable, false);
		saveSingleImage(grabber, "reference.jpeg");


		// Prepare a LUT that looks like it reduces the bit depth of the image to only 4 possible brightness levels
		const int LEVELS = 4;
		for (int i = 0; i <= index_max; ++i)
		{
			int lvl = i / (index_max / LEVELS);
			int val = value_max / (LEVELS - 1) * lvl;

			lut_index.setValue(i);
			lut_value.setValue(std::min(val, value_max));
		}

		// Enable the LUT and save an image
		map.setValue(ic4::PropId::LUTEnable, true);
		saveSingleImage(grabber, "4levels.jpeg");


		// Prepare an inverted LUT that creates a 'negative' effect
		for (int i = 0; i <= index_max; ++i)
		{
			int val = value_max - (i * value_max / index_max);

			lut_index.setValue(i);
			lut_value.setValue(std::min(val, value_max));
		}

		// Save an image with the new LUT
		saveSingleImage(grabber, "inverted.jpeg");


		// Prepare a S-curve LUT that enhances contrast
		double K = 10;

		auto sigmoid = [](double z)
			{
				return 1.0 / (1.0 + std::exp(-z));
			};

		double lo = sigmoid(-K / 2.0);
		double hi = sigmoid(K / 2.0);

		for (int i = 0; i <= index_max; ++i)
		{
			double t = i / static_cast<double>(index_max);

			double val = (sigmoid(K * (t - 0.5)) - lo) / (hi - lo);

			lut_index.setValue(i);
			lut_value.setValue(static_cast<int>(val * value_max));
		}

		// Save an image with the new LUT
		saveSingleImage(grabber, "high-contrast.jpeg");

		// Disable LUT to not interfere with future camera usage
		map.setValue(ic4::PropId::LUTEnable, false);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}
