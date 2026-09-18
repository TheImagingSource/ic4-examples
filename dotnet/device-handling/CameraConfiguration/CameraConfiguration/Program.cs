using System;

// This program opens a video capture device and performs simple camera configuration,
// demonstrating use of functions on ic4.PropertyMap.
//
// The example program
// - Configures maximum resolution
// - Sets maximum frame rate
// - Disables trigger mode
// - Disables auto gain, and sets gain to minimum
// - Enables auto exposure
// - Enables auto white balance
//
// Camera properties are set using functions on PropertyMap:
// - SetValue sets a property value
// - SetValueMaximum sets an int or float property to its maximum value
// - SetValueMinimum sets an int or float property to its minimum value
//
// When setting properties, the property to be modified is identified using the
// constants in ic4.PropId, e.g. ic4.PropId.ExposureTime.
//
// Please note: the type of the value passed to SetValue does not necessarily have
// to match the property type. For example SetValue will automatically convert
// boolean true to select the 'On' value on the enumeration property TriggerMode.
//
// Camera property values can be read as well:
// - GetValueLong
// - GetValueString
// - GetValueDouble
// - GetValueBool
//
// Please note: The type-part of the function name (Long, String, Double, Bool)
// refers to the return value of the function, not the property type.
// The property value is converted automatically.
// For example, it is possible to call GetValueString for integer property
// and get a string representation of the integer value.
// See the documentation for the specific function overloads for details.


class Program
{
    static void Main()
    {
        // Initialize the library
        ic4.Library.Init();

        // Let the user select a camera
        var deviceList = ic4.DeviceEnum.Devices;
        var device = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select device:");

        if (device == null)
            return;

        try
        {
            // Create a grabber with the selected device opened
            var grabber = new ic4.Grabber(device);

            // Camera settings are accessed via the device's property map:
            var map = grabber.DevicePropertyMap;

            // First, configure the camera:

            // Configure the camera for maximum resolution and frame rate
            map.SetValueMaximum(ic4.PropId.Width);
            map.SetValueMaximum(ic4.PropId.Height);
            map.SetValueMaximum(ic4.PropId.AcquisitionFrameRate);

            // Disable trigger mode
            // Use manual error handling instead of exceptions, since the camera might not support trigger mode.
            if (!map.TrySetValue(ic4.PropId.TriggerMode, "Off"))
                Console.Error.WriteLine("Failed to set trigger mode.");

            // Disable gain auto
            // Technically, GainAuto is usually an enumeration with entries 'Continuous' and 'Off',
            // but PropertyMap.SetValue tries to convert between property values automatically.
            // See the documentation for details.
            map.SetValue(ic4.PropId.GainAuto, false);

            // Set gain to its minimum value
            map.SetValueMinimum(ic4.PropId.Gain);

            // Enable auto exposure
            map.SetValue(ic4.PropId.ExposureAuto, "Continuous");

            // Enable auto white balance
            map.SetValue(ic4.PropId.BalanceWhiteAuto, "Continuous");

            // Then, query and print modified settings:

            // Query resolution
            var width = map.GetValueLong(ic4.PropId.Width);

            // Technically, Height is usually an integer.
            // PropertyMap.GetValueString automatically interprets that value as a string.
            var height = map.GetValueString(ic4.PropId.Height);

            // Query frame rate
            var fps = map.GetValueDouble(ic4.PropId.AcquisitionFrameRate);

            // Query trigger mode
            // Technically, TriggerMode is usually an enumeration with values 'On' and 'Off'.
            // PropertyMap.GetValueBool automatically interprets that value as a boolean.
            var triggerMode = map.GetValueBool(ic4.PropId.TriggerMode);

            // Query gain auto
            var gainAuto = map.GetValueString(ic4.PropId.GainAuto);

            // Query gain value
            var gainValue = map.GetValueDouble(ic4.PropId.Gain);

            // Query exposure auto
            var exposureAuto = map.GetValueString(ic4.PropId.ExposureAuto);

            // Query white balance auto
            var whiteBalanceAuto = map.GetValueString(ic4.PropId.BalanceWhiteAuto);

            Console.WriteLine("Camera configured:");
            Console.WriteLine($"- Width: {width}");
            Console.WriteLine($"- Height: {height}");
            Console.WriteLine($"- AcquisitionFrameRate: {fps}");
            Console.WriteLine($"- TriggerMode: {triggerMode}");
            Console.WriteLine($"- GainAuto: {gainAuto}");
            Console.WriteLine($"- Gain: {gainValue}");
            Console.WriteLine($"- ExposureAuto: {exposureAuto}");
            Console.WriteLine($"- BalanceWhiteAuto: {whiteBalanceAuto}");
        }
        catch (ic4.IC4Exception ex)
        {
            Console.Error.WriteLine("An exception occurred:");
            Console.Error.WriteLine(ex.Message);
        }
    }
}