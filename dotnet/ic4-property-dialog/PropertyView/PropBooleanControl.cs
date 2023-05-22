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
        private CheckBox checkBox_;

        public PropBooleanControl(ic4.Grabber grabber, ic4.PropBoolean property) : base(grabber, property)
        {
            InitializeComponent();
            UpdateAll();
        }

        internal override void UpdateAll()
        {
            BlockSignals = true;
            var propBool = Property as ic4.PropBoolean;
            checkBox_.Enabled = !base.IsLocked && !base.IsReadonly;
            checkBox_.Checked = propBool.Value;
            BlockSignals = false;
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();
            this.Height = Appearance.ControlHeight;
            checkBox_ = new CheckBox()
            {
                Dock = DockStyle.Fill,
                Text = string.Empty,
                TabIndex = AutoTabIndex,
                TabStop= true,
                Parent= this,
            };
            checkBox_.CheckedChanged += CheckBox__CheckedChanged;
            this.TabStop= false;
            this.ResumeLayout(false);
        }

        private void CheckBox__CheckedChanged(object sender, EventArgs e)
        {
            if (BlockSignals)
                return;

            base.Value = checkBox_.Checked;
        }
    }
}
