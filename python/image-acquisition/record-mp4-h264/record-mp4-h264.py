
import imagingcontrol4 as ic4


def example_record_mp4_h264():
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

    # Create a video writer for H264-compressed MP4 files
    video_writer = ic4.VideoWriter(ic4.VideoWriterType.MP4_H264)

    # Define a listener class to receive queue sink notifications
    # The listener will pass incoming frames to the video writer
    class Listener(ic4.QueueSinkListener):
        def __init__(self, video_writer: ic4.VideoWriter):
            self.video_writer = video_writer
            self.counter = 0
            self.do_write_frames = False

        def sink_connected(self, sink: ic4.QueueSink, image_type: ic4.ImageType, min_buffers_required: int) -> bool:
            # No need to configure anything, just accept the connection
            return True

        def frames_queued(self, sink: ic4.QueueSink):
            # Get the queued image buffer
		    # We have to remove buffers from the queue even if not recording; otherwise the device will not have
		    # buffers to write new video data into
            buffer = sink.pop_output_buffer()

            if self.do_write_frames:
                self.video_writer.add_frame(buffer)
                self.counter = self.counter + 1

        def enable_recording(self, enable: bool):
            if enable:
                self.counter = 0
            self.do_write_frames = enable

        def num_frames_written(self):
            return self.counter

    # Create an instance of the listener type defined above, specifying a partial file name
    listener = Listener(video_writer)

    # Create a QueueSink to capture all images arriving from the video capture device
    sink = ic4.QueueSink(listener)

    # Start the video stream into the sink
    grabber.stream_setup(sink)

    image_type = sink.output_image_type
    frame_rate = map[ic4.PropId.ACQUISITION_FRAME_RATE].value

    print("Stream started.")
    print(f"ImageType: {image_type}")
    print(f"AcquisitionFrameRate {frame_rate}")
    print()

    for i in range(3):
        input("Press ENTER to begin recording a video file")

        file_name = f"video{i}.mp4"

        # Begin writing a video file with a name, image type and playback rate
        video_writer.begin_file(file_name, image_type, frame_rate)

        # Instruct our QueueSinkListener to write frames into the video writer
        listener.enable_recording(True)

        input("Recording started. Press ENTER to stop")

        # Stop writing frames into the video writer
        listener.enable_recording(False)

        # Finalize the currently opened video file
        video_writer.finish_file()

        print(f"Saved video file {file_name}.")
        print(f"Wrote {listener.num_frames_written()} frames.")
        print()


    # We have to call streamStop before exiting the function, to make sure the listener object is not destroyed before the stream is stopped
    grabber.stream_stop()
    grabber.device_close()

if __name__ == "__main__":
    with ic4.Library.init_context(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR):

        example_record_mp4_h264()