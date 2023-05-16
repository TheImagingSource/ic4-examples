using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing;
using System.Text.RegularExpressions;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using System.Net.NetworkInformation;
using ic4;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Xml.Schema;
using System.Diagnostics.Eventing.Reader;

namespace ic4.Examples
{
    class PropIntegerControl : PropControl<long>
    {
        private long min_ = 0;
        private long max_ = 100;
        private long inc_ = 1;
        private long val_ = 0;
        private ic4.IntRepresentation representation_ = ic4.IntRepresentation.Linear;

        private System.Windows.Forms.TrackBar slider_;
        private CustomNumericUpDown spin_;
        private System.Windows.Forms.TextBox edit_;

        public PropIntegerControl(ic4.Grabber grabber, ic4.PropInteger property) : base(grabber, property)
        {
            representation_ = property.Representation;
            InitializeComponent();
            UpdateAll();
        }

        private const int SLIDER_MIN = 0;
        private const int SLIDER_MAX = 100000000;
        private const int SLIDER_TICKS = SLIDER_MAX - SLIDER_MIN;

        int lastTrackBarValue = 0;

        private long SliderPosToValue(int pos)
        {
            ulong rangelen = (ulong)(max_ - min_);
            System.Decimal frac = (System.Decimal)rangelen / SLIDER_TICKS;
            var offset = new System.Numerics.BigInteger(frac * (System.Decimal)pos);
            long val0 = (long)(min_ + offset);

            if (val0 > max_) val0 = max_;
            if (val0 < min_) val0 = min_;

            return val0;
        }

        private int ValueToSliderPosition(long value)
        {
            ulong rangelen = (ulong)(max_ - min_);
            int p = (int)((System.Decimal)(SLIDER_TICKS) / rangelen * (ulong)(value - min_));
            return p;
        }

        internal override void UpdateAll()
        {
            var propInt = Property as ic4.PropInteger;

            min_ = propInt.Minimum;
            max_ = propInt.Maximum;
            inc_ = propInt.Increment;
            val_ = propInt.Value;

            bool isLocked = base.IsLocked;
            bool isReadonly = base.IsReadonly;

            long winformsMin = Math.Max(long.MinValue, Math.Min(long.MaxValue, min_));
            long winformsMax = Math.Max(long.MinValue, Math.Min(long.MaxValue, max_));
            long value = Math.Min(winformsMax, Math.Max(winformsMin, val_));

            BlockSignals = true;

            if (slider_ != null)
            {
                slider_.Minimum = SLIDER_MIN;
                slider_.Maximum = SLIDER_MAX;
                slider_.SmallChange = 1;

                lastTrackBarValue = ValueToSliderPosition(value);
                slider_.Value = lastTrackBarValue;
                slider_.Enabled = !isLocked;
            }

            if (spin_ != null)
            {
                spin_.Minimum = min_;
                spin_.Maximum = max_;
                spin_.StepSize = inc_;
                spin_.Value = value;
                spin_.DisplayText = ValueToString(value, representation_);
                spin_.ReadOnly = isLocked || isReadonly;
                spin_.ShowButtons = !isReadonly;
            }

            if(edit_ != null)
            {
                edit_.Text = ValueToString(value, representation_);
                edit_.ReadOnly = isLocked || isReadonly;
            }

            BlockSignals = false;

        }

        private static string ValueToString(long value, IntRepresentation representation)
        {
            if (representation == IntRepresentation.HexNumber)
            {
                return "0x" + value.ToString("X4");
            }
            else if (representation == IntRepresentation.MacAddress)
            {
                ulong v0 = ((ulong)value >> 0) & 0xFF;
                ulong v1 = ((ulong)value >> 8) & 0xFF;
                ulong v2 = ((ulong)value >> 16) & 0xFF;
                ulong v3 = ((ulong)value >> 24) & 0xFF;
                ulong v4 = ((ulong)value >> 32) & 0xFF;
                ulong v5 = ((ulong)value >> 40) & 0xFF;
                return string.Format("{0}:{1}:{2}:{3}:{4}:{5}", v5, v4, v3, v2, v1, v0);
            }
            else if(representation == IntRepresentation.IP4Address)
            {
                ulong v0 = ((ulong)value >> 0) & 0xFF;
                ulong v1 = ((ulong)value >> 8) & 0xFF;
                ulong v2 = ((ulong)value >> 16) & 0xFF;
                ulong v3 = ((ulong)value >> 24) & 0xFF;
                return string.Format("{0}.{1}.{2}.{3}", v3, v2, v1, v0);
            }
            else
            {
                return value.ToString();
            }
        }

        private void SetValueUnchecked(long newVal)
        {
            base.Value = newVal;
            val_ = base.Value;
            UpdateValue(val_);
        }

        // handle increment when slider was moved by mouse
        private void SetValue(long newValue)
        {
            if (newValue == max_ || newValue == min_)
            {
                SetValueUnchecked(newValue);
            }
            else
            {
                long newVal = Math.Min(max_, Math.Max(min_, newValue));

                if (((newVal - min_) % inc_) != 0)
                {
                    var bigNewVal = new System.Numerics.BigInteger(newVal);
                    var bigMin = new System.Numerics.BigInteger(min_);
                    var offset = bigNewVal - bigMin;
                    offset = offset / inc_ * inc_;

                    var fixedVal = (long)(bigMin + offset);

                    if (fixedVal == val_)
                    {
                        if (newVal > val_)
                            newVal = val_ + inc_;
                        if (newVal < val_)
                            newVal = val_ - inc_;
                    }
                    else
                    {
                        newVal = (long)fixedVal;
                    }
                }

                SetValueUnchecked(newVal);
            }
        }

        private void UpdateValue(long newValue)
        {
            BlockSignals = true;
            if (slider_ != null)
            {
                lastTrackBarValue = ValueToSliderPosition(newValue);
                slider_.Value = lastTrackBarValue;
            }
            if (spin_ != null)
            { 
                spin_.Value = newValue;
                spin_.DisplayText = ValueToString(newValue, representation_);
            }
            if(edit_ != null)
            {
                edit_.Text = ValueToString(newValue, representation_);
            }
            BlockSignals = false;
        }

        private void Edit__TextChanged(object sender, EventArgs e)
        {
            throw new NotImplementedException();
        }

        private void CustomNumericUpDown1_TextInput(object sender, EventArgs e)
        {
            BlockSignals = true;
            try
            {
                SetValue(int.Parse(spin_.DisplayText));

            }
            catch(Exception ex)
            {
                SetValue(spin_.Value);
                System.Console.WriteLine(ex.Message);
            }
            BlockSignals = false;
        }

        private void NumericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            if(BlockSignals)
            {
                return;
            }

            SetValueUnchecked(spin_.Value);
        }

       
        private void TrackBar1_ValueChanged(object sender, EventArgs e)
        {
            if (BlockSignals)
            {
                return;
            }

            var newValue = slider_.Value;
            if (Math.Abs(newValue - lastTrackBarValue) == 1)
            {
                if(newValue > lastTrackBarValue)
                    spin_.Value+= inc_;
                else
                    spin_.Value -= inc_;

                SetValueUnchecked(spin_.Value);
            }
            else
            {
                SetValue(SliderPosToValue(slider_.Value));
            }
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();

            var property = Property as ic4.PropInteger;
            bool isReadOnly = property.IsReadonly;

            this.Size = new System.Drawing.Size(WinformsUtil.Scale(240), Appearance.ControlHeight);

            switch (representation_)
            {
                case ic4.IntRepresentation.Boolean:
                    throw new Exception("not implemented");
                case ic4.IntRepresentation.HexNumber:
                case ic4.IntRepresentation.PureNumber:
                    spin_ = new CustomNumericUpDown()
                    {
                        Parent = this,
                        Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
                        Location = new Point(1, 0),
                        Width = WinformsUtil.Scale(239),
                        TabIndex = 0
                    };
                    break;
                case ic4.IntRepresentation.Linear:
                case ic4.IntRepresentation.Logarithmic:
                    slider_ = new System.Windows.Forms.TrackBar()
                    {
                        Parent = this,
                        Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
                        Location = new Point(-7, 1),
                        Size = new Size((int)(150 * WinformsUtil.Scaling), 45),
                        TabIndex = 0,
                        TickStyle = System.Windows.Forms.TickStyle.None
                    };
                    spin_ = new CustomNumericUpDown()
                    {
                        Anchor = AnchorStyles.Top | AnchorStyles.Right,
                        Location = new Point((int)(150 * WinformsUtil.Scaling), 0),
                        Width = WinformsUtil.Scale(90),
                        TabIndex = 1,
                        Value = 0,
                        Parent = this
                    };
                    break;
                case ic4.IntRepresentation.MacAddress:
                case ic4.IntRepresentation.IP4Address:
                    edit_ = new System.Windows.Forms.TextBox()
                    {
                        Dock = DockStyle.Fill,
                        Font = new System.Drawing.Font("Microsoft Sans Serif", Appearance.ControlFontSize, FontStyle.Regular, GraphicsUnit.Point, 0),
                        Parent = this
                    };
                    break;
            }

            if (spin_ != null)
            {
                spin_.ValueChanged += NumericUpDown1_ValueChanged;
                spin_.TextInput += CustomNumericUpDown1_TextInput;
                this.Controls.Add(spin_);
            }

            if (slider_ != null)
            {
                slider_.ValueChanged += TrackBar1_ValueChanged;
                this.Controls.Add(slider_);
            }

            if (edit_ != null)
            {
                edit_.TextChanged += Edit__TextChanged;
                this.Controls.Add(edit_);
            }

            this.ResumeLayout(false);
            this.PerformLayout();
        }
    }
}
