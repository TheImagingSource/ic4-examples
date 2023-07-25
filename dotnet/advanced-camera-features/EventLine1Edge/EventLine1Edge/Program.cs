using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace DeviceEvents
{
    internal class Program
    {
        static void Main(string[] args)
        {
            ic4.Library.Init();

            // Let the user select a camera
            var deviceList = ic4.DeviceEnum.Devices.ToList();
            var devInfo = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select Device:");

            // Create grabber and open device
            var grabber = new ic4.Grabber();
            grabber.DeviceOpen(devInfo);

            /**
             * GigEVision and USB3Vision devices can send asynchronous events to applications
             * This example shows how to receive events EventLine1RisingEdge and EventLine1FallingEdge, which indicate activity
             * on one of the camera's digital inputs.
             * 
             * Events are configured and received through the device property map.
             * The following shows an excerpt from the device property map of a device supporting EventLine1RisingEdge and EventLine1FallingEdge:
             * 
             * - EventControl
             *   - EventSelector
             *   - EventNotification[EventSelector]
             *   - EventLine1RisingEdgeData
             *     - EventLine1RisingEdge
             *     - EventLine1RisingEdgeTimestamp
             *   - EventLine1FallingEdgeData
             *     - EventLine1FallingEdge
             *     - EventLine1FallingEdgeTimestamp
             *     
             * To receive notifications for a specific event, two steps have to be taken:
             * 
             * First, the device has to be configured to send generate the specific event. To enable the EventLine1RisingEdge event, set the
             * "EventSelector" enumeration property to "EventLine1RisingEdge", and then set the "EventNotification" enumeration property to "On".
             * 
             * Second, a property notification handler has to be registered for the property representing the event.
             * The EventLine1RisingEdge is represented by the integer property "EventLine1RisingEdge". This property has no function other
             * than being invalidated and thus having its notification raised when the device sends the event.
             * 
             * Event parameters are grouped with the event property in a property category with "Data" appended to the event's name,
             * in our case "EventLine1RisingEdgeData". The category contains the integer property "EventLine1RisingEdgeTimestamp"
             * which provides the time stamp of the event. Event argument properties should only be read inside the event notification
             * function to avoid data races.
             */

            try
            {
                // Get Line1RisingEdge and Line1FallingEdge event properties
                var eventLine1RisingEdge = grabber.DevicePropertyMap.Find("EventLine1RisingEdge");
                var eventLine1FallingEdge = grabber.DevicePropertyMap.Find("EventLine1FallingEdge");
                // Get Line1RisingEdge and Line1FallingEdge timestamp arguments
                var eventLine1RisingEdgeTimestamp = grabber.DevicePropertyMap.FindInteger("EventLine1RisingEdgeTimestamp");
                var eventLine1FallingEdgeTimestamp = grabber.DevicePropertyMap.FindInteger("EventLine1FallingEdgeTimestamp");

                // Enable both Line1RisingEdge and Line1FallingEdge events notifications
                grabber.DevicePropertyMap.SetValue("EventSelector", "EventLine1RisingEdge");
                grabber.DevicePropertyMap.SetValue("EventNotification", "On");
                grabber.DevicePropertyMap.SetValue("EventSelector", "EventLine1FallingEdge");
                grabber.DevicePropertyMap.SetValue("EventNotification", "On");

                try
                {
                    // Register notification for Line1RisingEdge
                    eventLine1RisingEdge.Notification += (s, e) =>
                    {
                        Console.WriteLine($"Line1 Falling Edge\t(Timestamp = {eventLine1RisingEdgeTimestamp.Value})");
                    };
                    // Register notification for Line1FallingEdge
                    eventLine1FallingEdge.Notification += (s, e) =>
                    {
                        Console.WriteLine($"Line1 Rising Edge\t(Timestamp = {eventLine1FallingEdgeTimestamp.Value})");
                    };

                    Console.WriteLine();
                    Console.WriteLine("Waiting for Line1RisingEdge and Line1FallingEdge events. Press any key to exit.");
                    Console.ReadKey();
                }
                finally
                {
                    // Disable both Line1RisingEdge and Line1FallingEdge event notifications
                    grabber.DevicePropertyMap.SetValue("EventSelector", "EventLine1RisingEdge");
                    grabber.DevicePropertyMap.SetValue("EventNotification", "Off");
                    grabber.DevicePropertyMap.SetValue("EventSelector", "EventLine1FallingEdge");
                    grabber.DevicePropertyMap.SetValue("EventNotification", "Off");
                }
            }
            catch(ic4.IC4Exception ex)
            {
                Console.WriteLine($"An error occurred: {ex.Message}");
                Console.WriteLine("Line1RisingEdge and/or Line1FallingEdge are not supported by this device");
            }
        }
    }
}