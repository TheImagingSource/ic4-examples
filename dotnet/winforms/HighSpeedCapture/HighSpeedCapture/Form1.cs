using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using ic4;
using ic4.WinForms;


namespace HighSpeedCapture
{
    public partial class Form1 : Form
    {
        private readonly Grabber _grabber = new Grabber();
        private QueueSink? _sink;
        private int _num_processed = 0;
        private int _frame_number = 0;
        private bool _cleanup_active = false;
        private object _frames_queued_mtx = new object();
        private bool _cancel_cleanup = false;
        private string? _destinationDirectory;
        private int _num_free = 0;
        private int _num_filled = 0;
        private int _num_total = 0;
        private int _bufferMemory = 4096;

        public Form1()
        {
            InitializeComponent();
            UpdateControls();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ReadSettings();

            _grabber.DeviceLost += _grabber_DeviceLost;

            textBoxDestinationFolder.Text = _destinationDirectory;
            textBoxBufferMemory.Text = _bufferMemory.ToString();
        }

        /// <summary>
        /// Handler of the frame queue sink for continuous 
        /// image processing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _sink_FramesQueued(object? sender, QueueSinkEventArgs e)
        {
            lock (_frames_queued_mtx)
            {
                using (var buffer = e.Sink.PopOutputBuffer())
                {
                    var filePath = $"{_destinationDirectory}/image_{_frame_number++}";
                    buffer.SaveAsJpeg($"{filePath}.jpeg");
                    _num_processed += 1;
                }

                var queueSizes = e.Sink.QueueSizes;
                _num_filled = queueSizes.OutputQueueLength;
                _num_free = queueSizes.FreeQueueLength;

                BeginInvoke(new Action(() =>
                {
                    UpdateCaptureInfo();
                }));
            }
        }

        /// <summary>
        /// Device lost handler. A message box is shown and the 
        /// states of the form's control are updated.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _grabber_DeviceLost(object? sender, EventArgs e)
        {
            Invoke(new Action(() =>
            {
                MessageBox.Show("Device Lost", "HighSpeedCapture", MessageBoxButtons.OK, MessageBoxIcon.Error);
                UpdateControls();
            }));
        }

        void UpdateControls()
        {
            if (_sink != null) // Presence of sink indicates capture is active
            {
                buttonStart.Text = "Stop";
                btnDevice.Enabled = false;
                textBoxBufferMemory.Enabled = false;
                buttonBrowse.Enabled = false;
            }
            else
            {
                buttonStart.Text = "Start";
                btnDevice.Enabled = true;
                textBoxBufferMemory.Enabled = true;
                buttonBrowse.Enabled = true;
            }

            if (_grabber.IsDeviceValid)
            {
                btnProperties.Enabled = true;
                buttonStart.Enabled = true;
            }
            else
            {
                btnProperties.Enabled = false;
                buttonStart.Enabled = false;
            }
        }

        /// <summary>
        /// Device selection. If a device was selected, the new
        /// device state will be saved.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnDevice_Click(object sender, EventArgs e)
        {
            _grabber.StreamStop();
            var devInfo = Dialogs.ShowSelectDeviceDialog(this);
            if (devInfo != null)
            {
                _grabber.DeviceClose();
                try
                {
                    _grabber.DeviceOpen(devInfo);
                }
                catch (Exception exceptionMessage)
                {
                    MessageBox.Show(exceptionMessage.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            if (_grabber.IsDeviceValid)
            {
                _grabber.StreamSetup(ic4display, StreamSetupOption.AcquisitionStart);
            }
            UpdateControls();
        }

        /// <summary>
        /// Show the device property dialog. If the dialog was closed by OK button
        /// the new settings are saved into the device file
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnProperties_Click(object sender, EventArgs e)
        {
            if (_grabber.IsDeviceValid)
            {
                if (_sink != null)  // Presence of sink indicates capture is active
                {
                    // Pass property map so that the dialog cannot restart the stream
                    Dialogs.ShowPropertyMapDialog(_grabber.DevicePropertyMap, this);
                }
                else
                {
                    // Pass grabber itself so that the dialog can restart the stream
                    Dialogs.ShowDevicePropertyDialog(_grabber, this, PropertyDialogFlags.AllowStreamRestart);
                }
            }
        }

        private void buttonBrowse_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog.ShowDialog() == DialogResult.OK)
            {
                _destinationDirectory = folderBrowserDialog.SelectedPath;
                textBoxDestinationFolder.Text = _destinationDirectory;
            }
        }



        private async void buttonStart_Click(object sender, EventArgs e)
        {
            if (_sink == null)
            {
                try
                {
                    Directory.CreateDirectory(textBoxDestinationFolder.Text);
                }
                catch (Exception exceptionMessage)
                {
                    MessageBox.Show(exceptionMessage.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                _grabber.StreamStop();
                _sink = new QueueSink();
                _num_processed = 0;
                _frame_number = 0;
                _sink.FramesQueued += _sink_FramesQueued;
                _sink.SinkConnected += sinkConnected;
                _grabber.StreamSetup(_sink, ic4display, StreamSetupOption.AcquisitionStart);
                UpdateControls();
            }
            else
            {
                Cursor.Current = Cursors.WaitCursor;
                buttonStart.Enabled = false;

                // Stop the device
                _grabber.AcquisitionStop();

                // Make sure we don't close the window while the cleanup thread runs
                _cleanup_active = true;

                await Task.Run(() =>
                {
                    while (!_cancel_cleanup)
                    {
                        lock (_frames_queued_mtx)
                        {
                            // Wait for the sink's output queue to be emptied by repeated framesQueued invocations
                            var qs = _sink.QueueSizes;
                            if (qs.OutputQueueLength == 0)
                                break;
                        }
                        Thread.Sleep(1);
                    }
                    _cleanup_active = false;
                });

                Cursor.Current = Cursors.Default;
                _grabber.StreamStop();
                _sink.Dispose();
                _sink = null;

                //Check if display was disposed by Close Event
                if (!ic4display.IsDisposed)
                    _grabber.StreamSetup(ic4display, StreamSetupOption.AcquisitionStart);

                UpdateControls();
            }
        }

        private void sinkConnected(object? sender, QueueSinkConnectedEventArgs e)
        {
            var bpp = e.ImageType.PixelFormat.GetBitsPerPixel();
            var imageSize = e.ImageType.Width * e.ImageType.Height * bpp / 8;
            int numBuffers = (int)(_bufferMemory * 1024L * 1024L / imageSize);
            if (numBuffers > e.MinBuffersRequired)
            {
                _num_total = numBuffers;
                e.Sink.AllocAndQueueBuffers(numBuffers);
            }
            else
            {
                _num_total = e.MinBuffersRequired;
            }
        }

        void UpdateCaptureInfo()
        {
            progressBarFilledBuffers.Maximum = _num_total;
            progressBarFilledBuffers.Value = _num_filled;
            labelFilledBuffers.Text = $"{_num_filled} / {_num_total}";

            progressBarFreeBuffers.Maximum = _num_total;
            progressBarFreeBuffers.Value = _num_free;
            labelFreeBuffers.Text = $"{_num_free} / {_num_total}";

            var stats = _grabber.StreamStatistics;
            var numDropped = stats.DeviceTransmissionError + stats.DeviceUnderrun + stats.SinkIgnored + stats.SinkUnderrun;
            labelCaptureInfo.Text = $"Saved Images: {_num_processed} Frames Dropped: {numDropped}";
        }

        void ReadSettings()
        {
            _destinationDirectory = Properties.Settings.Default.DestinationDirectory;
            if (_destinationDirectory == "")
            {
                _destinationDirectory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyPictures), "HighSpeedCapture");
            }
            _bufferMemory = Properties.Settings.Default.BufferMemory;
            if (File.Exists("device.xml"))
            {
                try
                {
                    _grabber.DeviceOpenFromState("device.xml");
                    _grabber.StreamSetup(ic4display, StreamSetupOption.AcquisitionStart);
                }
                catch
                {
                    // Ignore errors, camera might be unavailable
                    _grabber.DeviceClose();
                }
            }
            UpdateControls();
        }

        void SaveSettings()
        {
            Properties.Settings.Default.DestinationDirectory = _destinationDirectory;
            Properties.Settings.Default.BufferMemory = _bufferMemory;
            Properties.Settings.Default.Save();
            if (_grabber.IsDeviceOpen)
                _grabber.DeviceSaveState("device.xml");
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            // Cancel and wait for possible cleanup thread
            _cancel_cleanup = true;
            while (_cleanup_active)
            {
                Thread.Sleep(1);
            }
            // Make sure the stream is stopped so that framesQueued is no longer running
            _grabber.StreamStop();
            SaveSettings();
        }

        private void textBoxBufferMemory_TextChanged(object sender, EventArgs e)
        {
            textBoxBufferMemory_Validating(sender, new CancelEventArgs());
        }

        private void textBoxBufferMemory_Validating(object sender, CancelEventArgs e)
        {
            if (!Int32.TryParse(textBoxBufferMemory.Text, out _bufferMemory))
            {
                e.Cancel = true;
                textBoxBufferMemory.BackColor = Color.Red;
            }
            else
            {
                textBoxBufferMemory.BackColor = SystemColors.Window;
            }
        }
    }
}

