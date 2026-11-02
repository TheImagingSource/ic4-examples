using System;
using System.Threading;

using ic4;


class Program
{
    //
    // This example program demonstrates how to use the cameras LUT (lookup table) feature to modify the tone curve of the captured images.
    //

    // Helper function to let the grabber stream for a bit to let auto functions reach a useful state
    static void CalibrateBrightness(Grabber grabber)
    {
        using (var sink = new SnapSink())
        {
            grabber.StreamSetup(sink);

            Thread.Sleep(TimeSpan.FromSeconds(2));

            grabber.StreamStop();
        }
    }


    // Helper function to setup a datastream, and save a single image
    static void SaveSingleImage(Grabber grabber, string path)
    {
        // Setup a sink and data stream to snap an image
        using (var sink = new SnapSink())
        {
            grabber.StreamSetup(sink);

            using (var buffer = sink.SnapSingle(TimeSpan.FromSeconds(1)))
            {
                buffer.SaveAsJpeg(path);
            }

            grabber.StreamStop();

            Console.WriteLine($"Saved {path}");
        }
    }


    static void Main()
    {
        // Initialize the library with sensible defaults:
        // - Throw exceptions on errors
        // - Log errors and warnings from API calls
        // - Log to stdout and Windows debug log
        Library.Init(
            apiLogLevel: LogLevel.Warning,
            internalLogLevel: LogLevel.Off,
            logTargets: LogTarget.StdOut | LogTarget.WinDebug);

        try
        {
            // Let the user select a camera
            var deviceList = DeviceEnum.Devices;
            var device = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select Device:");
            if (device == null)
            {
                return;
            }

            // Open the device in a new Grabber instance
            var grabber = new Grabber(device);

            // Access the device's property map
            var map = grabber.DevicePropertyMap;

            // Make sure the camera supports the LUT feature
            if( !map.TryFind(PropId.LUTEnable, out _) )
            {
                Console.WriteLine("Failed to find LUTEnable property. The device likely does not have LUT support.");
                return;
            }

            // To get consistent behavior, first reset all camera properties to default by loading the 'Default' user set
            map.SetValue(PropId.UserSetSelector, "Default");
            map.ExecuteCommand(PropId.UserSetLoad);

            // Disable the camera's color preset if available, since LUT overrides gamma settings anyway
            // Linear settings provide a better result in this demo
            if (map.TrySetValue(PropId.UserSetSelector, "ColorLinear"))
            {
                map.ExecuteCommand(PropId.UserSetLoad);
            }

            map.SetValue(ic4.PropId.PixelFormat, ic4.PixelFormat.Mono8);

            // Let the device run with auto functions enabled for a few seconds to let them settle on useful values
            CalibrateBrightness(grabber);

            // First, we need to select which type of lookup table the camera shall use.
            // For this example, we are going to use and modify the 'Luminance' lookup table only.
            // Color cameras also have the option to use one lookup table per color channel ('Red', 'Green', 'Blue').
            // Please note that using per-channel and luminance lookup tables can be mutually exclusive.
            // Some cameras might not even have a LUTSelector -- in this case the error is ignored.
            map.TrySetValue(PropId.LUTSelector, "Luminance");

            // The lookup table is accessed by first writing the index of the table into LUTIndex,
            // and then writing the table entry using LUTValue.
            var lutIndex = map.Find(ic4.PropId.LUTIndex);
            var lutValue = map.Find(ic4.PropId.LUTValue);

            // Gather information about the ranges of both properties
            long indexMax = lutIndex.Maximum;
            long valueMax = lutValue.Maximum;

            // Minimum for both index and value is always 0
            System.Diagnostics.Debug.Assert(lutIndex.Minimum == 0);
            System.Diagnostics.Debug.Assert(lutValue.Minimum == 0);

            // Please note that both index and value do not necessarily have to align with the camera's current pixel format's bit depth.
            // The camera internally interpolates between values when required.
            Console.WriteLine($"Luminance LUT index has range 0..{indexMax}");

            Console.WriteLine($"Luminance LUT value has range 0..{valueMax}");

            // First, save a reference image with LUT disabled
            map.SetValue(PropId.LUTEnable, false);
            SaveSingleImage(grabber, "reference.jpeg");


            // Prepare a LUT that looks like it reduces the bit depth of the image to only 4 possible brightness levels
            const int LEVELS = 4;

            for (int i = 0; i <= indexMax; ++i)
            {
                long lvl = i / (indexMax / LEVELS);
                long val = valueMax / (LEVELS - 1) * lvl;

                lutIndex.Value = i;
                lutValue.Value = Math.Min(val, valueMax);
            }

            // Enable the LUT and save an image
            map.SetValue(PropId.LUTEnable, true);
            SaveSingleImage(grabber, "4levels.jpeg");


            // Prepare an inverted LUT that creates a 'negative' effect
            for (int i = 0; i <= indexMax; ++i)
            {
                long val = valueMax - (i * valueMax / indexMax);

                lutIndex.Value = i;
                lutValue.Value = Math.Min(val, valueMax);
            }

            // Save an image with the new LUT
            SaveSingleImage(grabber, "inverted.jpeg");


            // Prepare a S-curve LUT that enhances contrast
            double K = 10;

            Func<double, double> Sigmoid = (z) => 1.0 / (1.0 + Math.Exp(-z));

            double lo = Sigmoid(-K / 2.0);
            double hi = Sigmoid(K / 2.0);

            for (int i = 0; i <= indexMax; ++i)
            {
                // Normalize the LUT index to the range 0..1
                double t = (double)i / indexMax;

                double val =
                    (Sigmoid(K * (t - 0.5)) - lo) / (hi - lo);

                lutIndex.Value = i;
                lutValue.Value = (int)(val * valueMax);
            }

            // Save an image with the new LUT
            SaveSingleImage(grabber, "high-contrast.jpeg");

            // Disable LUT to not interfere with future camera usage
            map.SetValue(PropId.LUTEnable, false);
        }
        catch (Exception ex)
        {
            Console.WriteLine("An exception occurred:");
            Console.WriteLine(ex.Message);
        }
    }
}