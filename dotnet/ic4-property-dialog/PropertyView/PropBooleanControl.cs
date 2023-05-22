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
    internal class PropBooleanControl : PropControl<bool>
    {
        private CheckBox CheckBox_;

        public PropBooleanControl(ic4.Grabber grabber, ic4.PropBoolean property) : base(grabber, property)
        {
            InitializeComponent();
            UpdateAll();
        }

        internal override void UpdateAll()
        {
            BlockSignals = true;
            var propBool = Property as ic4.PropBoolean;
            CheckBox_.Enabled = !base.IsLocked && !base.IsReadonly;
            CheckBox_.Checked = propBool.Value;
            BlockSignals = false;
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();
            this.Height = Appearance.ControlHeight;
            CheckBox_ = new CheckBox()
            {
                Dock = DockStyle.Fill,
                Text = string.Empty,
                TabIndex = AutoTabIndex,
                TabStop= true,
                Parent= this,
            };
            CheckBox_.CheckedChanged += CheckBox__CheckedChanged;
            this.TabStop= false;
            this.ResumeLayout(false);
        }

        private void CheckBox__CheckedChanged(object sender, EventArgs e)
        {
            if (BlockSignals)
                return;

            base.Value = CheckBox_.Checked;
        }
    }
}
