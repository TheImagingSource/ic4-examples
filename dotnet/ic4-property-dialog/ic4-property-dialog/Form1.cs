using ic4;
using ic4.WinForms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Contexts;
using System.Runtime.Remoting.Messaging;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using ic4.Examples;

namespace ic4_property_dialog
{
    public partial class Form1 : Form
    {
        class ic4PropertyDialog_ : Form
        {
            private PropertyView ic4PropertyView_;
            public ic4PropertyDialog_(ic4.Grabber grabber)
            {
                ic4PropertyView_ = new PropertyView(grabber, PropertyViewFlags.AllowStreamRestart)
                {
                    Parent = this,
                    Dock= DockStyle.Fill
                };

                int width = (int)(480.0f * WinformsUtil.Scaling);
                int height = (int)(800.0f * WinformsUtil.Scaling);

                this.Text = "Properties";
                this.AutoScaleDimensions = new System.Drawing.SizeF(12F, 25F);
                this.ClientSize = new System.Drawing.Size(width, height);
            }

            protected override void OnResizeBegin(EventArgs e)
            {
                ic4PropertyView_.SuspendLayout();
                base.OnResizeBegin(e);
            }
            
            protected override void OnResizeEnd(EventArgs e)
            {
                ic4PropertyView_.SuspendDrawing();
                ic4PropertyView_.ResumeLayout();
                ic4PropertyView_.ResumeDrawing();
                this.Refresh();
                base.OnResizeEnd(e);
            }

            protected override void OnClosing(CancelEventArgs e)
            {
                WinformsUtil.SuspendDrawing(this);
                base.OnClosing(e);
                WinformsUtil.ResumeDrawing(this);
            }


        };

        private ic4.WinForms.Display display_;
        private ic4.Grabber g = new ic4.Grabber();
        private ic4PropertyDialog_ dlg_;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            display_ = new ic4.WinForms.Display();
            display_.Parent= this;
            display_.Dock = DockStyle.Fill;

            ic4.WinForms.Dialogs.ShowDeviceDialog(g, this);

            InitializeDevice();

            dlg_ = new ic4PropertyDialog_(g);
            dlg_.Show();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if(dlg_ != null)
            {
                dlg_.Close();
            }

            g.AcquisitionStop();
            g.Dispose();
        }

        void InitializeDevice()
        {
            Pixelformat = PixelFormats.Last();
            Resolution = Resolutions.Last();
            Framerate = Framerates.Last();

            var qs = new ic4.QueueSink();
            qs.FramesQueued += Qs_FramesQueued;

            g.StreamSetup(qs, display_, ic4.StreamSetupOption.AcquisitionStart);
        }

        private void Qs_FramesQueued(object sender, QueueSinkEventArgs e)
        {
            if(e.Sink.TryPopOutputBuffer( out var output ))
            {
                g.DevicePropertyMap.ConnectChunkData(output);

                output.Dispose();
            }
        }

        bool IsRunning
        {
            get
            {
                return g.IsAcquisitionActive && g.IsStreaming;
            }
        }

        bool IsValid
        {
            get
            {
                return g.IsDeviceOpen && g.IsDeviceValid;
            }
        }
        void StartLive()
        {
            if(IsValid)
            {
                if(IsRunning)
                {
                    return; 
                }

                g.StreamSetup(display_, ic4.StreamSetupOption.AcquisitionStart);
            }
        }

        void StopLive()
        {
            if (IsValid)
            {
                if (IsRunning)
                {
                    g.StreamStop();
                }
            }
        }
        string Pixelformat
        {
            set
            {
                // set the pixel format 
                var properties = g.DevicePropertyMap;
                var pixelFormatProperty = properties.FindEnumeration("PixelFormat");
                if (pixelFormatProperty != null)
                {
                    properties.SetValue("PixelFormat", value);
                }

                // set the resolution if necessary
                if (!Resolutions.Any(s => s == Resolution))
                {
                    Resolution = Resolutions.Last();
                }
            }
            get
            {
                var properties = g.DevicePropertyMap;
                var pixelFormatProperty = properties.FindEnumeration("PixelFormat");
                if (pixelFormatProperty != null)
                {
                    return pixelFormatProperty.SelectedEntry.Name;
                }
                return string.Empty;
            }
           
        }

        Size Resolution
        {
            get
            {
                var properties = g.DevicePropertyMap;
                var pWidth = properties.FindInteger("Width");
                var pHeight = properties.FindInteger("Height");

                return new Size((int)pWidth.Value, (int)pHeight.Value);
            }
            set
            {
                var properties = g.DevicePropertyMap;
                var pWidth = properties.FindInteger("Width");
                var pHeight = properties.FindInteger("Height");
            }
        }

        double Framerate
        {
            set
            {
                var props = g.DevicePropertyMap;
                var pFps = props.FindFloat("AcquisitionFrameRate");
                if(pFps != null)
                {
                    pFps.Value= value;
                }
            }
            get
            {
                var props = g.DevicePropertyMap;
                var pFps = props.FindFloat("AcquisitionFrameRate");
                if (pFps != null)
                {
                    return pFps.Value;
                }
                return 0.0;
            }
        }

        IEnumerable<string> PixelFormats
        {
            get
            {
                var formatsList = new List<string>();

                var properties = g.DevicePropertyMap;
                var pixelFormatProperty = properties.FindEnumeration("PixelFormat");
                if (pixelFormatProperty != null)
                {
                    foreach (var entry in pixelFormatProperty.Entries)
                    {
                        formatsList.Add(entry.Name);
                    }
                }

                return formatsList;
            }
        }


        private System.Drawing.Size[] standardSizes = new Size[]
        {
            new Size( 160,120 ),
            new Size( 320,240  ),
            new Size( 640,480 ),
            new Size( 1024, 768  ),
            new Size( 1032, 768  ),
            new Size( 1280, 720  ),
            new Size( 1284, 720  ),
            new Size( 1280, 960  ),
            new Size( 1284, 960  ),
            new Size( 1600, 1200 ),
            new Size( 1608, 1200  ),
            new Size( 1920, 1080  ),
            new Size( 2048, 1536  ),
            new Size( 2052, 1536  ),
            new Size( 2560, 1920  ),
            new Size( 2568, 1440  ),
            new Size( 2568, 1920  ),
            new Size( 2592, 1944  ),
            new Size( 3444, 1440  ),
            new Size( 3840, 2160  ),
            new Size( 4000, 3000  ),
            new Size( 4104, 2160  ),
            new Size( 5424, 5360  ),
            new Size( 7716, 5360  )
        };

        IEnumerable<Size> Resolutions
        {
            get
            {
                var list = new List<Size>();

                var props = g.DevicePropertyMap;

                var pWidth = props.FindInteger("Width");
                var pHeight = props.FindInteger("Height");

                // enumerate resolutions 
                long widthMin = pWidth.Minimum;
                long widthMax = pWidth.Maximum;
                long widthIncrement = pWidth.Increment;

                long heightMin = pHeight.Minimum;
                long heightMax = pHeight.Maximum;
                long heightIncrement = pHeight.Increment;

                foreach (var s in standardSizes)
                {
                    if (s.Width >= widthMin && s.Width <= widthMax && s.Height >= heightMin && s.Height <= heightMax)
                    {
                        if (!pWidth.TrySetValue((long)s.Width))
                        {
                            continue;
                        }
                        if (!pHeight.TrySetValue((long)s.Height))
                        {
                            continue;
                        }
                        list.Add(s);
                    }
                }
                return list;
            }
        }

        IEnumerable<double> Framerates
        {
            get
            {
                var framerates = new List<double>();

                var props = g.DevicePropertyMap;
                var pFps = props.FindFloat("AcquisitionFrameRate");
                double min = pFps.Minimum;
                double max = pFps.Maximum;

                if (max <= min)
                {
                    framerates.Add(min);
                }
                else
                {
                    framerates.Add(min);

                    // we do not want every framerate to have unnecessary decimals
                    // e.g. 1.345678 instead of 1.00000
                    double current_step = (int)min;

                    // 0.0 is not a valid framerate
                    if (current_step < 1.0)
                        current_step = 1.0;

                    while (current_step < max)
                    {
                        if (current_step < 20.0)
                        {
                            current_step += 1;
                        }
                        else if (current_step < 100.0)
                        {
                            current_step += 10.0;
                        }
                        else if (current_step < 1000.0)
                        {
                            current_step += 50.0;
                        }
                        else
                        {
                            current_step += 100.0;
                        }

                        if (current_step < max)
                        {
                            framerates.Add(current_step);
                        }
                    }
                    if (framerates.Last() != max)
                    {
                        framerates.Add(max);
                    }
                }

                return framerates;
            }   
        }

    }    
}
