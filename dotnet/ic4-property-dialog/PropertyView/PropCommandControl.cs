using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace ic4.Examples
{
    [ToolboxItem(false)]
    internal class PropCommandControl : PropControl<string>
    {
        private System.Windows.Forms.Button button1;

        public PropCommandControl(ic4.Grabber grabber, ic4.PropCommand property) : base(grabber, property)
        {
            InitializeComponent();
            UpdateAll();
        }

        internal override void UpdateAll()
        {
            button1.Enabled = !base.IsLocked && !base.IsReadonly;
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();

            button1 = new System.Windows.Forms.Button()
            {
                Parent = this,
                Dock = DockStyle.Fill,
                UseVisualStyleBackColor = true,
                Text = "Excecute",
                Font = new System.Drawing.Font("Microsoft Sans Serif",
                   Appearance.ControlFontSize,
                   System.Drawing.FontStyle.Regular,
                   System.Drawing.GraphicsUnit.Point, ((byte)(0)))
            };
            button1.Click += Button1_Click;
           
            this.Size = new System.Drawing.Size(105, (int)(23.0f * WinformsUtil.Scaling));
            this.ResumeLayout(false);
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            base.Value = string.Empty;// calling prop.Execute();
        }
    }
}
