"""
This program demonstrate how to save and load device settings to and from configuration files and memory buffers.

The example is split into 2 sections, demonstrating different modes of device settings serialization:

Section 1 demonstrates how to store the Grabber's device state and reopen the device from the state file:
- Save the Grabber's device state to a settings file
- Modify a device property (Width)
- Close the device
- Reopen the device using the settings file, restoring the original Width value

The settings file contains both a device identifier and the device settings, so that the device can be reopened and its
settings restored.
There are function overloads to store the Grabber's device state into a memory buffer instead.

In section 2, only the settings inside the device's PropertyMap are saved and restored:
- Save the device's PropertyMap into a memory buffer
- Modify a device property (Width)
- Restore the device PropertyMap's state from the memory buffer

The settings buffer only contains the PropertyMap's settings.
Therefore, this approach can be used to transfer camera settings from one device to another.
There are function overloads to store the PropertyMap's state into a file instead.
"""

import sys
import imagingcontrol4 as ic4


# Helper function to print info about the device opened in the passed Grabber
def print_device_info(label, grabber):
    if grabber.is_device_valid:
        info = grabber.device_info
        prop_map = grabber.device_property_map

        model_name = info.model_name
        width = prop_map.get_value_int(ic4.PropId.WIDTH)

        print(f"{label} -- {model_name} -- Width: {width}")
    else:
        print(f"{label} -- No Device Opened")


def example_device_settings_serialization():
    try:
        # Let the user select a camera
        device_list = ic4.DeviceEnum.devices()
        if not device_list:
            return -1

        for index, device in enumerate(device_list):
            print(f"{index}: {device.model_name} ({device.serial})")

        selection = input("Select a device: ")
        try:
            index = int(selection)
            if index < 0 or index >= len(device_list):
                return -1
        except ValueError:
            return -1

        selected_device = device_list[index]

        grabber = ic4.Grabber()
        print_device_info("Before opening device", grabber)

        # Open the selected device
        grabber.device_open(selected_device)
        print_device_info("After opening device", grabber)

        # Reset camera settings to default by executing UserSetLoad[Default]
        prop_map = grabber.device_property_map
        prop_map.set_value(ic4.PropId.USER_SET_SELECTOR, "Default")
        prop_map.execute_command(ic4.PropId.USER_SET_LOAD)
        print_device_info("After resetting device to defaults", grabber)

        # Section 1: Save Grabber's device state into a file and restore it
        DEVICE_STATE_FILE_NAME = "settings.json"

        # Save device state
        grabber.device_save_state_to_file(DEVICE_STATE_FILE_NAME)
        print(f">>> Saved device state into {DEVICE_STATE_FILE_NAME}")

        # Modify a property for demonstration
        prop_map.set_value_minimum(ic4.PropId.WIDTH)
        print_device_info("After setting Width to minimum", grabber)

        # Close the device
        grabber.device_close()
        print_device_info("After closing device", grabber)

        # Reopen using device state file
        grabber.device_open_from_state_file(DEVICE_STATE_FILE_NAME)
        print_device_info("After opening device from file", grabber)

        # Section 2: Save PropertyMap's state into a memory buffer and restore it

        # prop_map has become invalid after the device was closed above
        prop_map = grabber.device_property_map

        # Save PropertyMap's state into a buffer
        buffer = prop_map.serialize()
        print(">>> Saved property map state into memory buffer")

        # Modify a property for demonstration
        prop_map.set_value_minimum(ic4.PropId.WIDTH)
        print_device_info("After setting Width to minimum", grabber)

        # Restore PropertyMap state from buffer
        prop_map.deserialize(buffer)
        print(">>> Restored property map state from memory buffer")
        print_device_info("After deserializing property map state", grabber)

        return 0

    except ic4.IC4Exception as ex:
        print("An exception occurred:")
        print(ex)
        return -10


if __name__ == "__main__":
    with ic4.Library.init_context():
        sys.exit(example_device_settings_serialization())
