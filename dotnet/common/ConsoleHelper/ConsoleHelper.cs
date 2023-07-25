using System.Collections.Generic;
using System;

namespace ic4
{
    public static class ConsoleHelper
    {
        public static T PresentUserChoice<T>(IReadOnlyList<T> items, Func<T, string> getName, string header) where T : class
        {
            Console.WriteLine(header);

            for (int index = 0; index < items.Count; ++index)
            {
                var itemName = getName(items[index]);

                Console.WriteLine($"[{index}] {itemName}");
            }

            while (true)
            {
                Console.Write("Select index: ");

                var input = Console.ReadLine();
                if (string.IsNullOrEmpty(input))
                {
                    Console.WriteLine("> Cancelled");
                    return null;
                }

                if (!int.TryParse(input, out int selectedIndex))
                {
                    Console.WriteLine("> Invalid input");
                    continue;
                }

                if (selectedIndex < 0 || selectedIndex >= items.Count)
                {
                    Console.WriteLine("> Invalid index");
                    continue;
                }

                return items[selectedIndex];
            }
        }

        public static DeviceInfo PresentUserChoice(IReadOnlyList<DeviceInfo> deviceList, string prompt)
        {
            Func<DeviceInfo, string> buildDeviceString = (DeviceInfo di) =>
            {
                return $"{di.ModelName} ({di.Serial}) [{di.Interface.TransportLayerName}]";
            };

            return PresentUserChoice(deviceList, buildDeviceString, prompt);
        }
    }
}