namespace DoLP_Segmentation
{
    partial class MainForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            label1 = new Label();
            label2 = new Label();
            lblDoLPThreshold = new Label();
            lblIntensityThreshold = new Label();
            label3 = new Label();
            tbDoLPThreshold = new TrackBar();
            tbIntensityThreshold = new TrackBar();
            ic4Display = new ic4.WinForms.Display();
            ((System.ComponentModel.ISupportInitialize)tbDoLPThreshold).BeginInit();
            ((System.ComponentModel.ISupportInitialize)tbIntensityThreshold).BeginInit();
            SuspendLayout();
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(12, 19);
            label1.Name = "label1";
            label1.Size = new Size(91, 15);
            label1.TabIndex = 0;
            label1.Text = "DoLP Threshold";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(12, 70);
            label2.Name = "label2";
            label2.Size = new Size(108, 15);
            label2.TabIndex = 1;
            label2.Text = "Intensity Threshold";
            // 
            // lblDoLPThreshold
            // 
            lblDoLPThreshold.AutoSize = true;
            lblDoLPThreshold.Location = new Point(300, 19);
            lblDoLPThreshold.Name = "lblDoLPThreshold";
            lblDoLPThreshold.Size = new Size(25, 15);
            lblDoLPThreshold.TabIndex = 2;
            lblDoLPThreshold.Text = "128";
            // 
            // lblIntensityThreshold
            // 
            lblIntensityThreshold.AutoSize = true;
            lblIntensityThreshold.Location = new Point(300, 70);
            lblIntensityThreshold.Name = "lblIntensityThreshold";
            lblIntensityThreshold.Size = new Size(25, 15);
            lblIntensityThreshold.TabIndex = 3;
            lblIntensityThreshold.Text = "128";
            // 
            // label3
            // 
            label3.BackColor = SystemColors.Info;
            label3.BorderStyle = BorderStyle.FixedSingle;
            label3.Font = new Font("Microsoft Sans Serif", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label3.Location = new Point(338, 19);
            label3.Name = "label3";
            label3.Size = new Size(406, 82);
            label3.TabIndex = 4;
            label3.Text = resources.GetString("label3.Text");
            // 
            // tbDoLPThreshold
            // 
            tbDoLPThreshold.Location = new Point(126, 19);
            tbDoLPThreshold.Maximum = 255;
            tbDoLPThreshold.Name = "tbDoLPThreshold";
            tbDoLPThreshold.Size = new Size(168, 45);
            tbDoLPThreshold.TabIndex = 5;
            tbDoLPThreshold.Value = 128;
            tbDoLPThreshold.ValueChanged += tbDoLPThreshold_ValueChanged;
            // 
            // tbIntensityThreshold
            // 
            tbIntensityThreshold.Location = new Point(126, 70);
            tbIntensityThreshold.Maximum = 255;
            tbIntensityThreshold.Name = "tbIntensityThreshold";
            tbIntensityThreshold.Size = new Size(168, 45);
            tbIntensityThreshold.TabIndex = 6;
            tbIntensityThreshold.Value = 128;
            tbIntensityThreshold.ValueChanged += tbIntensityThreshold_ValueChanged;
            // 
            // ic4Display
            // 
            ic4Display.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            ic4Display.Location = new Point(12, 120);
            ic4Display.Margin = new Padding(3, 2, 3, 2);
            ic4Display.Name = "ic4Display";
            ic4Display.RenderHeight = 480;
            ic4Display.RenderLeft = 0;
            ic4Display.RenderPosition = ic4.DisplayRenderPosition.TopLeft;
            ic4Display.RenderTop = 0;
            ic4Display.RenderWidth = 640;
            ic4Display.Size = new Size(732, 461);
            ic4Display.TabIndex = 7;
            // 
            // MainForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(756, 592);
            Controls.Add(ic4Display);
            Controls.Add(tbIntensityThreshold);
            Controls.Add(tbDoLPThreshold);
            Controls.Add(label3);
            Controls.Add(lblIntensityThreshold);
            Controls.Add(lblDoLPThreshold);
            Controls.Add(label2);
            Controls.Add(label1);
            MinimumSize = new Size(772, 631);
            Name = "MainForm";
            Text = "Polarization Demo: DoLP Segmentation";
            Load += MainForm_Load;
            ((System.ComponentModel.ISupportInitialize)tbDoLPThreshold).EndInit();
            ((System.ComponentModel.ISupportInitialize)tbIntensityThreshold).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label label1;
        private Label label2;
        private Label lblDoLPThreshold;
        private Label lblIntensityThreshold;
        private Label label3;
        private TrackBar tbDoLPThreshold;
        private TrackBar tbIntensityThreshold;
        private ic4.WinForms.Display ic4Display;
    }
}
