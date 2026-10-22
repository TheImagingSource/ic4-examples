using ic4;
using System;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Timers;

namespace UserSets
{
    class Program
    {
        /*
         * This example program demonstrates the use of the UserSet features present in most modern cameras.
         *
         * UserSets provide a mechanism to store the camera configuration in the camera's non-volatile memory.
         *
         * Usually, a camera provides at least two 'slots' to save data into, named 'UserSet1' and 'UserSet2'.
         *
         * To save the camera configuration into one of the slots, first set UserSetSelector to the identifier of the slot to operate on.
         * Then, execute the UserSetSave command to store the current camera configuration into that slot.
         *
         * To later restore the camera configuration, first set UserSetSelector to the identifier of the slot to read from.
         * Then, execute the UserSetLoad command.
         *
         * There is a special 'Default' slot that contains the camera's default configuration that can be used to
         * restore the device into a known-good state. The UserSetSave operation is not available for the 'Default' slot.
         *
         * The UserSetDefault feature can be used to specify a slot that is loaded automatically when the camera is powered on
         * (or rebooted using DeviceReset).
         */

        static int Main()
        {
            try
            {
                // Initialize the library
                Library.Init();

                // Let the user select a camera
                var selectedDevice = ConsoleHelper.PresentUserChoice(DeviceEnum.Devices, "Select camera:");
                if (selectedDevice == null)
                {
                    return -1;
                }

                // Open the device in a new Grabber instance
                var grabber = new Grabber(selectedDevice);

                // Access the device's property map
                var map = grabber.DevicePropertyMap;

                // Make sure the camera supports the UserSet features
                if (!map.TryFind(PropId.UserSetSelector, out _))
                {
                    Console.Error.WriteLine("Failed to find UserSetSelector property. The device likely does not have UserSet support.");
                    return -2;
                }

                // Execute UserSetLoad[Default] to reset the camera to its default state
                map.SetValue(PropId.UserSetSelector, "Default");
                map.ExecuteCommand(PropId.UserSetLoad);

                // Query some property info for demonstration purposes
                var defaultHeight = map[PropId.Height].Value;
                var minHeight = map[PropId.Height].Minimum;

                // Modify a property for demonstration purposes
                map.SetValue(PropId.Height, minHeight);

                // Verify Height was changed to min_height
                Debug.Assert(map.GetValueLong(PropId.Height) == minHeight);

                // Execute UserSetSave[UserSet1] to store the camera configuration where Height==min_height
                map.SetValue(PropId.UserSetSelector, "UserSet1");
                map.ExecuteCommand(PropId.UserSetSave);

                // Load default UserSet again
                map.SetValue(PropId.UserSetSelector, "Default");
                map.ExecuteCommand(PropId.UserSetLoad);

                // Verify Height was restored to default_height
                Debug.Assert(map.GetValueLong(PropId.Height) == defaultHeight);

                // Load UserSet1 again
                map.SetValue(PropId.UserSetSelector, "UserSet1");
                map.ExecuteCommand(PropId.UserSetLoad);

                // Verify Height was restored to min_height
                Debug.Assert(map.GetValueLong(PropId.Height) == minHeight);

                // Set UserSetDefault so that the camera always loads UserSet1 at startup
                // Please note: Applications (such as ic4-demoapp and IC Capture 4) can have
                // their own mechanism to restore camera state, which can hide the effect of UserSetDefault.
                map.SetValue(PropId.UserSetDefault, "UserSet1");

                // Reboot the camera
                map.ExecuteCommand(PropId.DeviceReset);
                // DeviceReset is possibly an asynchronous operation
                Thread.Sleep(3000);

                // We have to call DeviceClose before DeviceOpen
                var uniqueName = selectedDevice.UniqueName;
                grabber.DeviceClose();

                // Try to re-discover the device after reset
                // Please note: We cannot use grabber.DeviceOpen(selectedDevice) here, because
                // selectedDevice refers to the previously-discovered device instance.
                Console.WriteLine("Waiting for the camera to re-appear after DeviceReset...");
                var waitTimeout = DateTime.UtcNow.AddSeconds(60);

                while (true)
                {
                    try
                    {
                        grabber.DeviceOpen(uniqueName);
                        break;
                    }
                    catch(IC4Exception)
                    {
                        // Ignore
                    }

                    if (DateTime.UtcNow > waitTimeout)
                    {
                        Console.Error.WriteLine("Failed to discover device after DeviceReset");
                        return -3;
                    }

                    Thread.Sleep(1000);
                }

                // Access the property map for the re-opened device
                map = grabber.DevicePropertyMap;

                // Verify Height was initialized to min_height
                Debug.Assert(map.GetValueLong(PropId.Height) == minHeight);

                // Revert camera startup behavior back to default
                map.SetValue(PropId.UserSetDefault, "Default");

                Console.WriteLine("All tests passed");
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("An exception occurred:");
                Console.Error.WriteLine(ex.Message);
                return -10;
            }

            return 0;
        }
    }
}
