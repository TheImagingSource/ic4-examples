import time
import ic4


def example_connect_chunkdata():
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    grabber = ic4.Grabber()
    grabber.device_open(dev_info)

    prop_map = grabber.device_property_map

    # Create a QueueSink to get access to each received image buffer
    # The image buffers are processed in an instance of PrintChunkExposureTimeListener, defined below
    sink = ic4.QueueSink(PrintChunkExposureTimeListener(prop_map))

    # Make sure the selected device actually supports ChunkExposureTime
    try:
        _ = prop_map.find_float(ic4.PropId.CHUNK_EXPOSURE_TIME)
    except:
        print("ChunkExposureTime is not supported by this device")
        return

    # Reset all camera settings to default
    # This reverses settings like TriggerMode=On, which would prevent this demo from running as expected
    prop_map.set_value(ic4.PropId.USER_SET_SELECTOR, "Default")
    prop_map.execute_command(ic4.PropId.USER_SET_LOAD)

    # Chunkdata-related properties have to be configured before streamSetup,
    # because enabling them increases the payload size
    prop_map.set_value(ic4.PropId.CHUNK_MODE_ACTIVE, True)
    prop_map.set_value(ic4.PropId.CHUNK_SELECTOR, "ExposureTime")
    try:
        prop_map.set_value(ic4.PropId.CHUNK_ENABLE, True)
    except:
        # Ignore possible error, since the device might have ChunkEnable[ExposureTime] locked to true
        pass

    print("Configure resolution 640x480")
    prop_map.set_value(ic4.PropId.WIDTH, 640)
    prop_map.set_value(ic4.PropId.HEIGHT, 480)

    print("Set AcquisitionFrameRate to 5")
    prop_map.set_value(ic4.PropId.ACQUISITION_FRAME_RATE, 5)

    print("Set ExposureAuto to Off")
    prop_map.set_value(ic4.PropId.EXPOSURE_AUTO, "Off")

    print("Set ExposureTime to 2 ms")
    prop_map.set_value(ic4.PropId.EXPOSURE_TIME, 2000)

    grabber.stream_setup(sink)

    print("Stream for 3 seconds")
    time.sleep(3)

    print("Set ExposureTime to 8 ms")
    prop_map.set_value(ic4.PropId.EXPOSURE_TIME, 8000)

    print("Continue streaming for 3 seconds")
    time.sleep(3)

    print("Set ExposureTime to 32 ms")
    prop_map.set_value(ic4.PropId.EXPOSURE_TIME, 32000)

    print("Continue streaming for 3 seconds")
    time.sleep(3)


class PrintChunkExposureTimeListener(ic4.QueueSinkListener):
    prop_map: ic4.PropertyMap

    def __init__(self, prop_map: ic4.PropertyMap):
        self.prop_map = prop_map

    def sink_connected(self, sink: ic4.QueueSink, image_type: ic4.ImageType, min_buffers_required: int) -> bool:
        return True

    def frames_queued(self, sink: ic4.QueueSink):
        # Do not throw from callback function, capture and log errors instead
        try:
            buffer = sink.pop_output_buffer()

            # Use the image buffer as backend for read operations on chunk properties
            self.prop_map.connect_chunkdata(buffer)

            # Read chunk property from image buffer
            val = self.prop_map.get_value_float(ic4.PropId.CHUNK_EXPOSURE_TIME)
            print(f"ChunkExposureTime = {val}")

        except ic4.IC4Exception as ex:
            print(f"Error trying to request ChunkExposuretime: {ex.code} ({ex.message})")
        finally:
            # Disconnecting is not strictly necessary, but will release the buffer for reuse earlier
            self.prop_map.connect_chunkdata(None)

    def sink_disconnected(self, sink: ic4.QueueSink):
        pass


if __name__ == "__main__":
    ic4.Library.init(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR)

    try:
        example_connect_chunkdata()
    finally:
        ic4.Library.exit()
