import imagingcontrol4 as ic4
import cv2

from PySide6.QtWidgets import QApplication


class ProcessAndDisplayListener(ic4.QueueSinkListener):
    # Listener to demonstrate processing and displaying received images

    def __init__(self, d: ic4.Display):
        self.display = d

    def sink_connected(self, sink: ic4.QueueSink, image_type: ic4.ImageType, min_buffers_required: int) -> bool:
        # Just accept whatever is passed
        return True

    def frames_queued(self, sink: ic4.QueueSink):
        # Get the new buffer from the sink
        buffer = sink.pop_output_buffer()

        # Create a numpy view onto the buffer
        # This view is only valid while the buffer itself exists,
        # which is guaranteed by them both not being passed out of this function
        buffer_wrap = buffer.numpy_wrap()

        # Blur the buffer in-place using a rather large kernel
        cv2.blur(buffer_wrap, (31, 31), buffer_wrap)

        # Write some text so that the user doesn't hopelessly try to focus the lens
        cv2.putText(
            buffer_wrap,
            "This image is blurred using OpenCV",
            (100, 100),
            fontFace=cv2.FONT_HERSHEY_SIMPLEX,
            fontScale=1,
            color=(255, 0, 0),
            thickness=2,
        )

        # Send the modified buffer to the display
        self.display.display_buffer(buffer)


def example_imagebuffer_numpy_opencv_live():

    # Create PySide6 application object
    app = QApplication()
    app.setApplicationDisplayName("IC4 ImageBuffer to NumPy/OpenCV with Live Display")
    app.setStyle("fusion")

    # Let the select a video capture device
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    # Open the selected device in a new Grabber
    grabber = ic4.Grabber()
    grabber.device_open(dev_info)

    # Create the main window. In this simple application, we use ic4.pyside6.DisplayWindow
    window = ic4.pyside6.DisplayWindow()
    window.setMinimumSize(640, 480)

    # Get ic4 Display object from window
    display = window.as_display()
    display.set_render_position(ic4.DisplayRenderPosition.STRETCH_CENTER)

    # Create a listener to process and display the received images
    listener = ProcessAndDisplayListener(display)

    # Create a sink that passes the images to the listener
    # This sink fixes the pixel format to BGR8, so that the listener receives 3-channel color images
    # max_output_buffers is set to 1 so that we always get the latest available image
    sink = ic4.QueueSink(listener, [ic4.PixelFormat.BGR8], max_output_buffers=1)
    grabber.stream_setup(sink)

    # Wait for the window to be closed
    window.show()
    app.exec()

    grabber.stream_stop()


if __name__ == "__main__":
    ic4.Library.init(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR)

    try:
        example_imagebuffer_numpy_opencv_live()
    finally:
        ic4.Library.exit()
