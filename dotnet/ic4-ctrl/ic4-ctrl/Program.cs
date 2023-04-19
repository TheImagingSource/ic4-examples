using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Linq.Expressions;
using System.Net.NetworkInformation;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Xml.Linq;
using CommandLine;
using ic4;
// prop --device-id=test123 --forceinterface prop1 prop2









namespace ic4_ctrl
{
    internal class Program
    {
        private static void Print(int offset, string text)
        {
            for (int i = 0; i < offset; ++i)
            {
                System.Console.Write("\t");
            }
            System.Console.Write(text);
        }

        private static void Print(string text)
        {
            System.Console.Write(text);
        }

        private static ic4.DeviceInfo FindDevice(string id)
        {
            var list = DeviceEnum.Devices;
            if(!list.Any())
            {
                throw new Exception("No devices are available");
            }

            var device = list.FirstOrDefault(dev => dev.Serial == id || dev.UniqueName == id || dev.ModelName == id);
            if(device != null)
            {
                return device;
            }

            try
            {
                int index = int.Parse(id);
                if (index >= 0 && index < list.Count())
                {
                    return list.ElementAt(index);
                }
            }
            catch
            {
                return null;
            }

            return null;
        }

        private static ic4.Interface FindInterface(string id)
        {
            var list = DeviceEnum.Interfaces;
            if (!list.Any())
            {
                throw new Exception("No Interfaces are available.");
            }

            var itf = list.FirstOrDefault(i => i.Name == id || i.TransportLayerName == id);
            if (itf != null)
            {
                return itf;
            }


            try
            {
                int index = int.Parse(id);
                if (index > 0 && index < list.Count())
                {
                    return list.ElementAt(index);
                }
            }
            catch
            {
            }

            return null;
        }

        private static void ListDevices()
        {
            var list = DeviceEnum.Devices;

            System.Console.WriteLine("Device list:");
            System.Console.WriteLine("\tIndex\tModelName\tSerial\t\tInterfaceName");
            int index = 0;
            foreach (var device in list)
            {
                Print(1, string.Format("{0}\t{1}\t{2}\t{3}", index, device.ModelName, device.Serial, device.Interface.TransportLayerName));
                index++;
            }
            if (!list.Any())
            {
                System.Console.WriteLine("\tNo devices found");
            }
        }

        private static void ListInterfaces()
        {
            var list = DeviceEnum.Interfaces;

            System.Console.WriteLine("Interface list:");

            foreach (var e in list)
            {
                Print(1, string.Format("{0}\n",e.Name));
                Print(2, string.Format("TransportLayerName: {0}\n", e.TransportLayerName));
            }

            if (!list.Any())
            {
                System.Console.WriteLine("No Interfaces found");
            }
        }

        private static void PrintDevice(string id)
        {
            var dev = FindDevice(id);
            if (dev == null)
            {
                throw new Exception(string.Format("Failed to find device for id '{0}'", id));
            }

            Print(string.Format("ModelName: '{0}'\n", dev.ModelName));
            Print(string.Format("Serial: '{0}'\n", dev.Serial));
            Print(string.Format("UniqueName: '{0}'\n", dev.UniqueName));
            Print(string.Format("DeviceVersion: '{0}'\n", dev.Version));
            Print(string.Format("InterfaceName: '{0}'\n", dev.Interface.TransportLayerName));

        }

        private static void PrintInterface(string interface_id)
        {
            var itf = FindInterface(interface_id);
            if (itf == null)
            {
                throw new Exception("Failed to find device for id '{id}'");
            }

            Print( string.Format(" Name: '{0}'", itf.Name)); ;
            Print( string.Format(" TransportLayerName: '{0}'\n", itf.TransportLayerName));
            Print( string.Format(" TransportLayerType: '{0}'\n", itf.TransportLayerType ));
            Print( string.Format(" TransportVersion: '{0}'\n", itf.TransportLayerVersion ));

            Print("Interface Properties:\n");
            var map = itf.PropertyMap;
            foreach(var property in map.All)
            {
                PrintProperty(1, property);
            }
        }

        private static string FetchPropertyMethodValue<T>(ic4.Property prop, Expression<Func<T>> address)
        {
            var propertyInfo = ((MemberExpression)address.Body).Member as PropertyInfo;
            if (propertyInfo == null)
            {
                throw new ArgumentException("The lambda expression 'property' should point to a valid Property");
            }

            object result;

            try
            {
                result = propertyInfo.GetValue(prop);
            }
            catch(ic4.IC4Exception ex)
            {
                if( ex.ErrorCode == ic4.Error.GenICamNotImplemented)
                {
                    return "n/a";
                }
                return "err";
            }
            catch 
            {
                return "err";
            }

            return result.ToString();
        }
        private static string FetchPropertyMethodValue<T>(ic4.Property prop, Expression<Func<T>> address, ic4.IntRepresentation int_rep)
        {
            var propertyInfo = ((MemberExpression)address.Body).Member as PropertyInfo;
            if (propertyInfo == null)
            {
                throw new ArgumentException("The lambda expression 'property' should point to a valid Property");
            }

            long v;

            try
            {
                v = (long)propertyInfo.GetValue(prop);
            }
            catch (ic4.IC4Exception ex)
            {
                if (ex.ErrorCode == ic4.Error.GenICamNotImplemented)
                {
                    return "n/a";
                }
                return "err";
            }
            catch
            {
                return "err";
            }

            switch (int_rep)
            {
                case ic4.IntRepresentation.Boolean: return string.Format("{0}", v != 0 ? 1 : 0);
                case ic4.IntRepresentation.HexNumber: return string.Format("0x{0}", v);
                case ic4.IntRepresentation.IP4Address:
                    {
                        ulong v0 = ((ulong)v >> 0) & 0xFF;
                        ulong v1 = ((ulong)v >> 8) & 0xFF;
                        ulong v2 = ((ulong)v >> 16) & 0xFF;
                        ulong v3 = ((ulong)v >> 24) & 0xFF;
                        return string.Format("{0}.{1}.{2}.{3}", v3, v2, v1, v0);
                    }
                case ic4.IntRepresentation.MacAddress:
                    {
                        ulong v0 = ((ulong)v >> 0) & 0xFF;
                        ulong v1 = ((ulong)v >> 8) & 0xFF;
                        ulong v2 = ((ulong)v >> 16) & 0xFF;
                        ulong v3 = ((ulong)v >> 24) & 0xFF;
                        ulong v4 = ((ulong)v >> 32) & 0xFF;
                        ulong v5 = ((ulong)v >> 40) & 0xFF;
                        return string.Format("{0}:{1}:{2}:{3}:{4}:{5}", v5, v4, v3, v2, v1, v0);
                    }
                case ic4.IntRepresentation.Linear:
                case ic4.IntRepresentation.Logarithmic:
                case ic4.IntRepresentation.PureNumber:
                default:
                    return string.Format("{0}", v);
            }
        }

        private static void PrintProperty(int offset, Property property)
        {
            var propType = property.Type;
            Print(offset + 0, string.Format("{0} - Type: {1}, DisplayName: {2}\n", property.Name, propType.ToString(), property.DisplayName));
            Print(offset + 1, string.Format("Description: {0}\n", property.Description));
            Print(offset + 1, string.Format("Tooltip: {0}\n", property.Tooltip ));
            Print(offset + 3, "\n");
            Print(offset + 1, string.Format("Visibility: {0}, Available: {1}, Locked: {2}, ReadOnly: {3}\n", property.Visibility.ToString(), property.IsAvailable, property.IsLocked, property.IsReadonly));

            if (property.IsSelector)
            {
                Print(offset + 1, "Selected properties:\n");
                foreach( var selected in property.SelectedProperties)
                {
                    Print(offset + 2, "{selected.Name}\n");
                }
            }

            switch (propType)
            {
                case PropertyType.Integer:
                    {
                        PropInteger prop = property as ic4.PropInteger;
                        var incMode = prop.IncrementMode;
                        var rep = prop.Representation;

                        Print(offset + 1, string.Format("Representation: '{0}', Unit: '{1}', IncrementMode: '{2}'\n", rep, prop.Unit, incMode));

                        if (prop.IsAvailable)
                        {
                            if (!prop.IsReadonly)
                            {
                                var min = prop.Minimum;
                                var max = prop.Maximum;

                                Print(offset + 1, string.Format("Min: {0}, Max: {1}\n",
                                    FetchPropertyMethodValue<long>(prop, () => prop.Minimum, rep),
                                    FetchPropertyMethodValue<long>(prop, () => prop.Maximum, rep))
                                );
                            }
                            if (incMode == ic4.PropertyIncrementMode.Increment)
                            {
                                if (!prop.IsReadonly)
                                {
                                    Print(offset + 1, string.Format("Inc: {0}\n",
                                        FetchPropertyMethodValue<long>(prop, () => prop.Increment, rep))
                                    );
                                }
                            }
                            else if (incMode == ic4.PropertyIncrementMode.ValueSet)
                            {
                                try
                                {
                                    var vvset = prop.ValidValueSet;
                                    Print(offset + 1, "ValidValueSet:\n");
                                    foreach(var val in  vvset )
                                    {
                                        Print(offset + 2, string.Format("{0}\n", val));
                                    }
                                    System.Console.Write("\n");
                                }
                                catch
                                {
                                    Print(offset + 1, "Failed to fetch ValidValueSet\n");
                                }
                               
                            }
                            Print(offset + 1, string.Format("Value: {0}\n",
                                FetchPropertyMethodValue<long>(prop, () => prop.Value, rep))
                            );
                        }
                    }
                    break;
                case PropertyType.Float:
                    {
                        var prop = property as PropFloat;
                        var incMode = prop.IncrementMode;

                        Print(offset + 1, string.Format("Representation: '{0}', Unit: '{1}', IncrementMode: '{2}', DisplayNotation: {3}, DisplayPrecision: {4}\n",
                            prop.Representation, prop.Unit, incMode, prop.DisplayNotation, prop.DisplayPrecision));

                        if (prop.IsAvailable)
                        {
                            if (!prop.IsReadonly)
                            {
                                Print(offset + 1, string.Format("Min: {0}, Max: {1}\n",
                                    FetchPropertyMethodValue<double>(prop, () => prop.Minimum),
                                    FetchPropertyMethodValue<double>(prop, () => prop.Maximum))
                                );
                            }

                            if (incMode == PropertyIncrementMode.Increment)
                            {
                                Print(offset + 1, string.Format("Inc: {0}\n",
                                    FetchPropertyMethodValue<double>(prop, () => prop.Increment))
                                );
                            }
                            else if (incMode == PropertyIncrementMode.ValueSet)
                            {
                                try
                                {
                                    var vvset = prop.ValidValueSet;
                                    Print(offset + 1, "ValidValueSet:\n");
                                    foreach(var val in vvset )
                                    {
                                        Print(offset + 2, string.Format("{}\n", val));
                                    }
                                    Print("\n");
                                }
                                catch
                                {
                                    Print(offset + 1, "Failed to fetch ValidValueSet\n");
                                }
                            }

                            Print(offset + 1, string.Format("Value: {0}\n",
                                FetchPropertyMethodValue<double>(prop, () => prop.Value))
                            );
                        }
                    }
                    break;
                case PropertyType.Enumeration:
                    {
                        var prop = property as PropEnumeration;
                        Print(offset + 1, "EnumEntries:\n");
                        foreach (var entry in  prop.Entries)
                        {
                            var PropEnumEntry = entry as PropEnumEntry;

                            PrintProperty(offset + 2, PropEnumEntry);
                            Print(0, "\n");
                        }

                        if (prop.IsAvailable)
                        {
                            try
                            {
                                var selectedEntry = prop.SelectedEntry;
                                Print(offset + 1, string.Format("Value: {0}, SelectedEntry.Name: '{1}'\n",
                                    FetchPropertyMethodValue<long>(prop, () => prop.Value),
                                    selectedEntry.Name)
                                );
                            }
                            catch
                            {
                                Print(offset + 1, string.Format("Value: {0}, SelectedEntry.Name: '{1}'\n", "err", "err"));
                            }   
                        }
                    }
                    break;
                case PropertyType.Boolean:
                    {
                        var prop = property as PropBoolean;

                        if (prop.IsAvailable)
                        {
                            Print(offset + 1, string.Format("Value: {0}\n",
                                FetchPropertyMethodValue<bool>(prop, () => prop.Value))
                            );
                        }
                    }
                    break;
                case PropertyType.String:
                    {
                        var prop = property as PropString;

                        if (prop.IsAvailable)
                        {
                            Print(offset + 1, string.Format("Value: '{0}', MaxLength: {1}\n",
                                FetchPropertyMethodValue<string> (prop, () => prop.Value),
                                FetchPropertyMethodValue<ulong>(prop, () => prop.MaxLength))
                            );
                        }
                       
                    }
                    break;
                case PropertyType.Command:
                    {
                        Print(0, "\n");
                    }
                    break;
                case PropertyType.Category:
                    {
                        var prop = property as PropCategory;
                        Print(offset + 1, "Features:\n");
                        foreach (var feature in prop.Features )
                        {
                            Print(offset + 2, string.Format("{0}\n", feature.Name));
                        }
                    }
                    break;
                case PropertyType.Register:
                    {
                        var prop = property as PropRegister;

                        Print(offset + 1, string.Format("Size: {0}\n",
                            FetchPropertyMethodValue<ulong>(prop, () => prop.Size))
                        );
                        if (prop.IsAvailable)
                        {
                            try
                            {
                                var vec = prop.Value;
                                string str = string.Empty;
                                int maxEntriesToPrint = 16;
                                for (int i = 0; i < Math.Min(maxEntriesToPrint, vec.Length); ++i)
                                {
                                    str += string.Format("{0}", vec[i]);
                                    str += ", ";
                                }
                                if (vec.Length > maxEntriesToPrint)
                                {
                                    str += "...";
                                }
                                Print(offset + 1, string.Format("Value: [{0}], Value-Size: {1}\n", str, vec.Length));
                            }
                            catch(Exception e)
                            {
                                Print(offset + 1, string.Format("Value: '{0}'", e.Message ));
                            }
                           
                        }
                        Print(0, "\n");
                    }
                    break;
                case PropertyType.Port:
                    {
                        Print(0, "\n");
                    }
                    break;
                case PropertyType.EnumEntry:
                    {
                        var prop = property as PropEnumEntry;

                        if (prop.IsAvailable)
                        {
                            Print(offset + 1, string.Format("Value: {0}\n", FetchPropertyMethodValue<long>(prop, () => prop.Value)));
                        }
                        Print(0, "\n");
                    }
                    break;
            };
            Print(0, "\n");
        }

        static Tuple<string, string> SplitPropEntry(string propString)
        {
            var f = propString.Split('=');
            if (f.Length == 2)
            {
                return new Tuple<string, string>(f[0], f[1]);
            }
            else
            {
                return new Tuple<string, string>(propString, string.Empty);
            }
        }

        private static void SetPropertyFromAssignEntry(PropertyMap propertyMap, string propName, string propValue)
        {
            Print(string.Format("Setting property '{0}' to '{1}'\n", propName, propValue));

            try
            {
                propertyMap.SetValue(propName, propValue);
            }
            catch (Exception ex)
            {
                Print(string.Format("Failed to set value '{0}' on property '{1}'. Message: {2}\n", propValue, propName, ex.Message));
            }
        }

        private static void PrintOrSetPropertyMapEntries(PropertyMap map, string[] lst)
        {
            if (lst.Length == 0)
            {
                foreach (var property in map.All)
                {
                    PrintProperty(0, property);
                }
            }
            else
            {
                foreach (var entry in lst)
                {
                    var parseEntry = SplitPropEntry(entry);
                    if (parseEntry.Item2 != string.Empty)
                    {
                        SetPropertyFromAssignEntry(map, parseEntry.Item1, parseEntry.Item2);
                    }
                    else
                    {
                        if (map.TryGet(entry, out Property property))
                        {
                            PrintProperty(0, property);
                        }
                        else
                        {
                            Print(string.Format("Failed to find property for name: '{0}'\n", entry));
                        }
                    }
                }
            }
        }

        private static void ExecPropCmd(string id, bool forceInterface, string[] lst)
        {
            if(forceInterface)
            {
                var dev = FindInterface(id);
                if( dev== null)
                {
                    Print(string.Format("Failed to find interface for id '{0}'", id));
                    return;
                }
                var map = dev.PropertyMap;
                PrintOrSetPropertyMapEntries(map, lst);
            }
            else
            {
                var dev = FindDevice(id);
                if(dev == null)
                {
                    Print(string.Format("Failed to find device for id '{0}'", id));
                    return;
                }
                using (var g = new Grabber())
                {
                    g.DeviceOpen(dev);
                    PrintOrSetPropertyMapEntries(g.DevicePropertyMap, lst);
                }
            }
        }

        private static void SaveProperties(string id, bool forceInterface, string filename)
        {
            if (forceInterface)
            {
                var dev = FindInterface(id);
                if (dev == null)
                {
                    Print(string.Format("Failed to find interface for id '{0}'", id));
                    return;
                }
                dev.PropertyMap.Serialize(filename);
            }
            else
            {
                var dev = FindDevice(id);
                if (dev == null)
                {
                    Print(string.Format("Failed to find device for id '{0}'", id));
                    return;
                }
                using(var g = new Grabber())
                {
                    g.DeviceOpen(dev);
                    g.DevicePropertyMap.Serialize(filename);
                }
            }
        }

        static void SaveImage(string id, string filename, int count, int timeoutInMillisecs, string imageType)
        {
            var dev = FindDevice(id);
            if (dev == null)
            {
                Print(string.Format("Failed to find device for id '{0}'", id));
                return;
            }
            using (var g = new Grabber())
            {
                g.DeviceOpen(dev);

                var snapSink = new SnapSink();
                g.StreamSetup(snapSink, ic4.StreamSetupOption.AcquisitionStart);

                List<ImageBuffer> images = null;
                try
                {
                    images = snapSink.SnapSequence(count, TimeSpan.FromMilliseconds(timeoutInMillisecs));
                }
                catch (TimeoutException)
                {
                    Print("Timeout elapsed.");
                }


                g.AcquisitionStop();

                int idx = 0;
                foreach (var image in images)
                {
                    string actualFilename = filename;
                    if (filename.Contains("{}"))
                    {
                        actualFilename = string.Format(filename.Replace("{}", "{0}"), idx);
                    }

                    if (imageType == "bmp")
                    {
                        ic4.ImageBufferExtensions.SaveAsBitmap(image, actualFilename);
                    }
                    else if (imageType == "png")
                    {
                        ic4.ImageBufferExtensions.SaveAsPng(image, actualFilename);
                    }
                    else if (imageType == "tiff")
                    {
                        ic4.ImageBufferExtensions.SaveAsTiff(image, actualFilename);
                    }
                    else if (imageType == "jpeg")
                    {
                        ic4.ImageBufferExtensions.SaveAsJpeg(image, actualFilename);
                    }
                }
            }
        }

        private static void ShowLive(string id)
        {
            var dev = FindDevice(id);
            if(dev == null)
            {
                Print(string.Format("Failed to find device for id '{0}'", id));
                return;
            }

            using(var g = new Grabber())
            {
                g.DeviceOpen(dev);

                var display = new ic4.Display(IntPtr.Zero);
                g.StreamSetup(display, ic4.StreamSetupOption.AcquisitionStart);

                var cond = new ManualResetEventSlim();
                bool ended = false;

                display.WindowClosed += (s,r) =>
                {
                    ended = true;
                    cond.Set();
                };


                while (!ended)
                {
                    cond.Wait();
                }

                g.AcquisitionStop();
            }
        }

        private static void ShowPropPage(string id, bool isInterfaceId)
        {
            //if (isInterfaceId)
            //{
            //    var dev = FindInterface(id);
            //    if (dev == null)
            //    {
            //        Print(string.Format("Failed to find interface  for id '{0}'", id));
            //        return;
            //    }
            //
            //    ic4.showPropertyDialog(IntPtr.Zero, dev.Interface.PropertyMap);
            //}
            //else
            //{
            //    var dev = FindDevice(id);
            //    if (dev == null)
            //    {
            //        Print(string.Format("Failed to find device for id '{0}'", id));
            //        return;
            //    }
            //    using(var g = new Grabber())
            //    {
            //        g.DeviceOpen(dev);
            //        ic4.showPropertyDialog(IntPtr.Zero, g.DevicePropertyMap);
            //    }
            //}
        }

        private static void Display_WindowClosed(object sender, EventArgs e)
        {
            throw new NotImplementedException();
        }

        private interface ICommand
        { 
            void Execute();
        }


        [Verb("list", 
            HelpText = @"List available devices and interfaces.",
            Hidden = false)]
        public class ListVerb : ICommand
        {
            public void Execute()
            {
                ListInterfaces();
                System.Console.WriteLine();
                ListDevices();
            }
        }

        [Verb("device",
           HelpText = @"
                List devices or a show information for a single device.\n
                \tTo list all devices use: `ic4-ctrl device`\n
                \tTo show only a specific device: `ic4-ctrl device \""<id>\""\n",
           Hidden = false)]
        public class DeviceVerb : ICommand
        {
            [Option("device-id", 
                Required = false, 
                HelpText = @"If specified only information for this device is printed, otherwise all device are listed. You can specify an index e.g. '0'.")]
            public string DeviceId { get; set; }

            public void Execute()
            {
                if(string.IsNullOrEmpty(DeviceId))
                {
                    ListDevices();
                }
                else
                {
                    PrintDevice(DeviceId);
                }
            }
        }


        [Verb("interface",
          HelpText = @"
            List devices or a show information for a single interface.\n
            \tTo list all interfaces: `ic4-ctrl interface`\n
            \tTo show only a specific interface: `ic4-ctrl interface \""<id>\""\n",
          Hidden = false)]
        public class InterfaceVerb : ICommand
        {
            [Option("interface-id",
                Required = false,
                HelpText = @"If specified only information for this interface is printed, otherwise all interfaces are listed. You can specify an index e.g. '0'.")]
            public string InterfaceId { get; set; }

            public void Execute()
            {
                if (string.IsNullOrEmpty(InterfaceId))
                {
                    ListInterfaces();
                }
                else
                {
                    PrintInterface(InterfaceId);
                }
            }
        }

        [Verb("prop",
            HelpText = @"
                List or set property values of the specified device or interface.\n
                \tTo list all device properties 'ic4-ctrl prop <device-id>'.\n
                \tTo list specific device properties 'ic4-ctrl prop <device-id> ExposureAuto ExposureTime'.\n
                \tTo set specific device properties 'ic4-ctrl prop <device-id> ExposureAuto=Off ExposureTime=0.5'.",
            Hidden = false)]
        public class PropVerb : ICommand
        {
            [Option("forceinterface", HelpText = "If set the <device-id> is interpreted as an interface-id.")]
            public bool ForceInterface { get; set; }

            public string PropertyId { get; set; }

            [Option("device-id",
               Required = true,
               HelpText = @"Specifies the device to open. You can specify an index e.g. '0'.")]
            public string DeviceId { get; set; }


            [Value(0,
              Required = false,
              HelpText = @"list specific device properties")]
            public IEnumerable<string> Remaining { get; set; }

            public void Execute()
            {
                ExecPropCmd( DeviceId, ForceInterface, Remaining.ToArray());
            }
        }

        [Verb("save-prop",
           HelpText = @"
                Save properties for the specified device 'ic4-ctrl save-prop -f <filename> <device-id>'.",
           Hidden = false)]
        public class SavePropVerb : ICommand
        {
            [Option("filename", HelpText = "Filename to save into.", Required = true)]
            public string Filename { get; set; }

            [Option("device-id",
             Required = true,
             HelpText = @"Specifies the device to open. You can specify an index e.g. '0'.")]
            public string DeviceId { get; set; }

            public void Execute()
            {
                SaveProperties(DeviceId, false, Filename);
            }

        }

        [Verb("image",
           HelpText = @"Save one or more images from the specified device 'ic4-ctrl image -f <filename> --count 3 --timeout 2000 --type bmp <device-id>'.",
           Hidden = false)]
        public class ImageVerb : ICommand
        {
            [Option("filename", 
                HelpText = "Filename to save into.", 
                Required = true)]
            public string Filename { get; set; }

            [Option("device-id",
             Required = true,
             HelpText = @"Specifies the device to open. You can specify an index e.g. '0'.")]
            public string DeviceId { get; set; }

            [Option("count",
                Default = 1,
            Required = false,
            HelpText = @"Count of frames to capture.")]
            public int Count { get; set; }

            [Option("timeout",
                Default =1000,
              Required = false,
              HelpText = @"Timeout in milliseconds.")]
            public int Timeout { get; set; }

            [Option("type",
              Default = "bmp",
            Required = false,
            HelpText = @"Image file type to save. [bmp,png,jpeg,tiff]")]
            public string ImageType { get; set; }

            public void Execute()
            {
                SaveImage(DeviceId, Filename, Count, Timeout, ImageType);
            }
        }

        [Verb("live",
         HelpText = @"Display a live stream. 'ic4-ctrl live <device-id>'.",
         Hidden = false)]
        public class LiveVerb : ICommand
        {
            [Option("device-id",
            Required = true,
            HelpText = @"Specifies the device to open. You can specify an index e.g. '0'.")]
            public string DeviceId { get; set; }

            public void Execute()
            {
                ShowLive(DeviceId);
            }
        }


        [Verb("show-prop",
         HelpText = @"Display the property page for the device or interface id. 'ic4-ctrl show-prop <id>'.",
         Hidden = false)]
        public class ShowPropVerb : ICommand
        {
            [Option("device-id",
            Required = true,
            HelpText = @"Specifies the device to open. You can specify an index e.g. '0'.")]
            public string DeviceId { get; set; }

            [Option("forceinterface", HelpText = "If set the <device-id> is interpreted as an interface-id.")]
            public bool ForceInterface { get; set; }

            public void Execute()
            {
                ShowPropPage(DeviceId, ForceInterface);
            }
        }

        static void Main(string[] args)
        {
            try
            {
                var result = Parser.Default.ParseArguments<
                    ListVerb, 
                    DeviceVerb, 
                    InterfaceVerb, 
                    PropVerb, 
                    SavePropVerb,
                    ImageVerb,
                    LiveVerb,
                    ShowPropVerb>(args)
                        .WithParsed(v => (v as ICommand)?.Execute());
            }
            catch (Exception ex)
            {
                System.Console.WriteLine(ex.ToString());
            }
        }
    }
}
