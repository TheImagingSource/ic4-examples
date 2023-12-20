using ic4;
using OpenCvSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ImageBufferOpenCVLive
{
    internal class Program
    {
        static void Main(string[] args)
        {
            ic4.Library.Init(apiLogLevel: ic4.LogLevel.Info, logTargets: ic4.LogTarget.WinDebug);

            // Let the select a video capture device
            var deviceInfo = ic4.ConsoleHelper.PresentUserChoice(ic4.DeviceEnum.Devices, dev => dev.ModelName, "Select device:");
            if (deviceInfo == null)
                return;

            // Open the selected device in a new Grabber
            var grabber = new ic4.Grabber();
            grabber.DeviceOpen(deviceInfo);

            // Create a floating display
            var display = new ic4.FloatingDisplay();
            display.RenderPosition = ic4.DisplayRenderPosition.StretchCenter;

            // Let the display set a event when it is closed
            var windowClosed = new ManualResetEventSlim(false);
            display.WindowClosed += (s, e) =>
            {
                windowClosed.Set();
            };

            // Create a sink that receives the images
            // This sink fixes the pixel format to BGR8, so that the callback function receives 3-channel color images
            // maxOutputBuffers is set to 1 so that we always get the latest available image
            var sink = new ic4.QueueSink(ic4.PixelFormat.BGR8, maxOutputBuffers: 1);
            sink.FramesQueued += (s, e) =>
            {
                // Get the new buffer from the sink
                // Buffer is disposed at the end of the using-block to allow re-queue
                using (var buffer = sink.PopOutputBuffer())
                {
                    // Create an OpenCVSharp view onto the buffer
                    // This view is only valid while the buffer itself exists,
                    // which is guaranteed by them both not being passed out of this function
                    var wrap = buffer.CreateOpenCvWrap();

                    // Blur the buffer in-place using a rather large kernel
                    Cv2.Blur(wrap, wrap, new Size(31, 31));

                    // Write some text so that the user doesn't hopelessly try to focus the lens
                    Cv2.PutText(
                        img: wrap,
                        text: "This image is blurred using OpenCV",
                        org: new Point(100, 100),
                        fontFace: HersheyFonts.HersheySimplex,
                        fontScale: 1,
                        color: new Scalar(255, 0, 0),
                        thickness: 2
                    );

                    // Send the modified buffer to the display
                    display.DisplayBuffer(buffer);
                }
            };

            // Start the stream
            grabber.StreamSetup(sink);

            // Wait for the window to be closed
            windowClosed.Wait();

            grabber.StreamStop();
        }
    }
}
