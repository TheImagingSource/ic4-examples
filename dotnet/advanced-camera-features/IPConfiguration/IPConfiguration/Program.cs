using System;
using ic4;

namespace IPConfiguration
{
    class IpConfiguration
    {
        // This program shows how to configure the IP Address and subnet mask of a GigE Vision camera.
        //
        // Please note: There is a pre-compiled program ic4-ipconfig_cmd.exe that is distributed with the
        // The Imaging Source GenTL Producer for GigE Vision Cameras.
        // That utility can do everything this example shows, but also recover devices that are misconfigured.
        //
        // The IP Configuration of the camera is accessed through the driver's property map obtained
        // from Grabber.DriverPropertyMap():
        // - Setting GevDeviceIPConfigPersistentIPEnable to true sets the camera to a fixed IP address
        // - Setting GevDeviceIPConfigDHCPEnable to true enables the camera to perform an IP address via DHCP
        // - If both are not set, the camera uses Link-Local Auto-IP (LLA)
        // If GevDeviceIPConfigPersistentIPEnable is set, the IP configuration is specified by 3 more parameters:
        // - GevDeviceIPConfigPersistentIPAddress is the persistent IP address
        // - GevDeviceIPConfigPersistentSubnetMask is the subnet mask
        // - GevDeviceIPConfigPersistentGateway is the default gateway
        // Writing to any of these features has no effect until GevDeviceIPConfigApply is executed.

        // Print help on how to use this program
        static void PrintUsage()
        {
            Console.WriteLine("Usage:");
            Console.WriteLine("ip-configuration <IDENTIFIER> [<IP> [<SUBNET> [GATEWAY]]]");

            Console.WriteLine("\t<IDENTIFIER> is the identifier of a connected GigE Vision camera, e.g. its");
            Console.WriteLine("\t\tSerial Number");
            Console.WriteLine("\t\tIP Address");
            Console.WriteLine("\t\tModel Name (if currently unique)");

            Console.WriteLine("\t<IP> is either");
            Console.WriteLine("\t\t'auto' to use Link-Local Auto-IP (LLA)");
            Console.WriteLine("\t\t'dhcp' to enable DHCP");
            Console.WriteLine("\t\tan IP Address in the format 192.168.0.27 that is configured to be used persistently");
            Console.WriteLine("\t\tnot present -- in this case the program prints the current IP configuration of <IDENTIFIER>");

            Console.WriteLine("\t<SUBNET> is the subnet mask to be used if <IP> is an IP Address. (Optional, default is 0.0.0.0)");
            Console.WriteLine("\t<GATEWAY> is the default gateway to configure if <IP> is an IP Address. (Optional, default is 0.0.0.0)");
        }

        static int Main(string[] args)
        {
            try
            {
                // Initialize the library with sensible defaults:
                // - Throw exceptions on errors
                // - Log errors and warnings from API calls
                // - Log to stdout and Windows debug log
                Library.Init();

                if (args.Length < 1 || args.Length > 4)
                {
                    PrintUsage();
                }
                else
                {
                    var grabber = new Grabber();

                    // Open the device specified by <IDENTIFIER>.
                    // Use "select" as the identifier to choose a device interactively.
                    if (args[0] == "select")
                    {
                        var selectedDevice = ConsoleHelper.PresentUserChoice(DeviceEnum.Devices, "Select device:");

                        grabber.DeviceOpen(selectedDevice);
                    }
                    else
                    {
                        grabber.DeviceOpen(args[0]);
                    }

                    // Query device info to get access to later print model name and serial number
                    var info = grabber.DeviceInfo;

                    // IP Configuration is accessed via the driver's property map
                    var map = grabber.DriverPropertyMap;

                    if (args.Length == 1)
                    {
                        Console.WriteLine($"Current IP Configuration for device {info.ModelName} ({info.Serial})");

                        Console.WriteLine($"IP Address: {map.GetValueString("GevDeviceIPAddress")}");
                        Console.WriteLine($"Subnet Mask: {map.GetValueString("GevDeviceSubnetMask")}");
                        Console.WriteLine($"Default Gateway: {map.GetValueString("GevDeviceGateway")}");
                    }
                    else
                    {
                        string ip = args[1];

                        if (ip == "auto")
                        {
                            // Disable both PersistentIP and DHCP modes, so that LLA remains
                            map.SetValue("GevDeviceIPConfigPersistentIPEnable", false);
                            map.SetValue("GevDeviceIPConfigDHCPEnable", false);
                        }
                        else if (ip == "dhcp")
                        {
                            // Disable PersistentIP and enable DHCP
                            map.SetValue("GevDeviceIPConfigPersistentIPEnable", false);
                            map.SetValue("GevDeviceIPConfigDHCPEnable", true);
                        }
                        else
                        {
                            // Disable PersistentIP mode
                            map.SetValue("GevDeviceIPConfigPersistentIPEnable", true);

                            // Set the IP address
                            map.SetValue("GevDeviceIPConfigPersistentIPAddress", ip);

                            // Set the subnet mask
                            if (args.Length > 2)
                            {
                                map.SetValue("GevDeviceIPConfigPersistentSubnetMask", args[2]);
                            }
                            else
                            {
                                // This default may be wrong, depending on the value of 'ip'.
                                // Sanity checks and automated generation are omitted for clarity.
                                map.SetValue("GevDeviceIPConfigPersistentSubnetMask", "255.255.255.0");
                            }

                            // Set the default gateway
                            if (args.Length > 3)
                            {
                                map.SetValue("GevDeviceIPConfigPersistentGateway", args[3]);
                            }
                            else
                            {
                                map.SetValue("GevDeviceIPConfigPersistentGateway", "0.0.0.0");
                            }
                        }

                        // Let the settings go active as configured
                        map.ExecuteCommand("GevDeviceIPConfigApply");

                        // Print summary of what was configured
                        Console.WriteLine($"Device {info.ModelName} ({info.Serial}) was reconfigured.");
                    }
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("An exception occurred:");
                Console.Error.WriteLine(ex.Message);
                return -10;
            }

            return 0;
        }
    }
}
