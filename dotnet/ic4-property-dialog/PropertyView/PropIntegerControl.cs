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
using System.IO;

namespace ic4.Examples
{
    class PropIntegerControl : PropControl<long>
    {
        private const int SLIDER_MIN = 0;
        private const int SLIDER_MAX = 100000000;
        private const int SLIDER_TICKS = SLIDER_MAX - SLIDER_MIN;

        private long min_ = 0;
        private long max_ = 100;
        private long inc_ = 1;
        private long val_ = 0;
        private int lastSliderIndex = 0;
        private ic4.IntRepresentation representation_ = ic4.IntRepresentation.Linear;

        private NoFocusTrackBar slider_;
        private CustomNumericUpDown spin_;
        private System.Windows.Forms.TextBox edit_;

        public PropIntegerControl(ic4.Grabber grabber, ic4.PropInteger property) : base(grabber, property)
        {
            representation_ = property.Representation;
            InitializeComponent();
            UpdateAll();
        }

        private long SliderPosToValue(int pos)
        {
            var position = new decimal(Math.Max(SLIDER_MIN, Math.Min(SLIDER_MAX, pos)));
            var decMin = new decimal(min_);
            var decMax = new decimal(max_);
            var frac = (decMax - decMin) / SLIDER_TICKS;
            var val0 = decMin + (frac * position);

            if (val0 > decMax) val0 = decMax;
            if (val0 < decMin) val0 = decMin;

            return (long)val0;
        }

        private int ValueToSliderPosition(long value)
        {
            var decVal = new decimal(value);
            var decMin = new decimal(min_);
            var decMax = new decimal(max_);
            var range = new decimal(SLIDER_TICKS);
            var p = (range / (decMax - decMin) * (decVal - decMin));

            return (int)p;
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

            BlockSignals = true;

            if (slider_ != null)
            {
                slider_.Minimum = SLIDER_MIN-1;
                slider_.Maximum = SLIDER_MAX+1;
                slider_.SmallChange = 1;
                slider_.LargeChange = SLIDER_TICKS / 100;

                lastSliderIndex = ValueToSliderPosition(val_);
                slider_.Value = lastSliderIndex;
                slider_.Enabled = !isLocked;
            }

            if (spin_ != null)
            {
                if(propInt.IncrementMode == PropertyIncrementMode.ValueSet)
                {
                    spin_.Minimum = 0;
                    spin_.Maximum = propInt.ValidValueSet.Count() - 1;
                }
                else
                { 
                    spin_.Minimum = min_;
                    spin_.Maximum = max_;
                }

                spin_.StepSize = inc_;
                spin_.Value = ValueToSpinboxIndex(val_);
                spin_.DisplayText = ValueToString(val_, representation_);
                spin_.ReadOnly = isLocked || isReadonly;
                spin_.ShowButtons = !isReadonly;

            }

            if(edit_ != null)
            {
                edit_.Text = ValueToString(val_, representation_);
                edit_.ReadOnly = isLocked || isReadonly;
            }

            BlockSignals = false;

        }

        private long ValueToSpinboxIndex(long value)
        {
            var propInt = Property as ic4.PropInteger;
            if (propInt.IncrementMode == PropertyIncrementMode.ValueSet)
            {
                for (int i = 0; i < propInt.ValidValueSet.Count(); ++i)
                {
                    if (propInt.ValidValueSet.ElementAt(i) == value)
                    {
                        return i;
                    }
                }
                return 0;
            }
            else
            {
                return value;
            }
        }

        private long SpinboxIndexToValue(long index)
        {
            var propInt = Property as ic4.PropInteger;
            if (propInt.IncrementMode == PropertyIncrementMode.ValueSet)
            {
                return propInt.ValidValueSet.ElementAt((int)index);
            }
            else
            {
                return Math.Max(min_, Math.Min( max_, index));
            }
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
                lastSliderIndex = ValueToSliderPosition(newValue);
                slider_.Value = lastSliderIndex;
            }
            if (spin_ != null)
            { 
                spin_.Value = ValueToSpinboxIndex(newValue);
                spin_.DisplayText = ValueToString(newValue, representation_);
            }
            if(edit_ != null)
            {
                edit_.Text = ValueToString(newValue, representation_);
            }
            BlockSignals = false;
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
                SetValue(SpinboxIndexToValue(spin_.Value));
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

            SetValueUnchecked(SpinboxIndexToValue(spin_.Value));
        }

       
        private void TrackBar1_ValueChanged(object sender, EventArgs e)
        {
            if (BlockSignals)
            {
                return;
            }

            var newIndex = slider_.Value;

            if (Math.Abs(newIndex - lastSliderIndex) == 1)
            {
                var bigVal = new System.Numerics.BigInteger(spin_.Value);

                if (newIndex > lastSliderIndex)
                {
                    bigVal += inc_;
                    var bigMax = new System.Numerics.BigInteger(max_);

                    if (bigVal > bigMax)
                        spin_.Value = max_;
                    else 
                        spin_.Value = (long)bigVal;
                }
                else
                {
                    bigVal -= inc_;
                    var bigMin = new System.Numerics.BigInteger(min_);

                    if (bigVal < bigMin)
                        spin_.Value = min_;
                    else
                        spin_.Value = (long)bigVal;
                }

                SetValueUnchecked(SpinboxIndexToValue(spin_.Value));
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

            int spinBoxWidth = 90;
            if(property.Maximum < int.MinValue || property.Maximum > int.MaxValue)
            {
                spinBoxWidth = 142;
            }

            this.Size = new System.Drawing.Size(WinformsUtil.Scale(240), Appearance.ControlHeight);
            this.TabStop = false;

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
                        TabIndex = AutoTabIndex,
                        TabStop= true
                    };
                    break;
                case ic4.IntRepresentation.Linear:
                case ic4.IntRepresentation.Logarithmic:
                    slider_ = new NoFocusTrackBar()
                    {
                        Parent = this,
                        Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
                        Location = new Point(-7, 1),
                        Size = new Size(WinformsUtil.Scale(240 - spinBoxWidth), 45),
                        TabIndex = AutoTabIndex,
                        TabStop = true,
                        TickStyle = System.Windows.Forms.TickStyle.None
                    };
                    spin_ = new CustomNumericUpDown()
                    {
                        Anchor = AnchorStyles.Top | AnchorStyles.Right,
                        Location = new Point(WinformsUtil.Scale(240 - spinBoxWidth), 0),
                        Width = WinformsUtil.Scale(spinBoxWidth),
                        TabIndex = AutoTabIndex,
                        Value = 0,
                        TabStop = true,
                        Parent = this
                    };
                    break;
                case ic4.IntRepresentation.MacAddress:
                case ic4.IntRepresentation.IP4Address:
                    edit_ = new System.Windows.Forms.TextBox()
                    {
                        Dock = DockStyle.Fill,
                        Font = new System.Drawing.Font("Microsoft Sans Serif", Appearance.ControlFontSize, FontStyle.Regular, GraphicsUnit.Point, 0),
                        Parent = this,
                        TabStop = true,
                        TabIndex = AutoTabIndex
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
                edit_.KeyDown += Edit__KeyDown;
                edit_.LostFocus += Edit__LostFocus;
                this.Controls.Add(edit_);
            }

            this.ResumeLayout(false);
            this.PerformLayout();
        }


        private string editText_ = string.Empty;

        private void Edit__LostFocus(object sender, EventArgs e)
        {
            if (editText_ != edit_.Text)
            {
                editText_ = edit_.Text;
                Edit__TextChanged();
            }

        }

        private void Edit__KeyDown(object sender, KeyEventArgs e)
        {
           
            if (e.KeyCode == Keys.Enter)
            {
                if (editText_ != edit_.Text)
                {
                    editText_ = edit_.Text;
                    Edit__TextChanged();
                }
            }
        }

        private void Edit__TextChanged()
        {
            BlockSignals = true;
            try
            {
                SetValue(long.Parse(editText_));
            }
            catch (Exception ex)
            {
                SetValue(SpinboxIndexToValue(spin_.Value));
                System.Console.WriteLine(ex.Message);
            }
            BlockSignals = false;
        }
    }
}
