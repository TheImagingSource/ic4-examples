import imagingcontrol4 as ic4

# These example program demonstrates how to set up multicast image transmission using GigE Vision cameras
# - multicast-master is started first opens the camera and configures it for multicast operation.
#   It owns the camera and is the only program that can configure camera parameters.
#   The master can be operated both with a data stream activated or in headless mode (without receiving image data)
# - multicast-monitor is started second and subscribes to the multicast group to (also) receive the camera's image data.
#
# This file contains the multicast-monitor program.
#
# Connecting to a device in monitor mode takes these steps:
# - Set the SpawnMonitorDevices flag on the GenTL representation of the selected device's network interface
#   This will add an additional entry to the list of enumerated devices with a "-Monitor" suffix on the model name and serial number.
# - Open the "-Monitor" device in a Grabber as usual
# - Use Grabber::streamStart just with a normal camera
#
# There are a few limitations when using a monitor device's property map:
# - The device cannot be controlled
# - There are only a few properties providing information about the stream and image format.
# - Controls for image transformation performed by the driver are availble.


# Multicast only works with GigE Vision devices
def enum_gige_devices():
    result = []

    for dev in ic4.DeviceEnum.devices():
        # Skip non-GigE devices
        if dev.interface.transport_layer_type != ic4.TransportLayerType.GIGEVISION:
            continue

        # Skip "-Monitor" entries and only show the regular device entry
        serial = dev.serial
        if serial.endswith("-Monitor"):
            continue

        result.append(dev)

    return result


class PrintFramesListener(ic4.QueueSinkListener):
    """Define a simple queue sink that prints frame numbers of received images."""

    def sink_connected(
        self,
        sink: ic4.QueueSink,
        image_type: ic4.ImageType,
        min_buffers_required: int,
    ) -> bool:
        return True

    def sink_disconnected(self, sink: ic4.QueueSink):
        pass

    def frames_queued(self, sink: ic4.QueueSink):
        buffer = sink.pop_output_buffer()

        print(
            f"Received frame {buffer.meta_data.device_frame_number}"
        )


def example_multicast_monitor():
    print("Started in monitor mode.")

    device_list = enum_gige_devices()

    # Let the user select a device
    for i, dev in enumerate(device_list):
        print(
            f"[{i}] {dev.model_name} ({dev.serial}) "
            f"[{dev.interface.display_name}]"
        )

    print(
        f"Select device [0..{len(device_list) - 1}]: ",
        end=""
    )
    selected_index = int(input())

    dev = device_list[selected_index]

    try:
        # Activate generation monitor device entries on the interface of the selected device
        itf = dev.interface
        itf_map = itf.property_map

        itf_map.set_value("SpawnMonitorDevices", True)

        try:
            # The serial number of the monitor device instance has "-Monitor" appended to it
            monitor_serial = dev.serial + "-Monitor"

            # Open the monitor device in a new Grabber
            grabber = ic4.Grabber()
            grabber.device_open(monitor_serial)

            print("Starting monitor stream. Press ENTER to quit.")

            # Start a stream, printing information about received images
            listener = PrintFramesListener()
            sink = ic4.QueueSink(listener)

            grabber.stream_setup(sink)

            # Wait for ENTER key
            input()

        finally:
            # Disable monitor device entries again
            itf_map.set_value("SpawnMonitorDevices", False)

    except Exception as ex:
        print("An exception occurred:")
        print(ex)
        return -10

    return 0


if __name__ == "__main__":
    # Initialize the library with sensible defaults:
    # - Throw exceptions on errors
    # - Log errors and warnings from API calls
    # - Log to stdout and Windows debug log
    ic4.Library.init(
        api_log_level=ic4.LogLevel.WARNING,
        log_targets=ic4.LogTarget.STDOUT,
    )

    try:
        example_multicast_monitor()
    finally:
        ic4.Library.exit()
