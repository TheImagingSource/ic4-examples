using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using CommandLine;
using ic4;
// prop --device-id=test123 --forceinterface prop1 prop2

namespace ic4_ctrl
{
    internal class Program
    {
        private static void print(int offset, string text)
        {
            for (int i = 0; i < offset; ++i)
            {
                System.Console.Write("\t");
            }
            System.Console.Write(text);
        }

        private static void list_interfaces()
        {
            var list = DeviceEnum.Interfaces;

            System.Console.WriteLine("Interface list:");

            foreach (var e in list)
            {
                System.Console.WriteLine("\t{e.Name}");
                System.Console.WriteLine("\tTransportLayerName: {e.TransportLayerName}");
            }

            if (!list.Any())
            {
                System.Console.WriteLine("No Interfaces found");
            }
        }

        private static ic4.Interface find_interface(string id)
        {
            var list = DeviceEnum.Interfaces;
            if (!list.Any())
            {
                throw new Exception("No Interfaces are available.");
            }


            foreach (var itf in list)
            {
                if(itf.Name == id)
                {
                    return itf;
                }
            }

            foreach (var itf in list)
            {
                if (itf.TransportLayerName == id)
                {
                    return itf;
                }
            }

            try
            {
                int index = Int32.Parse(id);
                if( index < 0 || index >= list.Count())
                {
                    return null;
                }
                return list.ToArray()[index];
            }
            catch
            {
            }
            return null;
        }
        private static void print_interface(string interface_id)
        {
            var itf = find_interface(interface_id);
            if (itf == null)
            {
                throw new Exception("Failed to find device for id '{id}'");
            }

            System.Console.WriteLine("Name: '{itf.Name}'");
            System.Console.WriteLine("TransportLayerName: '{itf.TransportLayerName}'");
            System.Console.WriteLine("TransportLayerType: '{itf.TransportLayerType}'");
            System.Console.WriteLine("TransportVersion: '{itf.TransportVersion}'");

            System.Console.WriteLine("Interface Properties:\n");
            var map = itf.PropertyMap;
            foreach(var property in map.All)
            {
                print_property(1, property);
            }
        }

        private static string fetch_PropertyMethod_value<T>(ic4.Property prop, Expression<Func<T>> address)
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
        private static string fetch_PropertyMethod_value<T>(ic4.Property prop, Expression<Func<T>> address, ic4.IntRepresentation int_rep)
        {
            var propertyInfo = ((MemberExpression)address.Body).Member as PropertyInfo;
            if (propertyInfo == null)
            {
                throw new ArgumentException("The lambda expression 'property' should point to a valid Property");
            }

            long v = 0;

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
                case ic4.IntRepresentation.Boolean: return string.Format("{}", v != 0 ? 1 : 0);
                case ic4.IntRepresentation.HexNumber: return string.Format("0x{:X}", v);
                case ic4.IntRepresentation.IP4Address:
                    {
                        ulong v0 = ((ulong)v >> 0) & 0xFF;
                        ulong v1 = ((ulong)v >> 8) & 0xFF;
                        ulong v2 = ((ulong)v >> 16) & 0xFF;
                        ulong v3 = ((ulong)v >> 24) & 0xFF;
                        return string.Format("{}.{}.{}.{}", v3, v2, v1, v0);
                    }
                case ic4.IntRepresentation.MacAddress:
                    {
                        ulong v0 = ((ulong)v >> 0) & 0xFF;
                        ulong v1 = ((ulong)v >> 8) & 0xFF;
                        ulong v2 = ((ulong)v >> 16) & 0xFF;
                        ulong v3 = ((ulong)v >> 24) & 0xFF;
                        ulong v4 = ((ulong)v >> 32) & 0xFF;
                        ulong v5 = ((ulong)v >> 40) & 0xFF;
                        return string.Format("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", v5, v4, v3, v2, v1, v0);
                    }
                case ic4.IntRepresentation.Linear:
                case ic4.IntRepresentation.Logarithmic:
                case ic4.IntRepresentation.PureNumber:
                default:
                    return string.Format("{}", v);
            }
        }

        private static void print_property(int offset, Property property)
        {
            var prop_type = property.Type;
            print(offset + 0, "{property.Name} - Type: {prop_type.ToString()}, DisplayName: {property.DisplayName}\n");
            print(offset + 1, "Description: {property.Description}\n");
            print(offset + 1, "Tooltip: {property.Tooltip}\n");
            print(offset + 3, "\n");
            print(offset + 1, "Visibility: {property.Visibility.ToString()}, Available: {property.IsAvailable}, Locked: {property.IsLocked}, ReadOnly: {property.IsReadonly}\n");

            if (property.IsSelector)
            {
                print(offset + 1, "Selected properties:\n");
                foreach( var selected in property.SelectedProperties)
                {
                    print(offset + 2, "{selected.Name}\n");
                }
            }

            switch (prop_type)
            {
                case PropertyType.Integer:
                    {
                        PropInteger prop = property as ic4.PropInteger;
                        var inc_mode = prop.IncrementMode;
                        var rep = prop.Representation;

                        print(offset + 1, "Representation: '{rep}', Unit: '{prop.Unit}', IncrementMode: '{inc_mode}'\n");

                        if (prop.IsAvailable)
                        {
                            if (!prop.IsReadonly)
                            {
                                var min = prop.Minimum;
                                var max = prop.Maximum;

                                print(offset + 1, string.Format("Min: {}, Max: {}\n",
                                    fetch_PropertyMethod_value<long>(prop, () => prop.Minimum, rep),
                                    fetch_PropertyMethod_value<long>(prop, () => prop.Maximum, rep))
                                );
                            }
                            if (inc_mode == ic4.PropertyIncrementMode.Increment)
                            {
                                if (!prop.IsReadonly)
                                {
                                    print(offset + 1, string.Format("Inc: {}\n",
                                        fetch_PropertyMethod_value<long>(prop, () => prop.Increment, rep))
                                    );
                                }
                            }
                            else if (inc_mode == ic4.PropertyIncrementMode.ValueSet)
                            {
                                try
                                {
                                    var vvset = prop.ValidValueSet;
                                    print(offset + 1, "ValidValueSet:");
                                    foreach(var val in  vvset )
                                    {
                                        print(offset + 2, string.Format("{}\n", val));
                                    }
                                    System.Console.Write("\n");
                                }
                                catch
                                {
                                    print(offset + 1, "Failed to fetch ValidValueSet\n");
                                }
                               
                            }
                            print(offset + 1, string.Format("Value: {}\n",
                                fetch_PropertyMethod_value<long>(prop, () => prop.Value, rep))
                            );
                        }
                        break;
                    }
                case PropertyType.Float:
                    {
                        var prop = property as PropFloat;
                        var inc_mode = prop.IncrementMode;

                        print(offset + 1, string.Format("Representation: '{}', Unit: '{}', IncrementMode: '{}', DisplayNotation: {}, DisplayPrecision: {}\n",
                            prop.Representation, prop.Unit, inc_mode, prop.DisplayNotation, prop.DisplayPrecision));

                        if (prop.IsAvailable)
                        {
                            if (!prop.IsReadonly)
                            {
                                print(offset + 1, string.Format("Min: {}, Max: {}\n",
                                    fetch_PropertyMethod_value<double>(prop, () => prop.Minimum),
                                    fetch_PropertyMethod_value<double>(prop, () => prop.Maximum))
                                );
                            }

                            if (inc_mode == PropertyIncrementMode.Increment)
                            {
                                print(offset + 1, string.Format("Inc: {}\n",
                                    fetch_PropertyMethod_value<double>(prop, () => prop.Increment))
                                );
                            }
                            else if (inc_mode == PropertyIncrementMode.ValueSet)
                            {
                                //std::vector<double> vvset;
                                //if( !prop.getValidValueSet( vvset, ic4::ignoreError ) ) {
                                //    print( offset + 1, "Failed to fetch ValidValueSet\n" );
                                //}
                                //else
                                //{
                                //    print( offset + 1, "ValidValueSet:" );
                                //    for( auto&& val : vvset ) {
                                //        print( offset + 2, "{}\n", val );
                                //    }
                                //    print( "\n" );
                                //}
                            }

                            print(offset + 1, string.Format("Value: {}\n",
                                fetch_PropertyMethod_value<double>(prop, () => prop.Value))
                            );
                        }
                        break;
                    }
                case PropertyType.Enumeration:
                    {
                        var prop = property as PropEnumeration;
                        print(offset + 1, "EnumEntries:\n");
                        foreach (var entry in  prop.Entries)
                        {
                            var prop_enum_entry = entry as PropEnumEntry;

                            print_property(offset + 2, prop_enum_entry);
                            print(0, "\n");
                        }

                        if (prop.IsAvailable)
                        {
                            try
                            {
                                var selected_entry = prop.SelectedEntry;
                                print(offset + 1, string.Format("Value: {}, SelectedEntry.Name: '{}'\n",
                                    fetch_PropertyMethod_value<long>(prop, () => prop.Value),
                                    selected_entry.Name)
                                );
                            }
                            catch
                            {
                                print(offset + 1, string.Format("Value: {}, SelectedEntry.Name: '{}'\n", "err", "err"));
                            }   
                        }
                        break;
                    }
                case PropertyType.Boolean:
                    {
                        var prop = property as PropBoolean;

                        if (prop.IsAvailable)
                        {
                            print(offset + 1, string.Format("Value: {}\n",
                                fetch_PropertyMethod_value<bool>(prop, () => prop.Value))
                            );
                        }
                        break;
                    }
                case PropertyType.String:
                    {
                        var prop = property as PropString;

                        if (prop.IsAvailable)
                        {
                            print(offset + 1, string.Format("Value: '{}', MaxLength: {}\n",
                                fetch_PropertyMethod_value<string> (prop, () => prop.Value),
                                fetch_PropertyMethod_value<ulong>(prop, () => prop.MaxLength))
                            );
                        }
                        break;
                    }
                case PropertyType.Command:
                    {
                        print(0, "\n");
                        break;
                    }
                case PropertyType.Category:
                    {
                        var prop = property as PropCategory;
                        print(offset + 1, "Features:\n");
                        foreach (var feature in prop.Features )
                        {
                            print(offset + 2, string.Format("{}\n", feature.Name));
                        }
                        break;
                    }
                case PropertyType.Register:
                    {
                        var prop = property as PropRegister;

                        print(offset + 1, string.Format("Size: {}\n",
                            fetch_PropertyMethod_value<ulong>(prop, () => prop.Size))
                        );
                        if (prop.IsAvailable)
                        {
                            try
                            {
                                var vec = prop.Value;
                                string str = string.Empty;
                                int max_entries_to_print = 16;
                                for (int i = 0; i < Math.Min(max_entries_to_print, vec.Length); ++i)
                                {
                                    str += string.Format("{:x}", vec[i]);
                                    str += ", ";
                                }
                                if (vec.Length > max_entries_to_print)
                                {
                                    str += "...";
                                }
                                print(offset + 1, string.Format("Value: [{}], Value-Size: {}\n", str, vec.Length));
                            }
                            catch
                            {
                                print(offset + 1, "Value: 'err'");
                            }
                           
                        }
                        print(0, "\n");
                        break;
                    }
                case PropertyType.Port:
                    {
                        print(0, "\n");
                        break;
                    }
                case PropertyType.EnumEntry:
                    {
                        var prop = property as PropEnumEntry;

                        if (prop.IsAvailable)
                        {
                            print(offset + 1, string.Format("Value: {}\n", fetch_PropertyMethod_value<long>(prop, () => prop.Value)));
                        }
                        print(0, "\n");
                        break;
                    }
            };
            print(0, "\n");
        }

        private static void list_devices()
        {
            var list = DeviceEnum.Devices;
            
            System.Console.WriteLine("Device list:\n");
            System.Console.WriteLine("ModelName     Serial  InterfaceName");
            int index = 0;
            foreach(var device in list)
            {
                // TODO
                //  print( "{:>3} {:24} {:8} {}\n", index, e.getModelName(), e.getSerial(), e.getInterface().getTransportLayerName() );
                System.Console.WriteLine("{index,24} {device.ModelName} { device.Serial} {device.Interface.TransportLayerName}\n");
                index++;
            }
            if (!list.Any())
            {
                System.Console.WriteLine("    No devices found\n");
            }
        }

        private static void print_device(string device_id)
        {
            Console.WriteLine("print_device:  " + device_id.ToString());
        }

        private static void exec_prop_cmd(string id, bool force_interface, string[] lst)
        {
            System.Console.WriteLine("prop --forceinterface=" + force_interface.ToString() + " --device-id=" + id + " " + string.Join(" ", lst));
        }

        private interface ICommand
        { 
            void Execute();
        }


        [Verb("list", 
            HelpText = @"List available devices and interfaces.",
            Hidden = false)]
        public class list_verb : ICommand
        {
            public void Execute()
            {
                list_interfaces();
                System.Console.WriteLine();
                list_devices();
            }
        }

        [Verb("device",
           HelpText = @"
                List devices or a show information for a single device.\n
                \tTo list all devices use: `ic4-ctrl device`\n
                \tTo show only a specific device: `ic4-ctrl device \""<id>\""\n",
           Hidden = false)]
        public class device_verb : ICommand
        {
            [Option("device-id", 
                Required = false, 
                HelpText = @"If specified only information for this device is printed, otherwise all device are listed. You can specify an index e.g. '0'.")]
            public string arg_device_id { get; set; }

            public void Execute()
            {
                if(string.IsNullOrEmpty(arg_device_id))
                {
                    list_devices();
                }
                else
                {
                    print_device(arg_device_id);
                }
            }
        }


        [Verb("interface",
          HelpText = @"
            List devices or a show information for a single interface.\n
            \tTo list all interfaces: `ic4-ctrl interface`\n
            \tTo show only a specific interface: `ic4-ctrl interface \""<id>\""\n",
          Hidden = false)]
        public class interface_verb : ICommand
        {
            [Option("interface-id",
                Required = false,
                HelpText = @"If specified only information for this interface is printed, otherwise all interfaces are listed. You can specify an index e.g. '0'.")]
            public string interface_id { get; set; }

            public void Execute()
            {
                if (string.IsNullOrEmpty(interface_id))
                {
                    list_interfaces();
                }
                else
                {
                    print_interface(interface_id);
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
        public class prop_verb : ICommand
        {
            [Option("forceinterface", HelpText = "If set the <device-id> is interpreted as an interface-id.")]
            public bool force_interface { get; set; }

            public string prop_id { get; set; }

            [Option("device-id",
               Required = true,
               HelpText = @"Specifies the device to open. You can specify an index e.g. '0'.")]
            public string arg_device_id { get; set; }


            [Value(0,
              Required = false,
              HelpText = @"list specific device properties")]
            public IEnumerable<string> remaining { get; set; }

            public void Execute()
            {
                exec_prop_cmd( arg_device_id, force_interface, remaining.ToArray());
            }
        }

        static void Main(string[] args)
        {
            try
            {
                var result = Parser.Default.ParseArguments<list_verb, device_verb, interface_verb, prop_verb>(args)
                    .WithParsed(v => (v as ICommand)?.Execute());
            }
            catch (Exception ex)
            {
               System.Console.WriteLine(ex.ToString());
            }
        }
    }
}
