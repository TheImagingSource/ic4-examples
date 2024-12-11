namespace  Graphic_Overlay
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            ic4Display = new ic4.WinForms.Display();
            btnDevice = new Button();
            btnProperties = new Button();
            btnStartStop = new Button();
            SuspendLayout();
            // 
            // ic4Display
            // 
            ic4Display.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            ic4Display.Location = new Point(12, 11);
            ic4Display.Margin = new Padding(3, 2, 3, 2);
            ic4Display.Name = "ic4Display";
            ic4Display.RenderHeight = 480;
            ic4Display.RenderLeft = 0;
            ic4Display.RenderPosition = ic4.DisplayRenderPosition.StretchCenter;
            ic4Display.RenderTop = 0;
            ic4Display.RenderWidth = 640;
            ic4Display.Size = new Size(640, 480);
            ic4Display.TabIndex = 0;
            // 
            // btnDevice
            // 
            btnDevice.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            btnDevice.Location = new Point(658, 12);
            btnDevice.Name = "btnDevice";
            btnDevice.Size = new Size(105, 29);
            btnDevice.TabIndex = 1;
            btnDevice.Text = "Device";
            btnDevice.UseVisualStyleBackColor = true;
            btnDevice.Click += btnDevice_Click;
            // 
            // btnProperties
            // 
            btnProperties.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            btnProperties.Enabled = false;
            btnProperties.Location = new Point(658, 47);
            btnProperties.Name = "btnProperties";
            btnProperties.Size = new Size(105, 29);
            btnProperties.TabIndex = 2;
            btnProperties.Text = "Properties";
            btnProperties.UseVisualStyleBackColor = true;
            btnProperties.Click += btnProperties_Click;
            // 
            // btnStartStop
            // 
            btnStartStop.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            btnStartStop.Enabled = false;
            btnStartStop.Location = new Point(658, 82);
            btnStartStop.Name = "btnStartStop";
            btnStartStop.Size = new Size(105, 29);
            btnStartStop.TabIndex = 3;
            btnStartStop.Text = "Start Stream";
            btnStartStop.UseVisualStyleBackColor = true;
            btnStartStop.Click += btnStartStop_Click;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(770, 500);
            Controls.Add(btnStartStop);
            Controls.Add(btnProperties);
            Controls.Add(btnDevice);
            Controls.Add(ic4Display);
            Icon = (Icon)resources.GetObject("$this.Icon");
            Name = "Form1";
            Text = "Form1";
            Load += Form1_Load;
            ResumeLayout(false);
        }

        #endregion

        private ic4.WinForms.Display ic4Display = null;
        private Button btnDevice;
        private Button btnProperties;
        private Button btnStartStop;
    }
}
