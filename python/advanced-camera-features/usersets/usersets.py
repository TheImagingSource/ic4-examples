import sys
import time
import imagingcontrol4 as ic4

# This example program demonstrates the use of the UserSet features present in most modern cameras.
#
# UserSets provide a mechanism to store the camera configuration in the camera's non-volatile memory.
#
# Usually, a camera provides at least two 'slots' to save data into, named 'UserSet1' and 'UserSet2'.
#
# To save the camera configuration into one of the slots, first set UserSetSelector to the identifier of the slot to operate on.
# Then, execute the UserSetSave command to store the current camera configuration into that slot.
#
# To later restore the camera configuration, first set UserSetSelector to the identifier of the slot to read from.
# Then, execute the UserSetLoad command.
#
# There is a special 'Default' slot that contains the camera's default configuration that can be used to
# restore the device into a known-good state. The UserSetSave operation is not available for the 'Default' slot.
#
# The UserSetDefault feature can be used to specify a slot that is loaded automatically when the camera is powered on
# (or rebooted using DeviceReset).


def example_user_sets():
    try:
        # Let the user select a camera
        device_list = ic4.DeviceEnum.devices()

        if not device_list:
            print("No cameras found.")
            return -1

        for index, device in enumerate(device_list):
            print(f"{index}: {device.model_name} ({device.serial})")

        selection = input("Select camera: ")
        try:
            selected_index = int(selection)
            if selected_index < 0 or selected_index >= len(device_list):
                raise ValueError()
        except ValueError:
            print("Invalid camera selection.")
            return -1

        selected_device = device_list[selected_index]

        # Open the device in a new Grabber instance
        grabber = ic4.Grabber(selected_device.unique_name)

        # Access the device's property map
        map = grabber.device_property_map

        # Make sure the camera supports the UserSet features
        try:
            selector = map.find("UserSetSelector")
        except Exception:
            print("Failed to find UserSetSelector property. " "The device likely does not have UserSet support.")
            return -2

        if selector is None:
            print("Failed to find UserSetSelector property. " "The device likely does not have UserSet support.")
            return -2

        # Execute UserSetLoad[Default] to reset the camera to its default state
        map.set_value("UserSetSelector", "Default")
        map.execute_command("UserSetLoad")

        # Query some property info for demonstration purposes
        default_height = map["Height"].value
        min_height = map["Height"].minimum

        # Modify a property for demonstration purposes
        map.set_value("Height", min_height)

        # Verify Height was changed to min_height
        assert map.get_value_int("Height") == min_height

        # Execute UserSetSave[UserSet1] to store the camera configuration where Height==min_height
        map.set_value("UserSetSelector", "UserSet1")
        map.execute_command("UserSetSave")

        # Load default UserSet again
        map.set_value("UserSetSelector", "Default")
        map.execute_command("UserSetLoad")

        # Verify Height was restored to default_height
        assert map.get_value_int("Height") == default_height

        # Load UserSet1 again
        map.set_value("UserSetSelector", "UserSet1")
        map.execute_command("UserSetLoad")

        # Verify Height was restored to min_height
        assert map.get_value_int("Height") == min_height

        # Set UserSetDefault so that the camera always loads UserSet1 at startup
        # Please note: Applications (such as ic4-demoapp and IC Capture 4) can have
        # their own mechanism to restore camera state, which can hide the effect of UserSetDefault.
        map.set_value("UserSetDefault", "UserSet1")

        # Reboot the camera
        map.execute_command("DeviceReset")
        # DeviceReset is possibly an asynchronous operation
        time.sleep(2)

        # We have to call device_close before device_open
        unique_name = selected_device.unique_name
        grabber.device_close()

        # Try to re-discover the device after reset
        # Please note: We cannot use the previously-discovered device instance here.
        print("Waiting for the camera to re-appear after DeviceReset...")
        wait_timeout = time.monotonic() + 60

        while True:
            try:
                grabber.device_open(unique_name)
                break
            except Exception:
                if time.monotonic() > wait_timeout:
                    print("Failed to discover device after DeviceReset")
                    return -3

                time.sleep(1)

        # Access the property map for the re-opened device
        map = grabber.device_property_map

        # Verify Height was initialized to min_height
        assert map.get_value_int("Height") == min_height

        # Revert camera startup behavior back to default
        map.set_value("UserSetDefault", "Default")

        print("All tests passed")

        return 0

    except Exception as ex:
        print("An exception occurred:")
        print(str(ex))
        return -10


if __name__ == "__main__":
    with ic4.Library.init_context():
        sys.exit(example_user_sets())
