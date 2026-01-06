import imagingcontrol4 as ic4
import cv2

from PySide6.QtWidgets import QApplication


class ProcessAndDisplayListener(ic4.QueueSinkListener):
    # Listener to demonstrate in-place-processing and displaying received images

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
        # which is guaranteed because the view is not passed out of this function.
        buffer_wrap = buffer.numpy_wrap()

        # Blur the buffer in place using a rather large kernel
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

        # Please note:
        # Writing to buffer_wrap is unsafe after this point,
        # because the underlying buffer is shared with the display.
        # Further updates might only be partially visible.


class ProcessAndDisplayNewBufferListener(ic4.QueueSinkListener):
    # Listener to demonstrate processing and displaying images that are transformed into a new image buffer

    def __init__(self, d: ic4.Display):
        self.display = d

        # Create a buffer pool to take image buffers from
        # Since one of the buffers is always displayed and the other is used in processing,
        # let the BufferPool cache two of them to avoid repeated allocations
        self.buffer_pool = ic4.BufferPool(cache_buffers_max=2)

    def sink_connected(self, sink: ic4.QueueSink, image_type: ic4.ImageType, min_buffers_required: int) -> bool:
        # Just accept whatever is passed
        return True

    def frames_queued(self, sink: ic4.QueueSink):
        # Get the new buffer from the sink
        src_buffer = sink.pop_output_buffer()

        # Create a numpy view onto the source buffer
        # This view is only valid while the buffer itself exists,
        # which is guaranteed by them both not being passed out of this function
        src_buffer_wrap = src_buffer.numpy_wrap()

        # Get a buffer from the buffer pool with width and height swapped so that we can rotate into it
        src_type = src_buffer.image_type
        dest_type = src_buffer.image_type.with_size(new_width=src_type.height, new_height=src_type.width)
        dest_buffer = self.buffer_pool.get_buffer(dest_type)

        # Create a numpy view onto the destination buffer
        # This view is only valid while the buffer itself exists,
        # which is guaranteed because the view is not passed out of this function.
        dest_buffer_wrap = dest_buffer.numpy_wrap()

        # Rotate by 90 degrees from the source into destination buffer
        cv2.rotate(src_buffer_wrap, cv2.ROTATE_90_CLOCKWISE, dest_buffer_wrap)

        # Write some text into the modified buffer
        cv2.putText(
            dest_buffer_wrap,
            "This image is rotated using OpenCV",
            (100, 100),
            fontFace=cv2.FONT_HERSHEY_SIMPLEX,
            fontScale=1,
            color=(255, 0, 0),
            thickness=2,
        )

        # Display the rotated buffer
        self.display.display_buffer(dest_buffer)

        # Please note:
        # Writing to dest_buffer_wrap is unsafe after this point,
        # because the underlying buffer is shared with the display.
        # Further updates might only be partially visible.


def example_imagebuffer_numpy_opencv_live():

    # Create PySide6 application object
    app = QApplication()
    app.setApplicationDisplayName("IC4 ImageBuffer to NumPy/OpenCV with Live Display")
    app.setStyle("fusion")

    # Let the user select a video capture device
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    # Let the user choose whether we want to rotate
    print()
    print("Available image processing operations:")
    print("[0] Blur buffer in place")
    print("[1] Rotate into new buffer")
    print("Select operation [0..1] ", end="")
    operation = int(input())

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
    listener = ProcessAndDisplayListener(display) if operation == 0 else ProcessAndDisplayNewBufferListener(display)

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
