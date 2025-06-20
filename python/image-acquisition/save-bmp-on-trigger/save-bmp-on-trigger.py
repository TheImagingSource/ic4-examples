
import imagingcontrol4 as ic4


def example_save_bmp_on_trigger():
    # Let the user select one of the connected cameras
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    # Open the selected device in a new Grabber
    grabber = ic4.Grabber(dev_info)
    map = grabber.device_property_map

    # Reset all device settings to default
    # Not all devices support this, so ignore possible errors
    map.try_set_value(ic4.PropId.USER_SET_SELECTOR, "Default")
    map.try_set_value(ic4.PropId.USER_SET_LOAD, 1)

    # Select FrameStart trigger (for cameras that support this)
    map.try_set_value(ic4.PropId.TRIGGER_SELECTOR, "FrameStart")

    # Enable trigger mode
    map.set_value(ic4.PropId.TRIGGER_MODE, "On")

    # Define a listener class to receive queue sink notifications
    class Listener(ic4.QueueSinkListener):
        def __init__(self, path_base: str):
            self.path_base = path_base
            self.counter = 0

        def sink_connected(self, sink: ic4.QueueSink, image_type: ic4.ImageType, min_buffers_required: int) -> bool:
            # No need to configure anything, just accept the connection
            return True
        
        def frames_queued(self, sink: ic4.QueueSink):
            # Get the queued image buffer
            buffer = sink.pop_output_buffer()

            # Save the image buffer's contents in a BMP file
            file_name = f"{self.path_base}{self.counter}.bmp"
            buffer.save_as_bmp(file_name)

            print(f"Saved image {file_name}")

            self.counter = self.counter + 1

    # Create an instance of the listener type defined above, specifying a partial file name
    path_base = "./test_image"
    listener = Listener(path_base)

    # Create a QueueSink to capture all images arriving from the video capture device
    sink = ic4.QueueSink(listener)

    # Start the video stream into the sink
    grabber.stream_setup(sink)

    print("Stream started.")
    print("Waiting for triggers")
    print(f"All images will be saved as {path_base}*.bmp")
    print()
    print("Input hardware triggers, or press ENTER to issue a software trigger")
    print("Press q + ENTER to quit")

    while input() != 'q':
        # Execute software trigger
        map.execute_command(ic4.PropId.TRIGGER_SOFTWARE)

    # We have to call streamStop before exiting the function, to make sure the listener object is not destroyed before the stream is stopped
    grabber.stream_stop()

    map.set_value(ic4.PropId.TRIGGER_MODE, "Off")

    grabber.device_close()

if __name__ == "__main__":
    with ic4.Library.init_context(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR):

        example_save_bmp_on_trigger()
