

import imagingcontrol4 as ic4


def example_save_jpeg_file():
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    # Open the selected device in a new Grabber
    grabber = ic4.Grabber(dev_info)

    # Create a snap sink for manual buffer capture
    sink = ic4.SnapSink()

    # Start data stream from device to sink
    grabber.stream_setup(sink)

    for i in range(10):
        input("Press ENTER to snap and save a jpeg image")

        # Grab the next image buffer
        buffer = sink.snap_single(1000)

        # Save buffer contents in a jpeg file
        filename = f"image_{i}.jpeg"
        buffer.save_as_jpeg(filename, quality_pct=90)

        print(f"Saved image file {filename}")
        print()

    # Only for completeness. Technically this is not necessary here, since the grabber is destroyed at the end of the function.
    grabber.stream_stop()
    grabber.device_close()

if __name__ == "__main__":
    with ic4.Library.init_context(api_log_level=ic4.LogLevel.INFO, log_targets=ic4.LogTarget.STDERR):

        example_save_jpeg_file()
