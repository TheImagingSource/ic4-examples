
using System;
using System.Collections.Generic;
using ic4;

/* This program demonstrate how to save and load device settings to and from configuration files and memory buffers.
 * 
 * The example is split into 2 sections, demonstrating different modes of device settings serialization:
 * 
 * Section 1 demonstrates how to store the Grabber's device state and reopen the device from the state file:
 * - Save the Grabber's device state to a settings file
 * - Modify a device property (Width)
 * - Close the device
 * - Reopen the device using the settings file, restoring the original Width value
 * 
 * The settings file contains both a device identifier and the device settings, so that the device can be reopened and its
 * settings restored.
 * There are function overloads to store the Grabber's device state into a memory buffer instead.
 * 
 * In section 2, only the settings inside the device's PropertyMap are saved and restored:
 * - Save the device's PropertyMap into a memory buffer
 * - Modify a device property (Width)
 * - Restore the device PropertyMap's state from the memory buffer
 * 
 * The settings buffer only contains the PropertyMap's settings.
 * Therefore, this approach can be used to transfer camera settings from one device to another.
 * There are function overloads to store the PropertyMap's state into a file instead.
 */


namespace CameraConfigurationFiles
{
    class Program
    {
        // Helper function to print info about the device opened in the passed Grabber
        static void PrintDeviceInfo(string label, Grabber grabber)
        {
            if (grabber.IsDeviceValid)
            {
                var info = grabber.DeviceInfo;
                var propMap = grabber.DevicePropertyMap;

                var modelName = info.ModelName;
                var width = propMap.GetValueLong(PropId.Width);

                Console.WriteLine($"{label} -- {modelName} -- Width: {width}");
            }
            else
            {
                Console.WriteLine($"{label} -- No Device Opened");
            }
        }

        static int Main()
        {
            // Initialize the library
            Library.Init();

            // Let the user select a camera
            var deviceList = DeviceEnum.Devices;
            var selectedDevice = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select device:");
            if (selectedDevice == null)
            {
                return -1;
            }

            try
            {
                var grabber = new Grabber();
                PrintDeviceInfo("Before opening device", grabber);

                // Open the selected device
                grabber.DeviceOpen(selectedDevice);
                PrintDeviceInfo("After opening device", grabber);

                // Reset camera settings to default by executing UserSetLoad[Default]
                var map = grabber.DevicePropertyMap;
                map.SetValue(PropId.UserSetSelector, "Default");
                map.ExecuteCommand(PropId.UserSetLoad);
                PrintDeviceInfo("After resetting device to defaults", grabber);


                // Section 1: Save Grabber's device state into a file and restore it
                const string DEVICE_STATE_FILE_NAME = "settings.json";

                // Save device state
                grabber.DeviceSaveState(DEVICE_STATE_FILE_NAME);
                Console.WriteLine($">>> Saved device state into {DEVICE_STATE_FILE_NAME}");

                // Modify a property for demonstration
                map.SetValueMinimum(PropId.Width);
                PrintDeviceInfo("After setting Width to minimum", grabber);

                // Close the device
                grabber.DeviceClose();
                PrintDeviceInfo("After closing device", grabber);

                // Reopen using device state file
                grabber.DeviceOpenFromState(DEVICE_STATE_FILE_NAME);
                PrintDeviceInfo("After opening device from file", grabber);


                // Section 2: Save PropertyMap's state into a memory buffer and restore it

                // propMap has become invalid after the device was closed above
                map = grabber.DevicePropertyMap;

                // Save PropertyMap's state into a buffer
                var buffer = map.Serialize();
                Console.WriteLine(">>> Saved property map state into memory buffer");

                // Modify a property for demonstration
                map.SetValueMinimum(PropId.Width);
                PrintDeviceInfo("After setting Width to minimum", grabber);

                // Restore PropertyMap state from buffer
                map.DeSerialize(buffer);
                Console.WriteLine(">>> Restored property map state from memory buffer");
                PrintDeviceInfo("After deserializing property map state", grabber);

                return 0;
            }
            catch (IC4Exception ex)
            {
                Console.Error.WriteLine("An exception occurred: ");
                Console.Error.WriteLine(ex.Message);
                return -10;
            }
        }
    }
}