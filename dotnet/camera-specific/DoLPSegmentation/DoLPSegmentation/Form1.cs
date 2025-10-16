using ic4;
using DoLP_Segmentation.PixelTypes;

namespace DoLP_Segmentation
{
    public partial class MainForm : Form
    {
        private ic4.Grabber _grabber = new ic4.Grabber();
        private BufferPool _bufferPool = new BufferPool();
        private int _DoLPThreshold = 30;
        private int _IntensityThreshold = 10;

        public MainForm()
        {
            InitializeComponent();
        }

        private static ic4.DeviceInfo? FindPolarizationCameraByName(IReadOnlyList<DeviceInfo> devices)
        {
            foreach (var dev in devices)
            {
                // Monochrome polarization cameras are DZK 33UX250 and DZK 33GX250
                if (dev.ModelName.StartsWith("DZK"))
                    return dev;
                // Color polarization cameras are DYK 33UX250 and DYK 33GX250
                if (dev.ModelName.StartsWith("DYK"))
                    return dev;
            }
            return null;
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            tbIntensityThreshold.Value = _IntensityThreshold;
            tbDoLPThreshold.Value = _DoLPThreshold;

            _grabber.DeviceOpen(FindPolarizationCameraByName(ic4.DeviceEnum.Devices));

            _grabber.DevicePropertyMap.SetValue(ic4.PropId.UserSetSelector, "Default");
            _grabber.DevicePropertyMap.ExecuteCommand("UserSetLoad");

            //Make sure to enable Processed Pixel Formats. Otherwise Pixel Formats are not available.
            _grabber.DevicePropertyMap.SetValue("ProcessedPixelFormatsEnable", true);
            try
            {
                // Camera is Mono
                _grabber.DevicePropertyMap.SetValue(ic4.PropId.PixelFormat, ic4.PixelFormat.PolarizedADIMono8);
            }
            catch
            {
                // Camera is Color
                _grabber.DevicePropertyMap.SetValue(ic4.PropId.PixelFormat, ic4.PixelFormat.PolarizedADIRGB8);
            }

            QueueSink sink = new QueueSink();
            sink.FramesQueued += FramesQueued;

            // Polarized Pixelformats can not be rendered. Thus we do not enable the Display, and
            // display visualized images manually.
            _grabber.StreamSetup(sink, StreamSetupOption.AcquisitionStart);
        }

        private unsafe void FramesQueued(object? sender, QueueSinkEventArgs e)
        {
            using (var buffer = e.Sink.PopOutputBuffer())
            {
                int width = buffer.ImageType.Width;
                int height = buffer.ImageType.Height;
                var pixelformat = buffer.ImageType.PixelFormat;

                var destBuffer = _bufferPool.GetBuffer(new ImageType(width, height, PixelFormat.BGRa8));

                // Decide on the transformation function based on the source format
                if (buffer.ImageType.PixelFormat == ic4.PixelFormat.PolarizedADIMono8)
                {
                    ThresholdPolarizedADIMono8(buffer, destBuffer, _DoLPThreshold, _IntensityThreshold);
                }
                else
                {
                    ThresholdPolarizedADIRGB8(buffer, destBuffer, _DoLPThreshold, _IntensityThreshold);
                }

                // Display the processed buffer
                ic4Display.DisplayBuffer(destBuffer);
            }
        }

        private static unsafe void ThresholdPolarizedADIRGB8(ImageBuffer src, ImageBuffer dest, int dolpThreshold, int intensityThreshold)
        {
            int width = src.ImageType.Width;
            int height = src.ImageType.Height;
            nint srcPtr = src.Ptr;
            long srcPitch = src.Pitch;
            nint dstPtr = dest.Ptr;
            long dstPitch = dest.Pitch;

            for (int y = 0; y < height; y++)
            {
                var pSrcLine = (PolarizedADIRGB8*)(srcPtr + y * srcPitch);
                var pDestLine = (BGRa8*)(dstPtr + y * dstPitch);

                for (int x = 0; x < width; x++)
                {
                    int avgDolp = (pSrcLine[x].DoLPRed + pSrcLine[x].DoLPGreen + pSrcLine[x].DoLPBlue) / 3;
                    int avgIntensity = (pSrcLine[x].IntensityRed + pSrcLine[x].IntensityGreen + pSrcLine[x].IntensityBlue) / 3;
                    if (avgDolp > dolpThreshold && avgIntensity > intensityThreshold)
                    {
                        pDestLine[x].Blue = 0x00;
                        pDestLine[x].Green = 0x00;
                        pDestLine[x].Red = 0xFF;
                        pDestLine[x].Reserved = 0xFF;
                    }
                    else
                    {
                        pDestLine[x].Blue = pSrcLine[x].IntensityBlue;
                        pDestLine[x].Green = pSrcLine[x].IntensityGreen;
                        pDestLine[x].Red = pSrcLine[x].IntensityRed;
                        pDestLine[x].Reserved = 0xFF;
                    }
                }
            }
        }

        private static unsafe void ThresholdPolarizedADIMono8(ImageBuffer src, ImageBuffer dest, int dolpThreshold, int intensityThreshold)
        {
            int width = src.ImageType.Width;
            int height = src.ImageType.Height;
            nint srcPtr = src.Ptr;
            long srcPitch = src.Pitch;
            nint dstPtr = dest.Ptr;
            long dstPitch = dest.Pitch;

            for (int y = 0; y < height; y++)
            {
                var pSrcLine = (PolarizedADIMono8*)(srcPtr + y * srcPitch);
                var pDestLine = (BGRa8*)(dstPtr + y * dstPitch);

                for (int x = 0; x < width; x++)
                {

                    if (pSrcLine[x].DoLP > dolpThreshold && pSrcLine[x].Intensity > intensityThreshold)
                    {
                        pDestLine[x].Blue = 0x00;
                        pDestLine[x].Green = 0x00;
                        pDestLine[x].Red = 0xFF;
                        pDestLine[x].Reserved = 0xFF;
                    }
                    else
                    {
                        pDestLine[x].Blue = pSrcLine[x].Intensity;
                        pDestLine[x].Green = pSrcLine[x].Intensity;
                        pDestLine[x].Red = pSrcLine[x].Intensity;
                        pDestLine[x].Reserved = 0xFF;
                    }
                }
            }
        }

        private void tbDoLPThreshold_ValueChanged(object sender, EventArgs e)
        {
            _DoLPThreshold = tbDoLPThreshold.Value;
            lblDoLPThreshold.Text = tbDoLPThreshold.Value.ToString();
        }
        private void tbIntensityThreshold_ValueChanged(object sender, EventArgs e)
        {
            _IntensityThreshold = tbIntensityThreshold.Value;
            lblIntensityThreshold.Text = tbIntensityThreshold.Value.ToString();
        }
    }
}
