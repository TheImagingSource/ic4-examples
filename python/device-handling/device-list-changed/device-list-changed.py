import ic4


def handle_device_list_changed(device_enum: ic4.DeviceEnum):
    print("Device list changed!")

    print(f"Found {len(ic4.DeviceEnum.devices())} devices")

    print(ic4.DeviceEnum.devices())


def example_device_list_changed():
    enumerator = ic4.DeviceEnum()

    token = enumerator.event_add_device_list_changed(handle_device_list_changed)

    print("Waiting for DeviceListChanged event")
    print("Press Enter to exit")
    input()

    # Technically, this is not necessary, because the enumerator object is deleted when the function is exited
    # But for demonstration purposes, the event handler is removed:
    enumerator.event_remove_device_list_changed(token)


if __name__ == "__main__":
    ic4.Library.init(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR)

    try:
        example_device_list_changed()
    finally:
        ic4.Library.exit()
