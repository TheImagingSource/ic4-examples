using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace ActionCommandBroadcastTrigger
{
    internal class Program
    {
        static T PresentUserChoice<T>(IReadOnlyList<T> items, Func<T, string> getName, string header) where T : class
        {
            Console.WriteLine(header);

            for ( int index = 0; index < items.Count; ++index )
            {
                var itemName = getName(items[index]);

                Console.WriteLine($"[{index}] {itemName}");
            }

            while( true )
            {
                Console.Write("Select index: ");

                var input = Console.ReadLine();
                if (string.IsNullOrEmpty(input))
                {
                    Console.WriteLine("> Cancelled");
                    return null;
                }

                if(!int.TryParse(input, out int selectedIndex))
                {
                    Console.WriteLine("> Invalid input");
                    continue;
                }

                if( selectedIndex < 0 || selectedIndex >= items.Count )
                {
                    Console.WriteLine("> Invalid index");
                    continue;
                }

                return items[selectedIndex];
            }
        }

        static void Main(string[] args)
        {
            ic4.Library.Init();
            // Filter interfaces for GigEVision, since Action Command broadcasts are only supported by GigEVision devices/interfaces.
            var allInterfaces = ic4.DeviceEnum.Interfaces;
            var gevInterfaces = allInterfaces.Where(i => i.TransportLayerType == ic4.TransportLayerType.GigEVision).ToList();

            // Let the user select a network interface.
            var itf = PresentUserChoice(gevInterfaces, i => i.DisplayName, "Select GigEVision interface:");
            if (itf == null)
                return;

            // Check whether there are any video capture devices on the selected network interface.
            if( !itf.Devices.Any() )
            {
                Console.WriteLine("No devices found!");
                return;
            }

            // Both camera configuration and Action Command broadcast packets contain 3 values to identify
            // which devices should respond to a broadcast packet.
            // The ActionDeviceKey value in the broadcast packet has to match the ActionDeviceKey setting of the camera.
            // The ActionGroupKey value in the broadcast packet has to match the ActionGroupKey setting of the camera.
            // The ActionGroupMask value in the broadcast packet has to have all bits set that are present in the ActionGroupMask setting of the camera.
            // Using a clever combination of device key, group key and group masks, it is possible to create
            // arbitrary groups of cameras that respond to a broadcast packet.

            // For testing, we use the same device key, group key, and group mask for both the broadcast packet and
            // all cameras.
            const long DEVICE_KEY = 0x00000123;
            const long GROUP_KEY = 0x00000456;
            const long GROUP_MASK = 0x00000001;

            /// Configure the broadcast packet that will be sent by the driver.
            itf.PropertyMap.SetValue(ic4.PropId.ActionDeviceKey, DEVICE_KEY);
            itf.PropertyMap.SetValue(ic4.PropId.ActionGroupKey, GROUP_KEY);
            itf.PropertyMap.SetValue(ic4.PropId.ActionGroupMask, GROUP_MASK);
            // Disable ActionScheduledTimeEnable, we want the actions to be executed immediately.
            itf.PropertyMap.SetValue("ActionScheduledTimeEnable", false);

            // Open all devices present on the selected network interface.
            var devices = itf.Devices.ToList();
            var grabbers = new ic4.Grabber[devices.Count];

            for( int i = 0; i < devices.Count; i++ )
            {
                grabbers[i] = new ic4.Grabber();
                grabbers[i].DeviceOpen(devices[i]);

                var map = grabbers[i].DevicePropertyMap;

                // Configure device for maximum resolution, maximum frame rate.
                map.SetValue(ic4.PropId.Width, map[ic4.PropId.Width].Maximum);
                map.SetValue(ic4.PropId.Height, map[ic4.PropId.Height].Maximum);
                map.SetValue(ic4.PropId.AcquisitionFrameRate, map[ic4.PropId.AcquisitionFrameRate].Maximum);

                // Configure Action0 to the same device key, group key and group mask as the broadcast packet prepared above.
                map.SetValue(ic4.PropId.ActionSelector, 0);
                map.SetValue(ic4.PropId.ActionDeviceKey, DEVICE_KEY);
                map.SetValue(ic4.PropId.ActionGroupKey, GROUP_KEY);
                map.SetValue(ic4.PropId.ActionGroupMask, GROUP_MASK);

                // Enable trigger mode, with trigger source set to Action0.
                map.SetValue(ic4.PropId.TriggerMode, "On");
                map.SetValue(ic4.PropId.TriggerSource, "Action0");

                // Create a sink to receive images.
                var sink = new ic4.QueueSink();
                var deviceIndex = i; // Have to capture value of loop index in a local variable
                sink.FramesQueued += (s, ea) =>
                {
                    if( sink.TryPopOutputBuffer(out ic4.ImageBuffer buffer))
                    {
                        Console.WriteLine($"Image on device {deviceIndex}, frameId = {buffer.MetaData.DeviceFrameNumber}, timestamp = {buffer.MetaData.DeviceTimestampNs}");
                        buffer.Dispose();
                    }
                };

                // Set up stream for this camera.
                grabbers[i].StreamSetup(sink);
            }

            // Capture 10 images
            for( int i = 0; i < 10; ++i)
            {
                // Instruct the network interface to send one broadcast Action Command.
                itf.PropertyMap.ExecuteCommand("ActionCommand");

                System.Threading.Thread.Sleep(500);
            }
        }
    }
}