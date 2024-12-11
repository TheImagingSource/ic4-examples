using ic4;
using ic4.WinForms;

namespace  Graphic_Overlay
{
    public partial class Form1 : Form
    {
        private readonly ic4.Grabber grabber = new();
        public Form1()
        {
            InitializeComponent();            
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // Add the device lost event handler.
            grabber.DeviceLost += grabber_DeviceLost;
        }

        /// <summary>
        /// Device button event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnDevice_Click(object sender, EventArgs e)
        {
            stopStream();
            grabber.DeviceClose();
            updateControls();
            Text = $"Graphic Overlay";
            try
            {
                if (ic4.WinForms.Dialogs.ShowDeviceDialog(grabber, this))
                {
                    Text = $"Graphic Overlay : {grabber.DeviceInfo.ModelName} {grabber.DeviceInfo.Serial}";
                    startStream();
                }
            }
            catch (IC4Exception iex)
            {
                MessageBox.Show($"Error starting stream:\n{iex.Message}",
                    "Graphic Overlay",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            updateControls();
        }

        /// <summary>
        /// Show the device property dialog. If the dialog was closed by OK button,
        /// the new settings are saved into the device file.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnProperties_Click(object sender, EventArgs e)
        {
            if (grabber.IsDeviceValid)
            {
                Dialogs.ShowDevicePropertyDialog(grabber, this, PropertyDialogFlags.AllowStreamRestart);
            }
        }

        /// <summary>
        /// Handler of the start / stop button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnStartStop_Click(object sender, EventArgs e)
        {
            if (grabber.IsStreaming)
            {
                stopStream();
            }
            else
            {
                startStream();
            }
        }   

        /// <summary>
        /// Update the form's control accordingly to the 
        /// current device state. 
        /// </summary>
        void updateControls()
        {
            btnProperties.Enabled = grabber.IsDeviceValid;
            btnStartStop.Enabled = grabber.IsDeviceValid;

            if (grabber.IsStreaming)
            {
                btnStartStop.Text = "Stop Stream";
            }
            else
            {
                btnStartStop.Text = "Start Stream";
            }
        }

        /// <summary>
        /// Start live streaming.
        /// </summary>
        void startStream()
        {
            if (!grabber.IsStreaming)
            {
                // Create the sink for continuous image processing.
                // We want to use BGRa8 (RGB32) in the sink, which 
                // allows us to use the System.Drawing.Graphics.                
                try
                {

                    var sink = new ic4.QueueSink(ic4.PixelFormat.BGRa8);
                    sink.FramesQueued += sink_FramesQueued;
    	
                    grabber.StreamSetup(sink );   
                    
                }
                catch( IC4Exception iex)
                {
                    MessageBox.Show($"Error starting stream:\n{iex.Message}", 
                        "Graphic Overlay",
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            updateControls();
        }

        /// <summary>
        /// Stop live streaming.
        /// </summary>
        void stopStream()
        {
            if (grabber.IsStreaming)
            {
                grabber.StreamStop();
            }
            updateControls();
        }

        /// <summary>
        /// Handler of the frame queue sink for continuous 
        /// image processing. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void sink_FramesQueued(object? sender, QueueSinkEventArgs e)
        {
            using var buffer = e.Sink.PopOutputBuffer();

            // Create a System.Drawing.Bitmap which shares the memory
            // of the image buffer.                
            using var bitmap = buffer.CreateBitmapWrap();

            // Now draw a line
            using var g = Graphics.FromImage(bitmap);
            g.DrawLine(new Pen(Color.Red, 3), 10, 10, 200, 200);

            // Now draw the image with the line in IC Imaging Display Control
            // This works fine, because buffer and bitmap share the same
            // memory. 
            ic4Display.DisplayBuffer(buffer);
        }

        /// <summary>
        /// Device lost handler. A message box is shown and the 
        /// states of the form's control are updated.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void grabber_DeviceLost(object? sender, EventArgs e)
        {
            Invoke(new Action(() =>
            {
                MessageBox.Show("Device Lost", "Graphic Overlay", 
                    MessageBoxButtons.OK, 
                    MessageBoxIcon.Error);
                updateControls();
            }));
        }
    }
}
