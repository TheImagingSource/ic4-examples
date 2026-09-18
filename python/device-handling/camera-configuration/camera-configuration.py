import imagingcontrol4 as ic4

# This program opens a video capture device and performs simple camera configuration,
# demonstrating use of functions on ic4.PropertyMap.
#
# The example program
# - Configures maximum resolution
# - Sets maximum frame rate
# - Disables trigger mode
# - Disables auto gain, and sets gain to minimum
# - Enables auto exposure
# - Enables auto white balance
#
# Camera properties are set using functions on PropertyMap:
# - set_value sets a property value
# - set_value_maximum sets an int or float property to its maximum value
# - set_value_minimum sets an int or float property to its minimum value
#
# When setting properties, the property to be modified is identified using the
# constants in ic4.PropId, e.g. ic4.PropId.EXPOSURE_TIME.
#
# Please note: the type of the value passed to set_value does not necessarily have
# to match the property type. For example, set_value will automatically convert
# boolean True to select the 'On' value on the enumeration property TriggerMode.
#
# Camera property values can be read as well:
# - get_value_int
# - get_value_str
# - get_value_float
# - get_value_bool
#
# Please note: The type-part of the function name (int, str, float, bool)
# refers to the return value of the function, not the property type.
# The property value is converted automatically.
# For example, it is possible to call get_value_str for an integer property
# and get a string representation of the integer value.
# See the documentation for the specific function overloads for details.


def camera_configuration():
    # Let the user select a camera
    device_list = ic4.DeviceEnum.devices()

    if len(device_list) == 0:
        print("No devices found.")
        return -1

    print("Available cameras:")

    for i, device_info in enumerate(device_list):
        print(f"[{i}] {device_info.model_name} " f"(Serial: {device_info.serial})")

    try:
        selection = int(input("Select a camera: "))
    except ValueError:
        print("Invalid selection.")
        return -1

    if selection < 0 or selection >= len(device_list):
        print("Invalid selection.")
        return -1

    try:
        # Create a grabber with the selected device opened
        grabber = ic4.Grabber(device_list[selection])

        # Camera settings are accessed via the device's property map:
        prop_map = grabber.device_property_map

        # First, configure the camera:

        # Configure the camera for maximum resolution and frame rate
        prop_map.set_value_maximum(ic4.PropId.WIDTH)
        prop_map.set_value_maximum(ic4.PropId.HEIGHT)
        prop_map.set_value_maximum(ic4.PropId.ACQUISITION_FRAME_RATE)

        # Disable trigger mode
        # Use manual error handling instead of exceptions, since the camera
        # might not support trigger mode.
        if not prop_map.try_set_value(
            ic4.PropId.TRIGGER_MODE,
            "Off",
        ):
            print("Failed to set trigger mode.")

        # Disable gain auto
        # Technically, GainAuto is usually an enumeration with entries
        # 'Continuous' and 'Off', but PropertyMap.set_value() tries to
        # convert between property values automatically.
        # See the documentation for details.
        prop_map.set_value(ic4.PropId.GAIN_AUTO, False)

        # Set gain to its minimum value
        prop_map.set_value_minimum(ic4.PropId.GAIN)

        # Enable auto exposure
        prop_map.set_value(
            ic4.PropId.EXPOSURE_AUTO,
            "Continuous",
        )

        # Enable auto white balance
        prop_map.set_value(
            ic4.PropId.BALANCE_WHITE_AUTO,
            "Continuous",
        )

        # Then, query and print modified settings:

        # Query resolution
        width = prop_map.get_value_int(ic4.PropId.WIDTH)

        # Technically, Height is usually an integer.
        # PropertyMap.get_value_str() automatically interprets that value
        # as a string.
        height = prop_map.get_value_str(ic4.PropId.HEIGHT)

        # Query frame rate
        fps = prop_map.get_value_float(ic4.PropId.ACQUISITION_FRAME_RATE)

        # Query trigger mode
        # Technically, TriggerMode is usually an enumeration with values
        # 'On' and 'Off'.
        # PropertyMap.get_value_bool() automatically interprets that value
        # as a boolean.
        trigger_mode = prop_map.get_value_bool(ic4.PropId.TRIGGER_MODE)

        # Query gain auto
        gain_auto = prop_map.get_value_str(ic4.PropId.GAIN_AUTO)

        # Query gain value
        gain_value = prop_map.get_value_float(ic4.PropId.GAIN)

        # Query exposure auto
        exposure_auto = prop_map.get_value_str(ic4.PropId.EXPOSURE_AUTO)

        # Query white balance auto
        white_balance_auto = prop_map.get_value_str(ic4.PropId.BALANCE_WHITE_AUTO)

        print("Camera configured:")
        print(f"- Width: {width}")
        print(f"- Height: {height}")
        print(f"- AcquisitionFrameRate: {fps}")
        print(f"- TriggerMode: {trigger_mode}")
        print(f"- GainAuto: {gain_auto}")
        print(f"- Gain: {gain_value}")
        print(f"- ExposureAuto: {exposure_auto}")
        print(f"- BalanceWhiteAuto: {white_balance_auto}")

        return 0

    except ic4.IC4Exception as ex:
        print("An exception occurred:")
        print(ex.message)
        return -10


if __name__ == "__main__":
    # Initialize the library with sensible defaults:
    # - Log warnings from API calls
    # - Log to stdout
    #
    # Python exceptions are raised automatically by the IC4 Python API.
    with ic4.Library.init_context(
        api_log_level=ic4.LogLevel.WARNING,
        internal_log_level=ic4.LogLevel.OFF,
        log_targets=ic4.LogTarget.STDOUT,
    ):
        exit_code = camera_configuration()

    raise SystemExit(exit_code)
