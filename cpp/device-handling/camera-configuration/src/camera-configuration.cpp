
#include <iostream>
#include <cstdlib>
#include <cstdio>

#include <ic4/ic4.h>

#include <console-helper.h>

// This program opens a video capture device and performs simple camera configuration,
// demonstrating use of functions on ic4::PropertyMap.
// 
// Camera properties are set using functions on PropertyMap:
// - setValue sets a property value
// - setValueMaximum sets an int or float property to its maximum value
// - setValueMinimum sets an int or float property to its minimum value
// 
// When setting properties, the property to be modified is identified using the
// constants in ic4::PropId, e.g. ic4::PropId::ExposureTime.
// 
// Please note: the type of the value passed to setValue does not necessarily have to match
// the property type. For example setValue will automatically convert boolean true to select the 'On'
// value on the enumeration property TriggerMode.
//
// Camera property values can be read as well:
// - getValueInt64
// - getValueString
// - getValueDouble
// - getValueBool
//
// Please note: The type-part of the function name (Int64, String, Double, Bool)
// refers to the return value of the function, not the property type.
// The property value is converted automatically.
// For example, it is possible to call getValueString for integer property
// and get a string representation of the integer value.
// See the documentation for the specific function overloads for details.

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
	std::atexit(ic4::exitLibrary);

	// Let the user select a camea
	auto device_list = ic4::DeviceEnum::enumDevices();
	auto it = ic4_examples::console::select_from_list(device_list);
	if (it == device_list.end())
	{
		return -1;
	}

	try
	{
		// Create a grabber with the selected device opened
		ic4::Grabber grabber(*it);

		// Camera settings are accessed via the device's property map:
		auto map = grabber.devicePropertyMap();

		// First, configure the camera:

		// Configure the camera for maximum resolution and frame rate
		map.setValueMaximum(ic4::PropId::Width);
		map.setValueMaximum(ic4::PropId::Height);
		map.setValueMaximum(ic4::PropId::AcquisitionFrameRate);

		// Error object to handle 'expected' errors
		ic4::Error err;

		// Disable trigger mode
		// Use manual error handling instead of exceptions, since the camera might not support trigger mode.
		if (!map.setValue(ic4::PropId::TriggerMode, "Off", err))
		{
			std::cerr << "Failed to set trigger mode: " << err.message() << std::endl;
		}

		// Disable gain auto
		// Technically, GainAuto is usually an enumeration with entries 'Continuous' and 'Off',
		// but PropertyMap::setValue tries to convert between property values automagically.
		// See the documentation for details.
		map.setValue(ic4::PropId::GainAuto, false);
		// Set gain to its minimum value
		map.setValueMinimum(ic4::PropId::Gain);

		// Enable auto exposure
		map.setValue(ic4::PropId::ExposureAuto, "Continuous");

		// Enable auto white balance
		map.setValue(ic4::PropId::BalanceWhiteAuto, "Continuous");


		// Then, query and print modified settings:

		// Query resolution
		auto width = map.getValueInt64(ic4::PropId::Width);
		// Technically, Height is usually an integer.
		// PropertyMap::getValueString automatically interprets that value as a string.
		auto height = map.getValueString(ic4::PropId::Height);
		// Query frame rate
		auto fps = map.getValueDouble(ic4::PropId::AcquisitionFrameRate);
		// Query trigger mode
		// Technically, TriggerMode is usually an enumeration with values 'On' and 'Off'.
		// PropertyMap::getValueBool automatically interprets that value as a boolean.
		auto trigger_mode = map.getValueBool(ic4::PropId::TriggerMode);
		// Query gain auto
		auto gain_auto = map.getValueString(ic4::PropId::GainAuto);
		// Query gain value
		auto gain_value = map.getValueDouble(ic4::PropId::Gain);
		// Query exposure auto
		auto exposure_auto = map.getValueString(ic4::PropId::ExposureAuto);
		// Query white balance auto
		auto white_balance_auto = map.getValueString(ic4::PropId::BalanceWhiteAuto);

		std::cout << "Camera configured:" << std::endl;
		std::cout << "- Width: " << width << std::endl;
		std::cout << "- Height: " << height << std::endl;
		std::cout << "- AcquisitionFrameRate: " << fps << std::endl;
		std::cout << "- TriggerMode: " << trigger_mode << std::endl;
		std::cout << "- GainAuto: " << gain_auto << std::endl;
		std::cout << "- Gain: " << gain_value << std::endl;
		std::cout << "- ExposureAuto: " << exposure_auto << std::endl;
		std::cout << "- BalanceWhiteAuto: " << white_balance_auto << std::endl;

		return 0;
	}
	catch (const ic4::IC4Exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}
