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
    internal class PropEnumerationControl : PropControl<long>
    {
        public class ComboBoxItem
        {
            public long Value { get; set;  }
            public string Text { get; set;  }

            public ComboBoxItem(long val, string text)
            {
                Value = val;
                Text = text;
            }

            public override string ToString()
            {
                return Text;
            }

        }

        private System.Windows.Forms.ComboBox comboBox1;


        public PropEnumerationControl(ic4.Grabber grabber, ic4.PropEnumeration property) : base(grabber, property)
        {
            InitializeComponent();
            UpdateAll();
        }

        internal override void UpdateAll()
        {
            BlockSignals = true;

            var propEnumeration = Property as ic4.PropEnumeration;

            comboBox1.Items.Clear();

            var selectedEntry = propEnumeration.SelectedEntry;
            bool selectedFound = false;

            foreach (var entry in propEnumeration.Entries)
            {
                long val = 0;
                try
                {
                    val = entry.Value;
                }
                catch
                {
                    continue;
                }

                if (!entry.IsAvailable)
                    continue;

                comboBox1.Items.Add(new ComboBoxItem(val, entry.DisplayName)); 

                if(entry.Value == selectedEntry.Value)
                { 
                    comboBox1.SelectedIndex = comboBox1.Items.Count-1;
                    selectedFound = true;
                }
            }

            if(!selectedFound)
            {
                comboBox1.SelectedIndex = -1;
            }


            bool isLocked = base.IsLocked;
            bool isReadonly = base.IsReadonly;

            comboBox1.Enabled = !isReadonly;

            BlockSignals = false;
        }


        private void InitializeComponent()
        {
            int height = (int)(21.0f * WinformsUtil.Scaling);
            if (height % 2 == 0)
            {
                height--;
            }

            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // comboBox1
            // 
            this.comboBox1.Dock = DockStyle.Fill;
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 
                Appearance.ControlFontSize, 
                System.Drawing.FontStyle.Regular, 
                System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(0, 0);
            this.comboBox1.TabIndex = 5;
            this.comboBox1.SelectedIndexChanged += ComboBox1_SelectedIndexChanged;
            // 
            // PropEnumerationControl
            // 
            this.Controls.Add(this.comboBox1);
            this.Size = new System.Drawing.Size(105, height);
            this.ResumeLayout(false);
        }

        private void ComboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (BlockSignals)
                return;

            try
            {
                base.Value = (comboBox1.SelectedItem as ComboBoxItem).Value;
            }
            catch(Exception ex)
            {
                System.Console.WriteLine(ex.Message);
            }
        }
    }
}
