using System;
namespace DeviceLost
{
    class Program
    {
        static void Main(string[] args)
        {
            ic4.Library.Init();
           
            var deviceList = ic4.DeviceEnum.Devices;
            var it = ic4.ConsoleHelper.PresentUserChoice(deviceList, "Select Device:");
            
            var grabber = new ic4.Grabber();
            grabber.DeviceOpen(it);
            grabber.DeviceLost += DeviceLostHandler;

            Console.WriteLine($"Opened device {grabber.DeviceInfo.ModelName} ({grabber.DeviceInfo.Serial})");
            Console.WriteLine("Disconnect device to produce device-lost event");

            Console.WriteLine("Press Enter to exit program");
            Console.ReadKey();

            return;
        }
        static void DeviceLostHandler(object sender, EventArgs e)
        {
            Console.WriteLine("Device Lost!");
        }
    }
}
