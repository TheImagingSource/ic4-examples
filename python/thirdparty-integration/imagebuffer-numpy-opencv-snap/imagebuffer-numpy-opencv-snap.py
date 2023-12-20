import imagingcontrol4 as ic4
import cv2


def example_imagebuffer_numpy_opencv_snap():
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
        example_imagebuffer_numpy_opencv_snap()
    finally:
        ic4.Library.exit()
