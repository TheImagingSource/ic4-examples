using System;

namespace DeviceListChanged
{
    class Program
    {
        static void Main(string[] args)
        {
            ic4.Library.Init();

            var deviceEnum = new ic4.DeviceEnum();
            deviceEnum.DeviceListChanged += DeviceListChanged;

            var deviceCount = ic4.DeviceEnum.Devices.Count;

            Console.WriteLine("Press ENTER to exit program");
            Console.WriteLine($"{deviceCount} devices connected initially.");
            Console.WriteLine("");

            Console.Read();

            deviceEnum.Dispose();

            return;
        }

        static void DeviceListChanged(object sender, EventArgs e)
        {
            var deviceList = ic4.DeviceEnum.Devices;

            var newDeviceCount = deviceList.Count;

            Console.WriteLine("Device list has changed!");
            Console.WriteLine($"Found {newDeviceCount} devices");
            Console.WriteLine("");
        }
    }
}
