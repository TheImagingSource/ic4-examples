using ic4;
using System;

namespace savebmpontrigger
{
    class Program
    {
        static void Main(string[] args)
        {
            ic4.Library.Init();

            //Let the user select a device
            var deviceList = ic4.DeviceEnum.Devices;
            var it = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select Device:");
            if (it == null)
                return;

            // Open the selected device
            var grabber = new ic4.Grabber();
            grabber.DeviceOpen(it);

            var propertyMap = grabber.DevicePropertyMap;

            // Reset all device settings to default
            // Not all devices support this, so ignore possible errors
            try
            {
                propertyMap.SetValue(ic4.PropId.UserSetSelector, "Default");
                propertyMap.ExecuteCommand(ic4.PropId.UserSetLoad);
            }
            catch { }

            // Select FrameStart trigger (for cameras that support this)
            try
            {
                propertyMap.SetValue(ic4.PropId.TriggerSelector, "FrameStart");
            }
            catch { }

            // Enable trigger mode
            propertyMap.SetValue(ic4.PropId.TriggerMode, "On");

            // Create a QueueSink to capture all images arriving from the video capture device, specifyin a partial file name
            string path_base = "./test_image";

            var sink = new ic4.QueueSink();
            
            int counter = 0;
            sink.FramesQueued += (s, ea) =>
            {
                if (sink.TryPopOutputBuffer(out ic4.ImageBuffer buffer))
                {
                    // Generate a file name for the bitmap file
                    var file_name = $"{path_base}{counter}.bmp";

                    // Save the image buffer in the bitmap file
                    try
                    {
                        buffer.SaveAsBitmap(file_name);
                        Console.WriteLine($"Saved image {file_name}");
                    }
                    catch (IC4Exception err)
                    {
                        Console.WriteLine($"Failed to save buffer: {err.Message}");
                    }
                    buffer.Dispose();
                    counter++;
                }
            };

            // Start the video stream into the sink
            grabber.StreamSetup(sink, StreamSetupOption.AcquisitionStart);

            Console.WriteLine("Stream started");
            Console.WriteLine("Waiting for triggers");
            Console.WriteLine($"All images will be saved as {path_base}.bmp");
            Console.WriteLine("");

            Console.WriteLine("Input hardware triggers, or press ENTER to issue a software trigger");
            Console.WriteLine("Press q + ENTER to quit");

            while (true) 
            {
                var key = Console.ReadLine();
                if (key == "q")
                    break;

                // Execute software trigger
                propertyMap.ExecuteCommand(ic4.PropId.TriggerSoftware);
            }

            grabber.StreamStop();
            grabber.DeviceClose();

        }
    }
}
