namespace HighSpeedCapture
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.btnDevice = new System.Windows.Forms.Button();
            this.ic4display = new ic4.WinForms.Display();
            this.btnProperties = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.labelFilledBuffers = new System.Windows.Forms.Label();
            this.labelFreeBuffers = new System.Windows.Forms.Label();
            this.labelCaptureInfo = new System.Windows.Forms.Label();
            this.buttonStart = new System.Windows.Forms.Button();
            this.buttonBrowse = new System.Windows.Forms.Button();
            this.label5 = new System.Windows.Forms.Label();
            this.progressBarFilledBuffers = new System.Windows.Forms.ProgressBar();
            this.progressBarFreeBuffers = new System.Windows.Forms.ProgressBar();
            this.textBoxBufferMemory = new System.Windows.Forms.TextBox();
            this.textBoxDestinationFolder = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnDevice
            // 
            this.btnDevice.Location = new System.Drawing.Point(6, 11);
            this.btnDevice.Margin = new System.Windows.Forms.Padding(2);
            this.btnDevice.Name = "btnDevice";
            this.btnDevice.Size = new System.Drawing.Size(80, 23);
            this.btnDevice.TabIndex = 1;
            this.btnDevice.Text = "Device";
            this.btnDevice.UseVisualStyleBackColor = true;
            this.btnDevice.Click += new System.EventHandler(this.btnDevice_Click);
            // 
            // ic4display
            // 
            this.ic4display.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ic4display.Location = new System.Drawing.Point(6, 38);
            this.ic4display.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.ic4display.MinimumSize = new System.Drawing.Size(640, 480);
            this.ic4display.Name = "ic4display";
            this.ic4display.RenderHeight = 480;
            this.ic4display.RenderLeft = 0;
            this.ic4display.RenderPosition = ic4.DisplayRenderPosition.StretchCenter;
            this.ic4display.RenderTop = 0;
            this.ic4display.RenderWidth = 640;
            this.ic4display.Size = new System.Drawing.Size(640, 480);
            this.ic4display.TabIndex = 0;
            // 
            // btnProperties
            // 
            this.btnProperties.Enabled = false;
            this.btnProperties.Location = new System.Drawing.Point(90, 11);
            this.btnProperties.Margin = new System.Windows.Forms.Padding(2);
            this.btnProperties.Name = "btnProperties";
            this.btnProperties.Size = new System.Drawing.Size(80, 23);
            this.btnProperties.TabIndex = 2;
            this.btnProperties.Text = "Properties";
            this.btnProperties.UseVisualStyleBackColor = true;
            this.btnProperties.Click += new System.EventHandler(this.btnProperties_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.labelFilledBuffers);
            this.groupBox1.Controls.Add(this.labelFreeBuffers);
            this.groupBox1.Controls.Add(this.labelCaptureInfo);
            this.groupBox1.Controls.Add(this.buttonStart);
            this.groupBox1.Controls.Add(this.buttonBrowse);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.progressBarFilledBuffers);
            this.groupBox1.Controls.Add(this.progressBarFreeBuffers);
            this.groupBox1.Controls.Add(this.textBoxBufferMemory);
            this.groupBox1.Controls.Add(this.textBoxDestinationFolder);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(10, 527);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(642, 167);
            this.groupBox1.TabIndex = 4;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Save Images";
            // 
            // labelFilledBuffers
            // 
            this.labelFilledBuffers.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelFilledBuffers.AutoSize = true;
            this.labelFilledBuffers.Location = new System.Drawing.Point(561, 110);
            this.labelFilledBuffers.Name = "labelFilledBuffers";
            this.labelFilledBuffers.Size = new System.Drawing.Size(0, 13);
            this.labelFilledBuffers.TabIndex = 13;
            // 
            // labelFreeBuffers
            // 
            this.labelFreeBuffers.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelFreeBuffers.AutoSize = true;
            this.labelFreeBuffers.Location = new System.Drawing.Point(561, 81);
            this.labelFreeBuffers.Name = "labelFreeBuffers";
            this.labelFreeBuffers.Size = new System.Drawing.Size(0, 13);
            this.labelFreeBuffers.TabIndex = 12;
            // 
            // labelCaptureInfo
            // 
            this.labelCaptureInfo.AutoSize = true;
            this.labelCaptureInfo.Location = new System.Drawing.Point(101, 144);
            this.labelCaptureInfo.Name = "labelCaptureInfo";
            this.labelCaptureInfo.Size = new System.Drawing.Size(0, 13);
            this.labelCaptureInfo.TabIndex = 11;
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(9, 139);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 10;
            this.buttonStart.Text = "Start";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // buttonBrowse
            // 
            this.buttonBrowse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonBrowse.Location = new System.Drawing.Point(561, 20);
            this.buttonBrowse.Name = "buttonBrowse";
            this.buttonBrowse.Size = new System.Drawing.Size(75, 23);
            this.buttonBrowse.TabIndex = 9;
            this.buttonBrowse.Text = "Browse";
            this.buttonBrowse.UseVisualStyleBackColor = true;
            this.buttonBrowse.Click += new System.EventHandler(this.buttonBrowse_Click);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(210, 52);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(25, 13);
            this.label5.TabIndex = 8;
            this.label5.Text = "MiB";
            // 
            // progressBarFilledBuffers
            // 
            this.progressBarFilledBuffers.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBarFilledBuffers.Location = new System.Drawing.Point(104, 104);
            this.progressBarFilledBuffers.Name = "progressBarFilledBuffers";
            this.progressBarFilledBuffers.Size = new System.Drawing.Size(451, 23);
            this.progressBarFilledBuffers.TabIndex = 7;
            // 
            // progressBarFreeBuffers
            // 
            this.progressBarFreeBuffers.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBarFreeBuffers.Location = new System.Drawing.Point(104, 75);
            this.progressBarFreeBuffers.Name = "progressBarFreeBuffers";
            this.progressBarFreeBuffers.Size = new System.Drawing.Size(451, 23);
            this.progressBarFreeBuffers.TabIndex = 6;
            // 
            // textBoxBufferMemory
            // 
            this.textBoxBufferMemory.Location = new System.Drawing.Point(104, 49);
            this.textBoxBufferMemory.Name = "textBoxBufferMemory";
            this.textBoxBufferMemory.Size = new System.Drawing.Size(100, 20);
            this.textBoxBufferMemory.TabIndex = 5;
            this.textBoxBufferMemory.TextChanged += new System.EventHandler(this.textBoxBufferMemory_TextChanged);
            this.textBoxBufferMemory.Validating += new System.ComponentModel.CancelEventHandler(this.textBoxBufferMemory_Validating);
            // 
            // textBoxDestinationFolder
            // 
            this.textBoxDestinationFolder.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxDestinationFolder.Location = new System.Drawing.Point(104, 23);
            this.textBoxDestinationFolder.Name = "textBoxDestinationFolder";
            this.textBoxDestinationFolder.Size = new System.Drawing.Size(451, 20);
            this.textBoxDestinationFolder.TabIndex = 4;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 110);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(67, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Filled Buffers";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 81);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(64, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Free Buffers";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 52);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Buffer Memory";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 26);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(92, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Destination Folder";
            // 
            // folderBrowserDialog
            // 
            this.folderBrowserDialog.Description = "Select Destination Folder";
            this.folderBrowserDialog.RootFolder = System.Environment.SpecialFolder.MyComputer;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(664, 701);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.btnProperties);
            this.Controls.Add(this.ic4display);
            this.Controls.Add(this.btnDevice);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.MinimumSize = new System.Drawing.Size(680, 740);
            this.Name = "Form1";
            this.Text = "IC4 High Speed Capture";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnDevice;
        private ic4.WinForms.Display ic4display = null;
        private System.Windows.Forms.Button btnProperties;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ProgressBar progressBarFilledBuffers;
        private System.Windows.Forms.ProgressBar progressBarFreeBuffers;
        private System.Windows.Forms.TextBox textBoxBufferMemory;
        private System.Windows.Forms.TextBox textBoxDestinationFolder;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button buttonBrowse;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog;
        private System.Windows.Forms.Label labelFilledBuffers;
        private System.Windows.Forms.Label labelFreeBuffers;
        private System.Windows.Forms.Label labelCaptureInfo;
    }
}

