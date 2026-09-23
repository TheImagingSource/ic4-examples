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
 * master and monitor programs can be run on the same computer.
 * 
 * This file contains the multicast-master program.
 * There are two optional features that are currently disabled behind a if(false) block:
 * - MTU auto-negotiation can be disabled, if the computer running the monitor program has a lower possible MTU than the
 *   computer running the master program
 * - The master can be configured to only control the camera but not actually receive data for itself.
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

            Console.WriteLine("Started in master mode.");

            var deviceList = EnumGigEDevices();
            var selectedDevice = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select device:");
            if (selectedDevice == null)
            {
                return -1;
            }

            // Define a simple queue sink that prints frame numbers of received images
            var printFramesSink = new ic4.QueueSink(ic4.PixelFormat.Mono8);

            printFramesSink.FramesQueued += (sender, e) =>
            {
                using (var buffer = printFramesSink.PopOutputBuffer())
                {
                    Console.WriteLine($"Received frame {buffer.MetaData.DeviceFrameNumber}");
                }
            };

            try
            {
                // Open the monitor device in a new Grabber
                var grabber = new ic4.Grabber(selectedDevice);

                // Make sure the device supports multicast
                if (!CheckFirmwareVersion(grabber))
                {
                    Console.Error.WriteLine("Firmware version does not support multicast operation. Please contact support.");

                    return -2;
                }

                // For multicast operation, the master has to manually configure stream options
                // on both the device's and driver's property maps:
                var driverMap = grabber.DriverPropertyMap;
                var deviceMap = grabber.DevicePropertyMap;

                // Optional:
                // If the monitor(s) have limited network access, it might be necessary to
                // limit MTU to a conservative value
                if (false)
                {
                    // Disable packet-size auto-negotiation and set it to fixed 1500
                    driverMap.SetValue("GevStreamNegotiatePacketSize", false);
                    deviceMap.SetValue("GevSCPSPacketSize", 1500L);
                }

                // Override destination address:port data with multicast address
                driverMap.SetValue("GevStreamDestinationOverride", true);
                driverMap.SetValue("GevStreamDestinationAddress", "225.0.0.1");
                driverMap.SetValue("GevStreamDestinationPort", "50002");

                Console.WriteLine("Starting master stream. Press ENTER to quit.");

                // Optional:
                // Enable headless mode, where master controls the camera,
                // but does not receive image data
                if (false)
                {
                    // Write destination address:port into the camera
                    driverMap.ExecuteCommand("GevStreamWriteDestinationSetup");

                    // Mark Transport-Layer as locked, allowing access to AcquisitionStart
                    deviceMap.SetValue(ic4.PropId.TLParamsLocked, 1);

                    // Let the camera start acquisition
                    deviceMap.ExecuteCommand(ic4.PropId.AcquisitionStart);
                }
                else
                {
                    // Start a stream, printing information about received images
                    grabber.StreamSetup(printFramesSink);
                }

                // Wait for ENTER key
                Console.ReadLine();
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

        // Make sure the camera firmware supports multicast operation
        static bool CheckFirmwareVersion(ic4.Grabber grabber)
        {
            var deviceVersion = grabber.DevicePropertyMap.GetValueString(ic4.PropId.DeviceVersion);

            var pattern = new Regex(@"Rev (\d+)");
            var match = pattern.Match(deviceVersion);

            if (match.Success)
            {
                int revision = int.Parse(match.Groups[1].Value);

                if (revision < 4000)
                {
                    // GigE3 cameras need at least version 3173
                    return revision >= 3173;
                }
                else
                {
                    // GigE4 cameras need at least version 4380
                    return revision >= 4380;
                }
            }

            Console.Error.WriteLine($"Failed to extract firmware revision from DeviceVersion '{deviceVersion}'");

            return false;
        }
    }
}


