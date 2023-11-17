namespace DialogApp
{
    public partial class Form1 : Form
    {
        private ic4.Grabber grabber = new ic4.Grabber();

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // Show a dialog allowing the user to select the device opened by the grabber object.
            ic4.WinForms.Dialogs.ShowDeviceDialog(grabber, this);

            // If no device was selected, close the application.
            if (!grabber.IsDeviceValid)
            {
                Close();
                return;
            }

            // Register a device-lost notification handler.
            grabber.DeviceLost += Grabber_DeviceLost;

            // Start data stream to display.
            grabber.StreamSetup(display1);
        }

        private void Grabber_DeviceLost(object? sender, EventArgs e)
        {
            // DeviceLost event is executed on a thread owned by the Grabber object.
            // Need to use Invoke to do UI operations.
            Invoke(
                () => MessageBox.Show(this, $"Device Lost: {grabber.DeviceInfo.ModelName}", "DialogApp", MessageBoxButtons.OK, MessageBoxIcon.Error)
            );
        }

        private void btnSelectDevice_Click(object sender, EventArgs e)
        {
            // Allow the user to select a different device.
            var selectedDeviceInfo = ic4.WinForms.Dialogs.ShowSelectDeviceDialog(this);

            // Check whether a new device was selected.
            if (selectedDeviceInfo != null && selectedDeviceInfo != grabber.DeviceInfo)
            {
                // Close the currently opened device.
                grabber.DeviceClose();

                // Open the newly selected device.
                grabber.DeviceOpen(selectedDeviceInfo);

                // Start data stream to display.
                grabber.StreamSetup(display1);
            }
        }

        private void btnDeviceProperties_Click(object sender, EventArgs e)
        {
            // Show the property dialog for the device's property map.
            // Pass AllowStreamRestart flag so that the user can change properties that would
            // be locked by the active data stream, such as PixelFormat, Width or Height.
            ic4.WinForms.Dialogs.ShowDevicePropertyDialog(grabber, this, ic4.WinForms.PropertyDialogFlags.AllowStreamRestart);
        }
    }
}