
import ic4
import cv2
import time


class DisplayCv2Listener(ic4.QueueSinkListener):

    def sink_connected(self, sink: ic4.QueueSink, image_type: ic4.ImageType, min_buffers_required: int) -> bool:
        return True

    def frames_queued(self, sink: ic4.QueueSink):
        # Do not throw from callback function, capture and log errors instead
        try:
            buffer = sink.pop_output_buffer()

            np = buffer.numpy_wrap()

            # cv2.blur(np, (75, 75), np)



        except ic4.IC4Exception as ex:
            print(f"Error trying to request ChunkExposuretime: {ex.code} ({ex.message})")
        finally:
            pass

    def sink_disconnected(self, sink: ic4.QueueSink):
        pass


def example_imagebuffer_numpy_opencv():
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    grabber = ic4.Grabber()
    grabber.device_open(dev_info)

    cv2.namedWindow("display")

    sink = ic4.SnapSink()
    grabber.stream_setup(sink)

    for i in range(5):
        print("Focus display window and press any key to continue...")
        cv2.waitKey(0)

        buffer = sink.snap_single(1000)
        np = buffer.numpy_wrap()
        print("Displaying captured image")
        cv2.imshow("display", np)

        print("Focus display window and press any key to continue...")
        cv2.waitKey(0)

        cv2.blur(np, (75, 75), np)
        print("Displaying blurred image")
        cv2.imshow("display", np)

    grabber.stream_stop()

if __name__ == "__main__":
    ic4.Library.init(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR)

    try:
        example_imagebuffer_numpy_opencv()
    finally:
        ic4.Library.exit()
