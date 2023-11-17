namespace DialogApp
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            display1 = new ic4.WinForms.Display();
            btnSelectDevice = new Button();
            btnDeviceProperties = new Button();
            SuspendLayout();
            // 
            // display1
            // 
            display1.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            display1.Location = new Point(12, 12);
            display1.Name = "display1";
            display1.RenderHeight = 480;
            display1.RenderLeft = 0;
            display1.RenderPosition = ic4.DisplayRenderPosition.StretchTopLeft;
            display1.RenderTop = 0;
            display1.RenderWidth = 640;
            display1.Size = new Size(841, 649);
            display1.TabIndex = 0;
            // 
            // btnSelectDevice
            // 
            btnSelectDevice.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            btnSelectDevice.Location = new Point(859, 12);
            btnSelectDevice.Name = "btnSelectDevice";
            btnSelectDevice.Size = new Size(159, 29);
            btnSelectDevice.TabIndex = 1;
            btnSelectDevice.Text = "Select Device...";
            btnSelectDevice.UseVisualStyleBackColor = true;
            btnSelectDevice.Click += btnSelectDevice_Click;
            // 
            // btnDeviceProperties
            // 
            btnDeviceProperties.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            btnDeviceProperties.Location = new Point(859, 47);
            btnDeviceProperties.Name = "btnDeviceProperties";
            btnDeviceProperties.Size = new Size(159, 29);
            btnDeviceProperties.TabIndex = 2;
            btnDeviceProperties.Text = "Device Properties...";
            btnDeviceProperties.UseVisualStyleBackColor = true;
            btnDeviceProperties.Click += btnDeviceProperties_Click;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(1030, 673);
            Controls.Add(btnDeviceProperties);
            Controls.Add(btnSelectDevice);
            Controls.Add(display1);
            Name = "Form1";
            Text = "IC4 Dialog Application";
            Load += Form1_Load;
            ResumeLayout(false);
        }

        #endregion

        private ic4.WinForms.Display display1;
        private Button btnSelectDevice;
        private Button btnDeviceProperties;
    }
}