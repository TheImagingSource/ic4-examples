using ic4;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace ic4.Examples
{
    [ToolboxItem(false)]
    internal class PropStringControl : PropControl<string>
    {
        private System.Windows.Forms.TextBox textBox_;
        private System.Windows.Forms.Panel panel_;

        public PropStringControl(ic4.Grabber grabber, ic4.PropString property) : base(grabber, property)
        {
            InitializeComponent();
            UpdateAll();
        }

        internal override void UpdateAll()
        {
            var prop = Property as ic4.PropString;

            string val = prop.Value;
            bool isLocked = base.IsLocked;
            bool isReadonly = base.IsReadonly;

            BlockSignals = true;
            textBox_.Text = val;
            textBox_.ReadOnly = isReadonly || isLocked;
            BlockSignals = false;

            panel_.BackColor = textBox_.BackColor;
        }

        private void InitializeComponent()
        {            
            this.SuspendLayout();

            panel_ = new System.Windows.Forms.Panel();
            panel_.Dock = DockStyle.Fill;
            panel_.BorderStyle = BorderStyle.FixedSingle;
            panel_.BackColor = System.Drawing.SystemColors.ControlLightLight;
            panel_.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            panel_.ForeColor = System.Drawing.SystemColors.ControlDark;
            //panel.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.Controls.Add(panel_);

            // 
            // textBox_
            // 
            textBox_ = new System.Windows.Forms.TextBox();
            textBox_.Text = string.Empty;
            textBox_.Font = new System.Drawing.Font("Microsoft Sans Serif", 
                Appearance.ControlFontSize,
                System.Drawing.FontStyle.Regular,
                System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            textBox_.BorderStyle = BorderStyle.None;
            textBox_.Width = 200 - WinformsUtil.Scale(6);
            textBox_.Anchor = AnchorStyles.Left | AnchorStyles.Right | System.Windows.Forms.AnchorStyles.Top;
            textBox_.Location = new System.Drawing.Point(WinformsUtil.Scale(3), Appearance.ControlHeight / 2 - 1 - textBox_.Height / 2);

            // 
            // PropStringControl
            // 
            panel_.Controls.Add(this.textBox_);
            this.Size = new System.Drawing.Size(200, Appearance.ControlHeight);
            this.MaximumSize = new System.Drawing.Size(2048, Appearance.ControlHeight);
            this.MinimumSize = new System.Drawing.Size(0, Appearance.ControlHeight);
            this.ResumeLayout(false);
        }
    }
}
