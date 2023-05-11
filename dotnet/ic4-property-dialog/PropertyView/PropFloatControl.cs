using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.NetworkInformation;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace ic4.Examples
{
    [ToolboxItem(false)]
    internal class PropFloatControl : PropControl<double>
    {
        private System.Windows.Forms.TrackBar slider_;
        private CustomNumericUpDown spin_;

        private const int SLIDER_MIN = 0;
        private const int SLIDER_MAX = 100;
        private const int SLIDER_TICKS = SLIDER_MAX - SLIDER_MIN;

        private double min_ = 0.0;
        private double max_ = 100.0;
        private string unit_ = string.Empty;
        private ic4.FloatRepresentation representation_ = ic4.FloatRepresentation.Liniear;
        public ic4.DisplayNotation notation_ = ic4.DisplayNotation.Fixed;
        private long precision_ = 0;

        public PropFloatControl(ic4.Grabber grabber, ic4.PropFloat property) : base(grabber, property)
        {
            InitializeComponent();
            UpdateAll();
        }

        internal override void UpdateAll()
        {
            var prop = Property as ic4.PropFloat;
            bool isLocked = prop.IsLocked;
            bool isReadonly = prop.IsReadonly;
            bool hasIncrement = prop.IncrementMode != ic4.PropertyIncrementMode.None;
            float increment = hasIncrement ? (float)prop.Increment : (float)1;
        
            min_ = prop.Minimum;
            max_ = prop.Maximum;
            unit_ = prop.Unit;

            BlockSignals = true;

            if(slider_ != null)
            {
                slider_.Minimum = SLIDER_MIN;
                slider_.Maximum = SLIDER_MAX;
                slider_.Value = GetSliderPosition(prop.Value);
                slider_.Enabled = !isLocked;
            }

            if (spin_ != null)
            {
                spin_.Minimum = SLIDER_MIN;
                spin_.Maximum = SLIDER_MAX;

                //if(hasIncrement)
                //{
                //    spin_.StepSize = increment;
                //}

                spin_.Value = GetSliderPosition(prop.Value);
                spin_.ReadOnly = isLocked || isReadonly;
                spin_.ShowButtons = !isReadonly;
                spin_.DisplayText = GetTextFromValue(prop.Value) + " " + prop.Unit;
            }

            BlockSignals = false;
        }

        private void CustomNumericUpDown1_TextInput(object sender, EventArgs e)
        {
            BlockSignals = true;
            double value = 0.0;
            if (ParseValue(spin_.DisplayText, ref value))
            {
                SetValueUnchecked(value);
            }
            BlockSignals = false;
        }

        private void CustomNumericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            if (BlockSignals)
                return;

            SetValueUnchecked(GetValue((int)spin_.Value));
        }

        private void TrackBar1_Scroll(object sender, EventArgs e)
        {
            if (BlockSignals)
                return;

            SetValueUnchecked(GetValue(slider_.Value));
        }

        private void SetValueUnchecked(double newVal)
        {
            base.Value = newVal;
            UpdateValue(base.Value);
        }

        private void UpdateValue(double newValue)
        {
            BlockSignals = true;
            int pos = GetSliderPosition(newValue);
            if (slider_ != null)
            {
                slider_.Value = pos;
            }
            if (spin_ != null)
            {
                spin_.DisplayText = GetTextFromValue(newValue) + " " + unit_;
                spin_.Value = pos;
                spin_.Invalidate();
            }
            BlockSignals = false;
        }

        private int GetSliderPosition(double value)
        {
            try
            {
                Func<double, double> f;
                if (representation_ == ic4.FloatRepresentation.Logarithmic)
                {
                    f = new Func<double, double>((val) => { return Math.Log(val); });
                }
                else
                {
                    f = new Func<double, double>((val) => { return val; });
                }


                double rangelen = f(max_) - f(min_);
                double p = (double)(SLIDER_TICKS) / rangelen * (f(value) - f(min_));

                p = (int)(p + 0.5);
                if (p == -2147483648)
                {
                    p = SLIDER_MAX;
                }

                return Math.Min(SLIDER_MAX, Math.Max(SLIDER_MIN, (int)(p + 0.5)));
            }
            catch
            {
                return 0;
            }
        }

        private double GetValue(int pos)
        {
            try
            {
                Func<double, double> f;
                Func<double, double> f_inv;
                if (representation_ == ic4.FloatRepresentation.Logarithmic)
                {
                    f = new Func<double, double>((val) => { return Math.Log(val); });
                    f_inv = new Func<double, double>((val) => { return Math.Exp(val); });
                }
                else
                {
                    f = new Func<double, double>((val) => { return val; });
                    f_inv = new Func<double, double>((val) => { return val; });
                }

                double rangelen = f(max_) - f(min_);
                double val0 = f_inv(f(min_) + rangelen / SLIDER_TICKS * pos);

                if (val0 > max_) val0 = max_;
                if (val0 < min_) val0 = min_;

                return val0;
            }
            catch
            {
                return min_;
            }
        }

        protected virtual string GetTextFromValue(double value)
        {
            if(notation_ ==  ic4.DisplayNotation.Scientific)
            {
                return value.ToString("0.00000E0");
            }

            if(value >= Math.Pow(10.0, precision_))
            {
                return ((int)value).ToString();
            }
            else
            {
                return Math.Round(value, Math.Max(0, (int)(precision_ - ((int)value).ToString().Length))).ToString();
            }
           
        }

        private bool ParseValue(string str, ref double result)
        {
            string dimText = string.IsNullOrEmpty(unit_) ? string.Empty : this.unit_;

            string trimmed = str.Trim(dimText.ToArray());

            int p = trimmed.IndexOf('/');
            double value = 0;
            bool valueParsed = false;

            if (p >= 0)
            {
                string leftStr = trimmed.Substring(0, p);
                string rightStr = trimmed.Substring(p + 1);

                try
                {
                    int left = System.Convert.ToInt32(leftStr);
                    int right = System.Convert.ToInt32(rightStr);
                    if (right != 0)
                    {
                        value = left;
                        value /= right;
                        valueParsed = true;
                    }
                }
                catch
                {

                }
            }
            else
            {
                try
                {
                    value = System.Convert.ToDouble(trimmed);
                    valueParsed = true;
                }
                catch
                {

                }
            }

            if (valueParsed)
            {
                try
                {
                    value = Math.Min(value, max_);
                    value = Math.Max(value, min_);
                    result = value;

                    return true;
                }
                catch
                {
                }
            }

            return false;
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();

            var prop = Property as ic4.PropFloat;

            bool readOnly = prop.IsReadonly;

            precision_ = prop.DisplayPrecision;
            notation_ = prop.DisplayNotation;
            representation_ = prop.Representation;

            int height = (int)(21.0f * WinformsUtil.Scaling);
            if (height % 2 == 0)
            {
                height--;
            }

            this.Size = new System.Drawing.Size((int)(240 * WinformsUtil.Scaling), height);

            switch (representation_)
            {
                case ic4.FloatRepresentation.PureNumber:
                    spin_ = new CustomNumericUpDown()
                    {
                        Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
                        Location = new Point(1, 0),
                        Size = new Size((int)(239 * WinformsUtil.Scaling), height),
                        TabIndex = 0,
                        Parent = this
                    };
                    break;
                case ic4.FloatRepresentation.Liniear:
                case ic4.FloatRepresentation.Logarithmic:
                    slider_ = new System.Windows.Forms.TrackBar()
                    {
                        Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
                        Location = new Point(-7, 1),
                        Size = new Size((int)(150 * WinformsUtil.Scaling), 45),
                        TabIndex = 0,
                        TickStyle = System.Windows.Forms.TickStyle.None,
                        Parent = this
                    };
                    spin_ = new CustomNumericUpDown()
                    {
                        Anchor = AnchorStyles.Top | AnchorStyles.Right,
                        Location = new Point((int)(150 * WinformsUtil.Scaling), 0),
                        Size = new Size((int)(90 * WinformsUtil.Scaling), height),
                        TabIndex = 1,
                        Value = 0,
                        Parent = this
                    };
                    break;
            }

            if (slider_ != null)
            {
                slider_.Scroll += TrackBar1_Scroll;
            }
            if (spin_ != null)
            {

                spin_.ValueChanged += CustomNumericUpDown1_ValueChanged; ;
                spin_.TextInput += CustomNumericUpDown1_TextInput;
            }

            ResumeLayout(false);
            PerformLayout();

        }
    }
}
