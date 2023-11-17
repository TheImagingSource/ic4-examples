namespace DialogApp_framework45
{
    partial class Form1
    {
        /// <summary>
        /// Erforderliche Designervariable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Verwendete Ressourcen bereinigen.
        /// </summary>
        /// <param name="disposing">True, wenn verwaltete Ressourcen gelöscht werden sollen; andernfalls False.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Vom Windows Form-Designer generierter Code

        /// <summary>
        /// Erforderliche Methode für die Designerunterstützung.
        /// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
        /// </summary>
        private void InitializeComponent()
        {
            this.btnSelectDevice = new System.Windows.Forms.Button();
            this.btnDeviceProperties = new System.Windows.Forms.Button();
            this.display1 = new ic4.WinForms.Display();
            this.SuspendLayout();
            // 
            // btnSelectDevice
            // 
            this.btnSelectDevice.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnSelectDevice.Location = new System.Drawing.Point(833, 13);
            this.btnSelectDevice.Margin = new System.Windows.Forms.Padding(4);
            this.btnSelectDevice.Name = "btnSelectDevice";
            this.btnSelectDevice.Size = new System.Drawing.Size(160, 28);
            this.btnSelectDevice.TabIndex = 0;
            this.btnSelectDevice.Text = "Select Device...";
            this.btnSelectDevice.UseVisualStyleBackColor = true;
            // 
            // btnDeviceProperties
            // 
            this.btnDeviceProperties.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnDeviceProperties.Location = new System.Drawing.Point(833, 48);
            this.btnDeviceProperties.Margin = new System.Windows.Forms.Padding(4);
            this.btnDeviceProperties.Name = "btnDeviceProperties";
            this.btnDeviceProperties.Size = new System.Drawing.Size(160, 28);
            this.btnDeviceProperties.TabIndex = 1;
            this.btnDeviceProperties.Text = "Device Properties...";
            this.btnDeviceProperties.UseVisualStyleBackColor = true;
            // 
            // display1
            // 
            this.display1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.display1.Location = new System.Drawing.Point(16, 15);
            this.display1.Margin = new System.Windows.Forms.Padding(4, 2, 4, 2);
            this.display1.Name = "display1";
            this.display1.RenderHeight = 480;
            this.display1.RenderLeft = 0;
            this.display1.RenderPosition = ic4.DisplayRenderPosition.TopLeft;
            this.display1.RenderTop = 0;
            this.display1.RenderWidth = 640;
            this.display1.Size = new System.Drawing.Size(809, 644);
            this.display1.TabIndex = 2;
            this.display1.TabStop = false;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1006, 673);
            this.Controls.Add(this.display1);
            this.Controls.Add(this.btnDeviceProperties);
            this.Controls.Add(this.btnSelectDevice);
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.Name = "Form1";
            this.Text = "IC4 Dialog Application";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnSelectDevice;
        private System.Windows.Forms.Button btnDeviceProperties;
        private ic4.WinForms.Display display1;
    }
}

