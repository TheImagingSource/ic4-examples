using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

/*
 * These example program demonstrates how to set up multicast image transmission using GigE Vision cameras
 * - multicast-master is started first opens the camera and configures it for multicast operation.
 *   It owns the camera and is the only program that can configure camera parameters.
 *   The master can be operated both with a data stream activated or in headless mode (without receiving image data)
 * - multicast-monitor is started second and subscribes to the multicast group to (also) receive the camera's image data.
 * 
 * This file contains the multicast-monitor program.
 * 
 * Connecting to a device in monitor mode takes these steps:
 * - Set the SpawnMonitorDevices flag on the GenTL representation of the selected device's network interface
 *   This will add an additional entry to the list of enumerated devices with a "-Monitor" suffix on the model name and serial number.
 * - Open the "-Monitor" device in a Grabber as usual
 * - Use Grabber::streamStart just with a normal camera
 * 
 * There are a few limitations when using a monitor device's property map:
 * - The device cannot be controlled
 * - There are only a few properties providing information about the stream and image format.
 * - Controls for image transformation performed by the driver are availble.
 */

namespace Multicast
{
    class MulticastMaster
    {
        public static int Main(string[] args)
        {

            // Initialize the library with sensible defaults:
            // - Log errors and warnings from API calls
            // - Log to stdout and Windows debug log
            //
            // Library.Init() initializes the .NET library before any IC4 objects are used.
            ic4.Library.Init(apiLogLevel: ic4.LogLevel.Warning, logTargets: ic4.LogTarget.StdOut | ic4.LogTarget.WinDebug);

            Console.WriteLine("Started in monitor mode.");

            var deviceList = EnumGigEDevices();
            var selectedDevice = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select device:");
            if (selectedDevice == null)
            {
                return -1;
            }

            // Define a simple queue sink that prints frame numbers of received images
            var printFramesSink = new ic4.QueueSink();

            printFramesSink.FramesQueued += (sender, e) =>
            {
                using (var buffer = printFramesSink.PopOutputBuffer())
                {
                    Console.WriteLine($"Received frame {buffer.MetaData.DeviceFrameNumber}");
                }
            };

            try
            {
                var dev = selectedDevice;

                // Activate generation monitor device entries on the interface of the selected device
                var itf = dev.Interface;
                var itfMap = itf.PropertyMap;

                itfMap.SetValue("SpawnMonitorDevices", true);

                try
                {
                    // The serial number of the monitor device instance has "-Monitor" appended to it
                    var monitorSerial = dev.Serial + "-Monitor";

                    // Open the monitor device in a new Grabber
                    var grabber = new ic4.Grabber(monitorSerial);

                    Console.WriteLine("Starting monitor stream. Press ENTER to quit.");

                    // Start a stream, printing information about received images
                    grabber.StreamSetup(
                        printFramesSink,
                        ic4.StreamSetupOption.AcquisitionStart);

                    // Wait for ENTER key
                    Console.ReadLine();
                }
                finally
                {
                    // Disable generation of monitor device entries again
                    itfMap.SetValue("SpawnMonitorDevices", false);
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("An exception occurred:");
                Console.Error.WriteLine(ex.Message);
                return -10;
            }

            return 0;
        }

        // Multicast only works with GigE Vision devices
        static List<ic4.DeviceInfo> EnumGigEDevices()
        {
            var result = new List<ic4.DeviceInfo>();

            foreach (var dev in ic4.DeviceEnum.Devices)
            {
                // Skip non-GigE devices
                if (dev.Interface.TransportLayerType != ic4.TransportLayerType.GigEVision)
                {
                    continue;
                }

                // Skip "-Monitor" entries and only show the regular device entry
                var serial = dev.Serial;
                if (serial.EndsWith("-Monitor", StringComparison.Ordinal))
                {
                    continue;
                }

                result.Add(dev);
            }

            return result;
        }
    }
}

