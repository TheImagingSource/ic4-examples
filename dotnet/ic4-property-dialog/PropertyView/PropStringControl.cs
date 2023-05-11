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
        }

        private void InitializeComponent()
        {
            textBox_ = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // textBox_
            // 
            textBox_.Dock = DockStyle.Fill;
            textBox_.Text = string.Empty;
            textBox_.Font = new System.Drawing.Font("Microsoft Sans Serif", 
                Appearance.ControlFontSize,
                System.Drawing.FontStyle.Regular,
                System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            // 
            // PropStringControl
            // 
            this.Controls.Add(this.textBox_);
            this.Size = new System.Drawing.Size(200, textBox_.Height);
            this.MaximumSize = new System.Drawing.Size(2048, textBox_.Height);
            this.ResumeLayout(false);
        }
    }
}
