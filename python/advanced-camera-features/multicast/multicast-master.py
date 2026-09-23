import re

import imagingcontrol4 as ic4


# These example program demonstrates how to set up multicast image transmission using GigE Vision cameras
# - multicast-master is started first opens the camera and configures it for multicast operation.
#   It owns the camera and is the only program that can configure camera parameters.
#   The master can be operated both with a data stream activated or in headless mode (without receiving image data)
# - multicast-monitor is started second and subscribes to the multicast group to (also) receive the camera's image data.
#
# master and monitor programs can be run on the same computer.
#
# This file contains the multicast-master program.
# There are two optional features that are currently disabled behind a if(false) block:
# - MTU auto-negotiation can be disabled, if the computer running the monitor program has a lower possible MTU than the
#   computer running the master program
# - The master can be configured to only control the camera but not actually receive data for itself.


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


# Make sure the camera firmware supports multicast operation
def check_firmware_version(grabber: ic4.Grabber):
    device_version = grabber.device_property_map.get_value_str(
        ic4.PropId.DEVICE_VERSION
    )

    match = re.search(r"Rev (\d+)", device_version)

    if match:
        revision = int(match.group(1))

        if revision < 4000:
            # GigE3 cameras need at least version 3173
            return revision >= 3173
        else:
            # GigE4 cameras need at least version 4380
            return revision >= 4380

    print(
        f"Failed to extract firmware revision from DeviceVersion "
        f"'{device_version}'"
    )

    return False


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


def example_multicast_master():
    print("Started in master mode.")

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

    dev_info = device_list[selected_index]

    try:
        # Open the monitor device in a new Grabber
        grabber = ic4.Grabber()
        grabber.device_open(dev_info)

        # Make sure the device supports multicast
        if not check_firmware_version(grabber):
            print(
                "Firmware version does not support multicast operation. "
                "Please contact support."
            )
            return -2

        # For multicast operation, the master has to manually configure stream options
        # on both the device's and driver's property maps:
        driver_map = grabber.driver_property_map
        device_map = grabber.device_property_map

        # Optional:
        # If the monitor(s) have limited network access, it might be necessary to
        # limit MTU to a conservative value
        if False:
            # Disable packet-size auto-negotiation and set it to fixed 1500
            driver_map.set_value("GevStreamNegotiatePacketSize", False)
            device_map.set_value("GevSCPSPacketSize", 1500)

        # Override destination address:port data with multicast address
        driver_map.set_value("GevStreamDestinationOverride", True)
        driver_map.set_value("GevStreamDestinationAddress", "225.0.0.1")
        driver_map.set_value("GevStreamDestinationPort", "50002")

        print("Starting master stream. Press ENTER to quit.")

        # Optional:
        # Enable headless mode, where master controls the camera,
        # but does not receive image data
        if False:
            # Write destination address:port into the camera
            driver_map.execute_command("GevStreamWriteDestinationSetup")

            # Mark Transport-Layer as locked, allowing access to AcquisitionStart
            device_map.set_value(ic4.PropId.TL_PARAMS_LOCKED, 1)

            # Let the camera start acquisition
            device_map.execute_command(ic4.PropId.ACQUISITION_START)
        else:
            # Start a stream, printing information about received images
            listener = PrintFramesListener()
            sink = ic4.QueueSink(listener)

            grabber.stream_setup(sink)

        # Wait for ENTER key
        input()

    except Exception as ex:
        print("An exception occurred:")
        print(ex)
        return -10

    return 0


if __name__ == "__main__":
    ic4.Library.init(
        api_log_level=ic4.LogLevel.WARNING,
        log_targets=ic4.LogTarget.STDOUT,
    )

    try:
        example_multicast_master()
    finally:
        ic4.Library.exit()