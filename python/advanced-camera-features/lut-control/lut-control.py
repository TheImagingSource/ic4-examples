import imagingcontrol4 as ic4

import math
import time


#
# This example program demonstrates how to use the cameras LUT (lookup table) feature to modify the tone curve of the captured images.
#

# Helper function to let the grabber stream for a bit to let auto functions reach a useful state
def calibrate_brightness(grabber: ic4.Grabber):
    sink = ic4.SnapSink()
    grabber.stream_setup(sink)

    time.sleep(2)

    grabber.stream_stop()


# Helper function to setup a datastream, and save a single image
def save_single_image(grabber: ic4.Grabber, path: str):
    # Setup a sink and data stream to snap an image
    sink = ic4.SnapSink()
    grabber.stream_setup(sink)

    buffer = sink.snap_single(1000)

    buffer.save_as_jpeg(path)

    grabber.stream_stop()

    print(f"Saved {path}")


# Helper function to let the user select a camera
def select_device(device_list):
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")

    if not device_list:
        return None

    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())

    if selected_index < 0 or selected_index >= len(device_list):
        return None

    return device_list[selected_index]


def example_lut():
    try:
        # Let the user select a camera
        device_list = ic4.DeviceEnum.devices()
        device = select_device(device_list)

        if device is None:
            return

        # Open the device in a new Grabber instance
        grabber = ic4.Grabber(device)

        # Access the device's property map
        map = grabber.device_property_map

        # Make sure the camera supports the LUT feature
        try:
            map.find(ic4.PropId.LUT_ENABLE)
        except ic4.IC4Exception:
            print(
                "Failed to find LUTEnable property. "
                "The device likely does not have LUT support."
            )
            return

        # To get consistent behavior, first reset all camera properties to default by loading the 'Default' user set
        map.set_value(ic4.PropId.USER_SET_SELECTOR, "Default")
        map.execute_command(ic4.PropId.USER_SET_LOAD)

        # Disable the camera's color preset if available, since LUT overrides gamma settings anyway
        # Linear settings provide a better result in this demo
        if map.try_set_value(ic4.PropId.USER_SET_SELECTOR, "ColorLinear"):
            map.execute_command(ic4.PropId.USER_SET_LOAD)

        # Let the device run with auto functions enabled for a few seconds to let them settle on useful values
        calibrate_brightness(grabber)

        # First, we need to select which type of lookup table the camera shall use.
        # For this example, we are going to use and modify the 'Luminance' lookup table only.
        # Color cameras also have the option to use one lookup table per color channel ('Red', 'Green', 'Blue').
        # Please note that using per-channel and luminance lookup tables can be mutually exclusive.
        # Some cameras might not even have a LUT_SELECTOR -- in this case the error is ignored.
        map.try_set_value(ic4.PropId.LUT_SELECTOR, "Luminance")

        # The lookup table is accessed by first writing the index of the table into LUTIndex,
        # and then writing the table entry using LUTValue.
        lut_index = map.find_integer(ic4.PropId.LUT_INDEX)
        lut_value = map.find_integer(ic4.PropId.LUT_VALUE)

        # Gather information about the ranges of both properties
        index_max = lut_index.maximum
        value_max = lut_value.maximum

        # Minimum for both index and value is always 0
        assert lut_index.minimum == 0
        assert lut_value.minimum == 0

        # Please note that both index and value do not necessarily have to align with the camera's current pixel format's bit depth.
        # The camera internally interpolates between values when required.
        print(f"Luminance LUT index has range 0..{index_max}")
        print(f"Luminance LUT value has range 0..{value_max}")

        # First, save a reference image with LUT disabled
        map.set_value(ic4.PropId.LUT_ENABLE, False)
        save_single_image(grabber, "reference.jpeg")


        # Prepare a LUT that looks like it reduces the bit depth of the image to only 4 possible brightness levels
        LEVELS = 4
        for i in range(index_max + 1):
            lvl = i // (index_max // LEVELS)
            val = value_max // (LEVELS - 1) * lvl

            lut_index.value = i
            lut_value.value = min(val, value_max)

        # Enable the LUT and save an image
        map.set_value(ic4.PropId.LUT_ENABLE, True)
        save_single_image(grabber, "4levels.jpeg")


        # Prepare an inverted LUT that creates a 'negative' effect
        for i in range(index_max + 1):
            val = value_max - (i * value_max // index_max)

            lut_index.value = i
            lut_value.value = min(val, value_max)

        # Save an image with the new LUT
        save_single_image(grabber, "inverted.jpeg")


        # Prepare a S-curve LUT that enhances contrast
        K = 10

        def sigmoid(z):
            return 1.0 / (1.0 + math.exp(-z))

        lo = sigmoid(-K / 2.0)
        hi = sigmoid(K / 2.0)

        for i in range(index_max + 1):
            # Normalize the LUT index to the range 0..1
            t = i / index_max

            val = (sigmoid(K * (t - 0.5)) - lo) / (hi - lo)

            lut_index.value = i
            lut_value.value = int(val * value_max)

        # Save an image with the new LUT
        save_single_image(grabber, "high-contrast.jpeg")

        # Disable LUT to not interfere with future camera usage
        map.set_value(ic4.PropId.LUT_ENABLE, False)

    except Exception as ex:
        print("An exception occurred:")
        print(str(ex))


if __name__ == "__main__":
    # Initialize the library with sensible defaults:
    # - Throw exceptions on errors
    # - Log errors and warnings from API calls
    # - Log to stdout and Windows debug log
    ic4.Library.init(
        api_log_level=ic4.LogLevel.WARNING,
        internal_log_level=ic4.LogLevel.OFF,
        log_targets=ic4.LogTarget.STDOUT | ic4.LogTarget.WINDEBUG,
    )

    try:
        example_lut()
    finally:
        ic4.Library.exit()
