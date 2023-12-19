using ic4;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ic4.Examples
{
    [ToolboxItem(false)]
    class PropControlBase : Control
    {
        internal static PropertyViewFlags Flags { get; set; } = PropertyViewFlags.Default;

        internal ic4.Property Property { get; set; }

        public PropControlBase(ic4.Property property)
        {
            Property = property;
            Property.Notification += Property__Notification;
        }

        internal virtual void UpdateAll() { }

        private void Property__Notification(object sender, EventArgs e)
        {
            if (this.Created)
            {
                this.BeginInvoke(new Action(() =>
                {
                    UpdateAll();
                }));
            }
        }

    }

    [ToolboxItem(false)]
    class PropControl<T> : PropControlBase
    {
        private static int autoTabIndex_ = 0;
        protected static int AutoTabIndex
        {
            get
            {
                autoTabIndex_++;
                return autoTabIndex_;
            }
        }
        private ic4.Grabber Grabber { get; set; }

        protected bool BlockSignals { get; set; } = false;


        public PropControl(ic4.Grabber grabber, ic4.Property property) : base(property)
        {
            Grabber = grabber;
        }

        private void SetValueInternal(string value)
        {
            try
            {
                switch (Property.Type)
                {
                    case PropertyType.Integer:
                        var propInt = Property as ic4.PropInteger;
                        propInt.Value = long.Parse(value);
                        break;
                    case PropertyType.Float:
                        var propFloat = Property as ic4.PropFloat;
                        propFloat.Value = double.Parse(value);
                        break;
                    case PropertyType.Boolean:
                        var propBool = Property as ic4.PropBoolean;
                        propBool.Value = bool.Parse(value);
                        break;
                    case PropertyType.Enumeration:
                        var propEnum = Property as ic4.PropEnumeration;
                        propEnum.Value = value;
                        break;
                    case PropertyType.String:
                        var propString = Property as ic4.PropString;
                        propString.Value = value;
                        break;
                    case PropertyType.Command:
                        var propCmd = Property as ic4.PropCommand;
                        propCmd.Execute();
                        break;
                }
            }
            catch (Exception ex)
            {
                System.Console.WriteLine(ex.Message);
            }
        }

        protected bool IsLocked
        {
            get
            {
                if (PropControlBase.Flags == PropertyViewFlags.AllowStreamRestart)
                {
                    if (PropNeedsStreamStartStop)
                    {
                        return false;
                    }
                }
                return Property.IsLocked;
            }
        }

        protected bool IsReadonly
        {
            get
            {
                if (PropControlBase.Flags == PropertyViewFlags.AllowStreamRestart)
                {
                    if (PropNeedsStreamStartStop)
                    {
                        return false;
                    }
                }
                return Property.IsReadonly;
            }
        }


        bool PropNeedsStreamStartStop
        {
            get
            {
                if (Property.Name == "Width")//ic4.PropId.Width.ToString())
                    return true;
                if (Property.Name == "Height")//ic4.PropId.Height.ToString())
                    return true;
                if (Property.Name == "PixelFormat")//ic4.PropId.PixelFormat.ToString())
                    return true;
                if (Property.Name == "ChunkModeActive")//ic4.PropId.ChunkModeActive.ToString())
                    return true;
                if (Property.Name == "ChunkEnable")//ic4.PropId.ChunkEnable.ToString())
                    return true;
                if (Property.Name == "BinningHorizontal")//ic4.PropId.BinningHorizontal.ToString())
                    return true;
                if (Property.Name == "BinningVertical")//ic4.PropId.BinningVertical.ToString())
                    return true;
                if (Property.Name == "DecimationHorizontal")//ic4.PropId.DecimationHorizontal.ToString())
                    return true;
                if (Property.Name == "DecimationVertical")//ic4.PropId.DecimationVertical.ToString())
                    return true;
                if (Property.Name == "UserSetLoad")//ic4.PropId.UserSetLoad.ToString())
                    return true;

                return false;
            }
        }


        protected T Value
        {
            set
            {
                if (PropNeedsStreamStartStop)
                {
                    bool need_prepare = Grabber.IsStreaming;
                    bool need_start = Grabber.IsAcquisitionActive;
                    var sink = Grabber.Sink;
                    var display = Grabber.Display;

                    Grabber.StreamStop();

                    SetValueInternal(value.ToString());

                    if (need_prepare)
                    {
                        Grabber.StreamSetup(sink, display, ic4.StreamSetupOption.DeferAcquisitionStart);
                    }
                    if (need_start)
                    {
                        Grabber.AcquisitionStart();
                    }
                }
                else
                {
                    SetValueInternal(value.ToString());
                }
            }
            get
            {
                switch (Property.Type)
                {
                    case PropertyType.Integer:
                        var propInt = Property as ic4.PropInteger;
                        return (T)(object)propInt.Value;
                    case PropertyType.Float:
                        var propFloat = Property as ic4.PropFloat;
                        return (T)(object)propFloat.Value;
                    case PropertyType.Boolean:
                        var propBool = Property as ic4.PropBoolean;
                        return (T)(object)propBool.Value;
                    case PropertyType.Enumeration:
                        var propEnum = Property as ic4.PropEnumeration;
                        return (T)(object)propEnum.Value;
                    case PropertyType.String:
                        var propString = Property as ic4.PropString;
                        return (T)(object)propString.Value;
                }
                return default(T);
            }
        }
    }
}
