using System;
using System.Linq;

namespace DeviceEnumeration
{
    internal class Program
    {
        static string FormatDeviceInfo(ic4.DeviceInfo deviceInfo)
        {
            return $"Model: {deviceInfo.ModelName} Serial: {deviceInfo.Serial} Version {deviceInfo.Version}";
        }

        static void PrintDeviceList()
        {
            Console.WriteLine("Enumerating all attached video capture devices in a single list...");

            var deviceList = ic4.DeviceEnum.Devices.ToList();

            if (deviceList.Count == 0)
            {
                Console.WriteLine("No devices found");
            }

            Console.WriteLine($"Found {deviceList.Count} devices:");

            foreach( var deviceInfo in deviceList )
            {
                Console.WriteLine($"\t{FormatDeviceInfo(deviceInfo)}");
            }

            Console.WriteLine();
        }

        static void PrintInterfaceDeviceTree()
        {
            Console.WriteLine("Enumerating video capture devices by interface...");

            var interfaceList = ic4.DeviceEnum.Interfaces.ToList();

            foreach (var itf in interfaceList)
            {
                Console.WriteLine($"Interface: {itf.DisplayName}");
                Console.WriteLine($"\tProvided by {itf.TransportLayerName} [TLType: {itf.TransportLayerType}]");
                var deviceList = itf.Devices.ToList();

                if (deviceList.Count == 0)
                {
                    Console.WriteLine("\tNo devices found");
                }

                Console.WriteLine($"\tFound {deviceList.Count} devices:");

                foreach (var deviceInfo in deviceList)
                {
                    Console.WriteLine($"\t\t{FormatDeviceInfo(deviceInfo)}");
                }
            }

            Console.WriteLine();
        }

        static void Main(string[] args)
        {
            ic4.Library.Init();
            PrintDeviceList();
            PrintInterfaceDeviceTree();
        }
    }
}