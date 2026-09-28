import sys
import imagingcontrol4 as ic4

# This program shows how to configure the IP Address and subnet mask of a GigE Vision camera.
#
# Please note: There is a pre-compiled program ic4-ipconfig_cmd.exe that is distributed with the
# The Imaging Source GenTL Producer for GigE Vision Cameras.
# That utility can do everything this example shows, but also recover devices that are misconfigured.
#
# The IP Configuration of the camera is accessed through the driver's property map obtained
# from Grabber.driver_property_map():
# - Setting GevDeviceIPConfigPersistentIPEnable to true sets the camera to a fixed IP address
# - Setting GevDeviceIPConfigDHCPEnable to true enables the camera to perform an IP address via DHCP
# - If both are not set, the camera uses Link-Local Auto-IP (LLA)
# If GevDeviceIPConfigPersistentIPEnable is set, the IP configuration is specified by 3 more parameters:
# - GevDeviceIPConfigPersistentIPAddress is the persistent IP address
# - GevDeviceIPConfigPersistentSubnetMask is the subnet mask
# - GevDeviceIPConfigPersistentGateway is the default gateway
# Writing to any of these features has no effect until GevDeviceIPConfigApply is executed.


# Print help on how to use this program
def print_usage():
    print("Usage:")
    print("ip-configuration.py <IDENTIFIER> [<IP> [<SUBNET> [GATEWAY]]]")

    print("\t<IDENTIFIER> is the identifier of a connected GigE Vision camera, e.g. its")
    print("\t\tSerial Number")
    print("\t\tIP Address")
    print("\t\tModel Name (if currently unique)")

    print("\t<IP> is either")
    print("\t\t'auto' to use Link-Local Auto-IP (LLA)")
    print("\t\t'dhcp' to enable DHCP")
    print("\t\tan IP Address in the format 192.168.0.27 that is configured to be used persistently")
    print("\t\tnot present -- in this case the program prints the current IP configuration of <IDENTIFIER>")

    print("\t<SUBNET> is the subnet mask to be used if <IP> is an IP Address. (Optional, default is 0.0.0.0)")
    print("\t<GATEWAY> is the default gateway to configure if <IP> is an IP Address. (Optional, default is 0.0.0.0)")


def example_ip_configuration():
    try:
        args = sys.argv[1:]

        if len(args) < 1 or len(args) > 4:
            print_usage()
            return 0

        # Open the device specified by <IDENTIFIER>
        grabber = ic4.Grabber(args[0])

        # Query device info to get access to later print model name and serial number
        info = grabber.device_info

        # IP Configuration is accessed via the driver's property map
        map = grabber.driver_property_map

        if len(args) == 1:
            print(f"Current IP Configuration for device " f"{info.model_name} ({info.serial})")

            print(f"IP Address: {map.get_value_str('GevDeviceIPAddress')}")
            print(f"Subnet Mask: {map.get_value_str('GevDeviceSubnetMask')}")
            print(f"Default Gateway: {map.get_value_str('GevDeviceGateway')}")
        else:
            ip = args[1]

            if ip == "auto":
                # Disable both PersistentIP and DHCP modes, so that LLA remains
                map.set_value("GevDeviceIPConfigPersistentIPEnable", False)
                map.set_value("GevDeviceIPConfigDHCPEnable", False)

            elif ip == "dhcp":
                # Disable PersistentIP and enable DHCP
                map.set_value("GevDeviceIPConfigPersistentIPEnable", False)
                map.set_value("GevDeviceIPConfigDHCPEnable", True)

            else:
                # Disable PersistentIP mode
                map.set_value("GevDeviceIPConfigPersistentIPEnable", True)

                # Set the IP address
                map.set_value("GevDeviceIPConfigPersistentIPAddress", ip)

                # Set the subnet mask
                if len(args) > 2:
                    map.set_value("GevDeviceIPConfigPersistentSubnetMask", args[2])
                else:
                    # This default may be wrong, depending on the value of 'ip'.
                    # Sanity checks and automated generation are omitted for clarity.
                    map.set_value("GevDeviceIPConfigPersistentSubnetMask", "255.255.255.0")

                # Set the default gateway
                if len(args) > 3:
                    map.set_value("GevDeviceIPConfigPersistentGateway", args[3])
                else:
                    map.set_value("GevDeviceIPConfigPersistentGateway", "0.0.0.0")

            # Let the settings go active as configured
            map.execute_command("GevDeviceIPConfigApply")

            # Print summary of what was configured
            print(f"Device {info.model_name} ({info.serial}) was reconfigured.")

        return 0

    except Exception as ex:
        print("An exception occurred:")
        print(str(ex))
        return -10


if __name__ == "__main__":
    with ic4.Library.init_context():
        sys.exit(example_ip_configuration())
