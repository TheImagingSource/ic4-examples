using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;

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
        private System.Windows.Forms.TextBox textBox1;
        private DownButton cmdDown;
        private UpButton cmdUp;
        private System.Windows.Forms.Panel panel1;

        private Color bgColor_ = Color.White;
        private int _value = 0;
        private string _displayText = string.Empty;
        private bool showButtons_ = true;
        private bool readOnly_ = true;

        public event EventHandler TextInput;
        public event EventHandler ValueChanged;

        public int Minimum { get; set; }
        public int Maximum { get; set; }

        public int StepSize { get; set; }

        public CustomNumericUpDown()
        {
            InitializeComponent();
            bgColor_ = this.BackColor;
        }

        private void InitializeComponent()
        {
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.cmdUp = new UpButton();
            this.cmdDown = new DownButton();
            this.panel1.SuspendLayout();
            this.SuspendLayout();

            int width = (int)(50.0f * WinformsUtil.Scaling);
            int height = (int)(21.0f * WinformsUtil.Scaling);
            if (height % 2 == 0)
            {
                height--;
            }
            this.Size = new Size(width, height);
           

            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.panel1.Controls.Add(this.cmdUp);
            this.panel1.Controls.Add(this.cmdDown);
            this.panel1.ForeColor = System.Drawing.SystemColors.ControlLight;
            this.panel1.Size = new System.Drawing.Size((int)(16.0f * WinformsUtil.Scaling + 0.5f), height-1);
            this.panel1.Location = new Point(width - panel1.Width - 1, 0);
            this.panel1.Name = "panel1";
            this.panel1.TabIndex = 4;

            int buttonHeight = (panel1.Height - 3 ) / 2 + 1 ;

            // 
            // cmdUp
            // 
            this.cmdUp.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cmdUp.ForeColor = System.Drawing.SystemColors.Control;
            this.cmdUp.Location = new Point(0, 0);
            this.cmdUp.Size = new Size(panel1.Width, buttonHeight);
            this.cmdUp.TabIndex = 3;
            this.cmdUp.UseVisualStyleBackColor = true;
            this.cmdUp.Click += CmdUp_Click;
            // 
            // cmdDown
            // 
            this.cmdDown.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cmdDown.ForeColor = System.Drawing.SystemColors.Control;
            this.cmdDown.Size = new Size(panel1.Width, buttonHeight);
            this.cmdDown.Location = new Point(0, cmdUp.Location.Y + cmdUp.Height );
            this.cmdDown.TabIndex = 2;
            this.cmdDown.UseVisualStyleBackColor = true;
            this.cmdDown.Click += CmdDown_Click;
            // 
            // textBox1
            // 
            this.textBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top)
            | System.Windows.Forms.AnchorStyles.Left)
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBox1.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBox1.Font = new System.Drawing.Font("Microsoft Sans Serif",
                Appearance.ControlFontSize,
                System.Drawing.FontStyle.Regular,
                System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBox1.Location = new Point(1, height/2 - textBox1.Height/2);
            this.textBox1.Size = new System.Drawing.Size(width - cmdDown.Width - 4, height );
            this.textBox1.TabIndex = 1;
            textBox1.LostFocus += TextBox1_LostFocus;
            textBox1.KeyDown += TextBox1_KeyDown;
            // 
            // CustomNumericUpDown
            // 
            this.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ForeColor = System.Drawing.SystemColors.ControlDark;
            this.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.textBox1);
            this.ForeColor = System.Drawing.SystemColors.ControlDark;
            this.Maximum = 100;
            this.Minimum = 0;
            this.Value = 0;

            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();
            
        }

        private void TextBox1_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
            {
                if (_displayText != textBox1.Text)
                {
                    _displayText = textBox1.Text;
                    TextInput?.Invoke(sender, EventArgs.Empty);   
                }
            }
        }

        private void TextBox1_LostFocus(object sender, EventArgs e)
        {
            if (_displayText != textBox1.Text)
            {
                _displayText = textBox1.Text;
                TextInput?.Invoke(sender, EventArgs.Empty);
            }
        }

        public bool ShowButtons
        {
            set
            {
                showButtons_ = value;
                panel1.Visible = value;
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
                textBox1.ReadOnly = value;
                this.cmdDown.Enabled = !value;
                this.cmdUp.Enabled = !value;
                this.BackColor = textBox1.BackColor;
            }
            get
            {
                return readOnly_;
            }
        }

        private void CmdDown_Click(object sender, EventArgs e)
        {
            Value-= StepSize;
            ValueChanged?.Invoke(sender, e);
        }

        private void CmdUp_Click(object sender, EventArgs e)
        {
            Value+= StepSize;
            ValueChanged?.Invoke(sender, e);
        }

        public int Value
        {
            get
            {
                return _value;
            }
            set
            {
                _value = Math.Min(Maximum, Math.Max(Minimum, value));
            }
        }

        public string DisplayText
        {
            get
            {
                return _displayText;
            }
            set
            {
                _displayText = value;
                textBox1.Text = value;
            }
        }

    }
}
