using ic4;
using System;
using System.Diagnostics;
using System.Linq;
using System.Threading;

namespace EventExposureEnd
{
    /// <summary>
    /// This is our "real world" simulation.
    /// </summary>
    class RealWorld
    {
        private object mutex = new object();
        private long currentFrameID = -1;
        private System.Threading.Tasks.Task activeTask = null;
        private int taskDelay;

        public RealWorld(TimeSpan sceneSetupDuration)
        {
            taskDelay = (int)sceneSetupDuration.TotalMilliseconds;
        }

        /// <summary>
        /// Requests the real world to setup the scene for frameID.
        /// When called with an increased frame ID, will create a task that can be waited for.
        /// </summary>
        /// <param name="frameID">Frame ID</param>
        public void BeginSetupScene(long frameID)
        {
            lock (mutex)
            {
                if ( frameID > currentFrameID )
                {
                    currentFrameID = frameID;

                    activeTask = System.Threading.Tasks.Task.Delay(taskDelay);
                }
            }
        }

        /// <summary>
        /// Waits for the scene setup to be completed.
        /// </summary>
        public void WaitSetupSceneCompletion()
        {
            System.Threading.Tasks.Task t;

            lock (mutex)
            {
                while (activeTask == null)
                {
                    Monitor.Wait(mutex, 1);
                }

                t = activeTask;
                activeTask = null;
            }

            t.Wait();
        }

        /// <summary>
        /// Reset the world to its original state
        /// </summary>
        public void Reset()
        {
            currentFrameID = -1;
            activeTask = null;
        }
    }

    internal class Program
    {
        static void Main(string[] args)
        {
            ic4.Library.Init();

            // Let the user select a camera
            var deviceList = ic4.DeviceEnum.Devices.ToList();
            var devInfo = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select Device:");
            if (devInfo == null)
                return;

            // Create grabber and open device
            var grabber = new ic4.Grabber();
            grabber.DeviceOpen(devInfo);

            /**
             * GigEVision and USB3Vision devices can send asynchronous events to applications
             * This example shows how to receive events EventExposureEnd, which indicates that the camera has completed
             * the integration time and will start transmitting the image data.
             * 
             * This event is useful when syncronizing real-world activities with camera operation, for example moving
             * something while the image is being transmitted.
             * 
             * Events are configured and received through the device property map.
             * The following shows an excerpt from the device property map of a device supporting EventExposureEnd:
             * 
             * - EventControl
             *   - EventSelector
             *   - EventNotification[EventSelector]
             *   - EventExposureEndData
             *     - EventExposureEnd
             *     - EventExposureEndTimestamp
             *     - EventExposureEndFrameID
             *     
             * To receive notifications for a specific event, two steps have to be taken:
             * 
             * First, the device has to be configured to send generate the specific event. To enable the EventExposureEnd event, set the
             * "EventSelector" enumeration property to "EventExposureEnd", and then set the "EventNotification" enumeration property to "On".
             * 
             * Second, a property notification handler has to be registered for the property representing the event.
             * The EventExposureEnd is represented by the integer property "EventExposureEnd". This property has no function other
             * than being invalidated and thus having its notification raised when the device sends the event.
             * 
             * Event parameters are grouped with the event property in a property category with "Data" appended to the event's name,
             * in our case "EventExposureEndData". The category contains the integer properties "EventExposureEndTimestamp"
             * and "EventExposureEndFrameID", which provide the time stamp and frame ID of the event.
             * Event argument properties should only be read inside the event notification function to avoid data races.
             */

            // Get EventExposureEnd event property
            if (!grabber.DevicePropertyMap.TryFind(ic4.PropId.EventExposureEnd, out var eventExposureEnd))
            {
                Console.WriteLine("EventExposureEnd is not supported by this device");
                return;
            }

            // Reset all camera settings to default so that prior configuration does not interfere with this program
            grabber.DevicePropertyMap.SetValue(ic4.PropId.UserSetSelector, "Default");
            grabber.DevicePropertyMap.ExecuteCommand(ic4.PropId.UserSetLoad);

            // Configure a constant exposure time
            grabber.DevicePropertyMap.SetValue(ic4.PropId.ExposureAuto, "Off");
            grabber.DevicePropertyMap.SetValue(ic4.PropId.ExposureTime, 1000.0);

            // Enable trigger mode
            grabber.DevicePropertyMap.SetValue(ic4.PropId.TriggerMode, "On");

            // Create our "real world" with a next-frame setup time of 40 ms
            var realWorld = new RealWorld(TimeSpan.FromMilliseconds(40));

            // Run test without supplying EventExposureEnd event
            Console.WriteLine("Test WITHOUT EventExposureEnd");
            RunTest(grabber, realWorld, 50, null);

            // Reset real world for next test run
            realWorld.Reset();

            // Run test with registered notification handler for EventExposureEnd
            // This time, the real-world simulation is notified to setup the next scene at an earlier point in time than before,
            // leading to a reduced cycle time.
            Console.WriteLine("Test WITH EventExposureEnd");
            RunTest(grabber, realWorld, 50, eventExposureEnd);
        }

        /// <summary>
        /// Performs a test run. A test run is a number of cycles consisting of the following steps:
        /// <list type="bullet">
        ///     <item>Wait until real-world simulation is ready for the next image to be taken</item>
        ///     <item>Wait until next image can be triggered</item>
        ///     <item>Issue software trigger</item>
        /// </list>
        /// The real-world simulation is instructed to prepare the next scene when an image is received,
        /// or the EventExposureEnd notification occurs.
        /// </summary>
        /// <param name="grabber">The grabber to use for this test</param>
        /// <param name="realWorld">The real-world simulation to use for this tes</param>
        /// <param name="numCycles">Number of cycles to run</param>
        /// <param name="eventExposureEnd">Optional reference to the EventExposureEnd property of the device</param>
        private static void RunTest(Grabber grabber, RealWorld realWorld, int numCycles, Property eventExposureEnd)
        {
            if (eventExposureEnd != null)
            {
                // Register a notification handler for EventExposureEnd
                eventExposureEnd.Notification += (s, e) =>
                {
                    // Extract frame ID from event data
                    var fid = grabber.DevicePropertyMap.GetValueLong(ic4.PropId.EventExposureEndFrameID);

                    // Request real world scene-setup for next frame
                    // At this time, exposure is complete, but the image is still being transmitted.
                    // If we would wait for this call until the image is transmitted completely, we would waste time.
                    realWorld.BeginSetupScene(fid + 1);
                };

                // Enable EventExposureEnd event notification
                grabber.DevicePropertyMap.SetValue(ic4.PropId.EventSelector, "ExposureEnd");
                grabber.DevicePropertyMap.SetValue(ic4.PropId.EventNotification, "On");
            }
            else
            {
                // Disable EventExposureEnd event notification
                grabber.DevicePropertyMap.SetValue(ic4.PropId.EventSelector, "ExposureEnd");
                grabber.DevicePropertyMap.SetValue(ic4.PropId.EventNotification, "Off");
            }

            // An event to wait for an image being received by the sink.
            var imageReceived = new AutoResetEvent(true);

            // Create a new sink.
            var sink = new QueueSink();
            sink.FramesQueued += (s, e) =>
            {
                using (var buffer = sink.PopOutputBuffer())
                {
                    // Notify the test thread that an image was received, and it can proceed.
                    imageReceived.Set();

                    Console.Write(".");

                    var fid = buffer.MetaData.DeviceFrameNumber;

                    // Request real world scene-setup for next frame.
                    // This call will be ignored if the setup was already requested by the EventExposureEnd notification handler.
                    realWorld.BeginSetupScene((long)(fid + 1));
                }
            };

            // Setup stream
            grabber.StreamSetup(sink);

            // Request real-world scene for first frame.
            realWorld.BeginSetupScene(0);

            Console.WriteLine($"Running {numCycles} cycles...");
            var sw = System.Diagnostics.Stopwatch.StartNew();

            for (int i = 0; i < numCycles; ++i)
            {
                // Wait for the previous image to be received (minimum cycle time).
                imageReceived.WaitOne();

                // Wait for the scene setup to be completed.
                realWorld.WaitSetupSceneCompletion();

                // Issue software trigger after both the real-world scene setup is completed and the previous image was received.
                grabber.DevicePropertyMap.ExecuteCommand(PropId.TriggerSoftware);
            }

            sw.Stop();

            grabber.StreamStop();

            Console.WriteLine();
            Console.WriteLine($"Processed {numCycles} cycles in {sw.Elapsed.TotalMilliseconds:#.#} ms. {numCycles / sw.Elapsed.TotalSeconds:#.#} Cycles/sec");
        }
    }
}