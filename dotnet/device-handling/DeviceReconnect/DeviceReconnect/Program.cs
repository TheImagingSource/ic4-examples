using ic4;
using System;
using System.Linq;

namespace DeviceReconnect
{
    /*
     * This example program shows how to utilize the device-lost event on the Grabber object to
     * receive a notification in case the connection to the currently opened video capture device#
     * is interrupted.
     * It then re-enumerates the connected devices until the previously opened camera is found
     * and opens it again.
     */

    class Program
    {
        // The unique name of the device we are trying to reopen later
        private static string deviceUniqueName = "";

        // Flag to indicate that the search should be aborted
        private static volatile bool abortReconnect = false;

        static void Main(string[] args)
        {
            ic4.Library.Init();

            // Let the user select a device
            var deviceList = ic4.DeviceEnum.Devices;
            var it = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select Device:");

            // Create Grabber object and open device
            var grabber = new ic4.Grabber(it);

            // Register a function to be called when the opened device is disconnected.
            // The registeration function returns a token that can later be used to unregister the callback.
            grabber.DeviceLost += DeviceLostHandler;

            // Remember device unique name to reopen later
            deviceUniqueName = grabber.DeviceInfo.UniqueName;

            Console.WriteLine($"Opened device {grabber.DeviceInfo.ModelName} ({grabber.DeviceInfo.Serial})");
            Console.WriteLine("Disconnect device to produce device-lost event");

            Console.WriteLine("Press Enter to exit program");
            Console.Read();

            // Let the device-lost handler know to stop looking
            abortReconnect = true;

            return;
        }

        static void DeviceLostHandler(object sender, EventArgs e)
        {
            Console.WriteLine("Device Lost!");
            Console.WriteLine("Waiting for device to be reconnected...");

            var grabber = sender as ic4.Grabber;

            // We need to formally close the lost device before trying to open it again
            grabber.DeviceClose();

            while( !abortReconnect )
            {
                // Check whether one of the connected devices matches the unique name of the device we are looking for
                var dev = ic4.DeviceEnum.Devices.SingleOrDefault(d => d.UniqueName == deviceUniqueName);
                if( dev != null )
                {
                    // If it was found, try to open it
                    try
                    {
                        grabber.DeviceOpen(dev);
                        Console.WriteLine("Device re-opened successfully");
                    }
                    catch( IC4Exception ex )
                    {
                        Console.WriteLine($"Failed to re-open device: {ex.Message}");
                    }

                    return;
                }

                System.Threading.Thread.Sleep(100);
            }
        }
    }
}
