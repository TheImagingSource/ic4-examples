using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;
using System.Runtime.InteropServices.WindowsRuntime;

namespace ic4.Examples
{
    [ToolboxItem(false)]
    public class UpButton : Button
    {
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            Point center = new Point(this.Width / 2, this.Height / 2);

            var b = Enabled ? Brushes.Black : new SolidBrush(SystemColors.ControlDark);
            var p = Enabled ? Pens.Black : new Pen(SystemColors.ControlDark);

            e.Graphics.FillRectangle(b, center.X, center.Y - 1, 1, 1);
            e.Graphics.DrawLine(p, center.X - 1, center.Y, center.X + 1, center.Y);
            e.Graphics.DrawLine(p, center.X - 2, center.Y + 1, center.X + 2, center.Y + 1);
        }
    }

    [ToolboxItem(false)]
    public class DownButton : Button
    {
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            Point center = new Point(this.Width / 2, this.Height / 2);

            var b = Enabled ? Brushes.Black : new SolidBrush(SystemColors.ControlDark);
            var p = Enabled ? Pens.Black : new Pen(SystemColors.ControlDark);

            e.Graphics.DrawLine(p, center.X - 2, center.Y - 1, center.X + 2, center.Y - 1);
            e.Graphics.DrawLine(p, center.X - 1, center.Y, center.X + 1, center.Y);
            e.Graphics.FillRectangle(b, center.X, center.Y + 1, 1, 1);
        }
    }

    [ToolboxItem(false)]
    internal class CustomNumericUpDown : UserControl
    {
        private TextBox textBox_;
        private DownButton cmdDown_;
        private UpButton cmdUp_;
        private Panel panel_;

        private Color bgColor_ = Color.White;
        private long value_ = 0;
        private string displayText_ = string.Empty;
        private bool showButtons_ = true;
        private bool readOnly_ = true;

        public event EventHandler TextInput;
        public event EventHandler ValueChanged;

        public long Minimum { get; set; }
        public long Maximum { get; set; }
        public long StepSize { get; set; } = 1;

        public CustomNumericUpDown()
        {
            InitializeComponent();
            bgColor_ = this.BackColor;
        }

        private void InitializeComponent()
        {
            this.textBox_ = new System.Windows.Forms.TextBox();
            this.panel_ = new System.Windows.Forms.Panel();
            this.cmdUp_ = new UpButton();
            this.cmdDown_ = new DownButton();
            this.panel_.SuspendLayout();
            this.SuspendLayout();

            int width = (int)(50.0f * WinformsUtil.Scaling);
            int height = Appearance.ControlHeight;
            this.Size = new Size(width, height);
           

            // 
            // panel1
            // 
            this.panel_.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            this.panel_.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.panel_.Controls.Add(this.cmdUp_);
            this.panel_.Controls.Add(this.cmdDown_);
            this.panel_.ForeColor = System.Drawing.SystemColors.ControlLight;
            this.panel_.Size = new System.Drawing.Size((int)(16.0f * WinformsUtil.Scaling + 0.5f), height - 2);
            this.panel_.Location = new Point(width - panel_.Width, 0);
            this.panel_.Name = "panel1";
            this.panel_.TabStop = false;

            int buttonHeight = (panel_.Height) / 2 + 1;

            // 
            // cmdUp
            // 
            this.cmdUp_.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            this.cmdUp_.ForeColor = System.Drawing.SystemColors.Control;
            this.cmdUp_.Location = new Point(0, 0);
            this.cmdUp_.Size = new Size(panel_.Width, buttonHeight);
            this.cmdUp_.TabIndex = 3;
            this.cmdUp_.UseVisualStyleBackColor = true;
            this.cmdUp_.Click += CmdUp_Click;
            this.cmdUp_.TabStop= false;

            // 
            // cmdDown
            // 
            this.cmdDown_.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            this.cmdDown_.ForeColor = System.Drawing.SystemColors.Control;
            this.cmdDown_.Size = new Size(panel_.Width, buttonHeight);
            this.cmdDown_.Location = new Point(0, cmdUp_.Location.Y - 1 + cmdUp_.Height );
            this.cmdDown_.TabIndex = 2;
            this.cmdDown_.UseVisualStyleBackColor = true;
            this.cmdDown_.Click += CmdDown_Click;
            this.cmdDown_.TabStop = false;

            // 
            // textBox1
            // 
            this.textBox_.Anchor =AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            this.textBox_.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBox_.Font = new System.Drawing.Font("Microsoft Sans Serif",
                Appearance.ControlFontSize,
                System.Drawing.FontStyle.Regular,
                System.Drawing.GraphicsUnit.Point, ((byte)(0)));            
            this.textBox_.Size = new System.Drawing.Size(width - cmdDown_.Width - WinformsUtil.Scale(6), height );
            this.textBox_.Location = new Point(WinformsUtil.Scale(3), height / 2 - 1 - textBox_.Height / 2);
            this.textBox_.TabIndex = 0;
            textBox_.LostFocus += TextBox1_LostFocus;
            textBox_.KeyDown += TextBox1_KeyDown;

            // 
            // CustomNumericUpDown
            // 
            this.BackColor = SystemColors.ControlLightLight;
            this.BorderStyle = BorderStyle.FixedSingle;
            this.ForeColor = SystemColors.ControlDark;
            this.Margin = new Padding(4, 5, 4, 5);
            this.Controls.Add(this.panel_);
            this.Controls.Add(this.textBox_);
            this.ForeColor = SystemColors.ControlDark;
            this.Maximum = 100;
            this.Minimum = 0;
            this.Value = 0;
            this.panel_.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

            this.MaximumSize = new System.Drawing.Size(int.MaxValue, Height);
            this.MinimumSize = new System.Drawing.Size(0, Height);
        }

        private void TextBox1_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
            {
                if (displayText_ != textBox_.Text)
                {
                    displayText_ = textBox_.Text;
                    TextInput?.Invoke(sender, EventArgs.Empty);   
                }
            }
        }

        private void TextBox1_LostFocus(object sender, EventArgs e)
        {
            if (displayText_ != textBox_.Text)
            {
                displayText_ = textBox_.Text;
                TextInput?.Invoke(sender, EventArgs.Empty);
            }
        }

        public bool ShowButtons
        {
            set
            {
                showButtons_ = value;
                panel_.Visible = value;
            }
            get
            {
                return showButtons_;
            }
        }

        public bool ReadOnly
        {
            set
            {
                readOnly_ = value;
                textBox_.ReadOnly = value;
                this.cmdDown_.Enabled = !value;
                this.cmdUp_.Enabled = !value;
                this.BackColor = textBox_.BackColor;
            }
            get
            {
                return readOnly_;
            }
        }

        private void CmdDown_Click(object sender, EventArgs e)
        {
            value_ = Math.Min(Maximum, Math.Max(Minimum, value_ - StepSize));
            ValueChanged?.Invoke(sender, e);
        }

        private void CmdUp_Click(object sender, EventArgs e)
        {
            value_ = Math.Min(Maximum, Math.Max(Minimum, value_ + StepSize));
            ValueChanged?.Invoke(sender, e);
        }

        public long Value
        {
            get
            {
                return value_;
            }
            set
            {
                value_ = Math.Min(Maximum, Math.Max(Minimum, value));
            }
        }


        public string DisplayText
        {
            get
            {
                return displayText_;
            }
            set
            {
                displayText_ = value;
                textBox_.Text = value;
            }
        }

    }
}
