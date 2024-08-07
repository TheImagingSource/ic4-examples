import imagingcontrol4 as ic4


def handle_device_lost(grabber: ic4.Grabber):
    print("Device lost!")


def example_device_lost():
    # Let the user select a device
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    # Create Grabber object and open device
    grabber = ic4.Grabber()
    grabber.device_open(dev_info)

    # Register a function to be called when the opened device is disconnected.
    # The registeration function returns a token that can later be used to unregister the callback.
    token = grabber.event_add_device_lost(handle_device_lost)

    print(f"Opened device {dev_info.model_name} ({dev_info.serial})")
    print("Disconnect device to produce device-lost event")
    print("Press ENTER to exit program")
    input()

    # Technically, this is not necessary, because the grabber object is deleted when the function is exited
    # But for demonstration purposes, the event handler is removed:
    grabber.event_remove_device_lost(token)


if __name__ == "__main__":
    ic4.Library.init(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR)

    try:
        example_device_lost()
    finally:
        ic4.Library.exit()
