import imagingcontrol4 as ic4
import time

# This example program shows how to utilize the device-lost event on the Grabber object to
# receive a notification in case the connection to the currently opened video capture device#
# is interrupted.
# It then re-enumerates the connected devices until the previously opened camera is found
# and opens it again.

# The unique name of the device we are trying to reopen later
g_device_unique_name = ""
# Flag to indicate that the search should be aborted
g_abort_reconnect = False


def handle_device_lost(grabber: ic4.Grabber):
    print("Device lost!")
    print("Waiting for device to be reconnected...")

    # We need to formally close the lost device before trying to open it again
    grabber.device_close()

    while not g_abort_reconnect:
        devices = ic4.DeviceEnum.devices()

        # Check whether one of them matches the unique name of the device we are looking for
        dev = next((d for d in devices if d.unique_name == g_device_unique_name), None)
        if dev is not None:
            # If it was found, try to open it
            try:
                grabber.device_open(dev)
                print("Device re-opened successfully")
            except ic4.IC4Exception as e:
                print(f"Failed to re-open device: {e.message}")
            return

        time.sleep(0.1)



def example_device_lost():
    # Let the user select a device
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    # Create Grabber object and open device
    grabber = ic4.Grabber(dev_info)

    # Register a function to be called when the opened device is disconnected.
    # The registeration function returns a token that can later be used to unregister the callback.
    token = grabber.event_add_device_lost(handle_device_lost)

    # Remember device unique name to reopen later
    global g_device_unique_name
    g_device_unique_name = dev_info.unique_name

    print(f"Opened device {dev_info.model_name} ({dev_info.serial})")
    print("Disconnect device to produce device-lost event")
    print("Press ENTER to exit program")
    input()

    # Let the device-lost handler know to stop looking
    global g_abort_reconnect
    g_abort_reconnect = True

    # Technically, this is not necessary, because the grabber object is deleted when the function is exited
    # But for demonstration purposes, the event handler is removed:
    grabber.event_remove_device_lost(token)


if __name__ == "__main__":
    ic4.Library.init(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR)

    try:
        example_device_lost()
    finally:
        ic4.Library.exit()
