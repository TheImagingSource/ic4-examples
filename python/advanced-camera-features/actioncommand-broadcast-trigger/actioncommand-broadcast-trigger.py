import imagingcontrol4 as ic4
import time


def example_actioncommand_broadcast_trigger():

    # Filter interfaces for GigEVision, since Action Command broadcasts are only supported by GigEVision devices/interfaces.
    all_interfaces = ic4.DeviceEnum.interfaces()
    gige_interfaces = [itf for itf in all_interfaces if itf.transport_layer_type == ic4.TransportLayerType.GIGEVISION]

    # Let the user select a network interface
    for i, itf in enumerate(gige_interfaces):
        print(f"[{i}] {itf.display_name}")
    print(f"Select device [0..{len(gige_interfaces) - 1}]: ", end="")
    selected_index = int(input())
    itf = gige_interfaces[selected_index]

    # Check whether there are any video capture devices on the selected network interface.  
    if not itf.devices:
        print("No devices found")
        return
    
    # Both camera configuration and Action Command broadcast packets contain 3 values to identify
    # which devices should respond to a broadcast packet.
    # The ActionDeviceKey value in the broadcast packet has to match the ActionDeviceKey setting of the camera.
    # The ActionGroupKey value in the broadcast packet has to match the ActionGroupKey setting of the camera.
    # The ActionGroupMask value in the broadcast packet has to have all bits set that are present in the ActionGroupMask setting of the camera.
    # Using a clever combination of device key, group key and group masks, it is possible to create
    # arbitrary groups of cameras that respond to a broadcast packet.

    # For testing, we use the same device key, group key, and group mask for both the broadcast packet and
    # all cameras.
    DEVICE_KEY = 0x00000123
    GROUP_KEY = 0x00000456
    GROUP_MASK = 0x00000001

    # Configure the broadcast packet that will be sent by the driver.
    itf.property_map.set_value(ic4.PropId.ACTION_DEVICE_KEY, DEVICE_KEY)
    itf.property_map.set_value(ic4.PropId.ACTION_GROUP_KEY, GROUP_KEY)
    itf.property_map.set_value(ic4.PropId.ACTION_GROUP_MASK, GROUP_MASK)
    # Disable ActionScheduledTimeEnable, we want the actions to be executed immediately.
    itf.property_map.set_value("ActionScheduledTimeEnable", False)

    devices = itf.devices
    grabbers = []

    for (device_index, dev) in enumerate(devices):
        grabber = ic4.Grabber()
        grabber.device_open(dev)
        map = grabber.device_property_map

        # Configure device for maximum resolution, maximum frame rate.
        map.set_value(ic4.PropId.WIDTH, map[ic4.PropId.WIDTH].maximum)
        map.set_value(ic4.PropId.HEIGHT, map[ic4.PropId.HEIGHT].maximum)
        map.set_value(ic4.PropId.ACQUISITION_FRAME_RATE, map[ic4.PropId.ACQUISITION_FRAME_RATE].maximum)

        # Configure Action0 to the same device key, group key and group mask as the broadcast packet prepared above.
        map.set_value(ic4.PropId.ACTION_SELECTOR, 0)
        map.set_value(ic4.PropId.ACTION_DEVICE_KEY, DEVICE_KEY)
        map.set_value(ic4.PropId.ACTION_GROUP_KEY, GROUP_KEY)
        map.set_value(ic4.PropId.ACTION_GROUP_MASK, GROUP_MASK)

        # Enable trigger mode, with trigger source set to Action0.
        map.set_value(ic4.PropId.TRIGGER_MODE, "On")
        map.set_value(ic4.PropId.TRIGGER_SOURCE, "Action0")

        # Create a sink with a listener to receive frame notifications
        class Listener(ic4.QueueSinkListener):
            def __init__(self, device_index: int):
                self.device_index = device_index

            def sink_connected(self, sink: ic4.QueueSink, image_type: ic4.ImageType, min_buffers_required: int) -> bool:
                return True
            def sink_disconnected(self, sink: ic4.QueueSink):
                pass

            def frames_queued(self, sink: ic4.QueueSink):
                buffer = sink.pop_output_buffer()
                print(f"Image from device {self.device_index}, frameId = {buffer.meta_data.device_frame_number}, timestamp = {buffer.meta_data.device_timestamp_ns}")

        sink = ic4.QueueSink(Listener(device_index))

        # Set up stream for this camera
        grabber.stream_setup(sink)

        # Keep grabber alive
        grabbers.append(grabber)

    # Capture 10 images from all cameras
    for _ in range(10):
        # Instruct the network interface to send one broadcast Action Command.
        itf.property_map.execute_command("ActionCommand")

        time.sleep(0.5)

if __name__ == "__main__":
    with ic4.Library.init_context(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR):
        example_actioncommand_broadcast_trigger()

