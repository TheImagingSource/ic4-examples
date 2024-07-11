from threading import Timer, Condition, Lock, Event
from typing import Optional
from time import time

import imagingcontrol4 as ic4


class RealWorld:
    # This is our "real world" simulation.

    current_frame_id: int = -1
    scene_setup_duration_ms: int

    lock: Lock
    cv: Condition
    scene_setup_completed: bool = True

    scene_setup_timer: Optional[Timer]

    def __init__(self, scene_setup_duration_ms: int):
        self.scene_setup_duration_ms = scene_setup_duration_ms
        self.lock = Lock()
        self.cv = Condition(self.lock)

    def begin_setup_scene(self, frame_id: int):
        """Requests the real world to setup the scene for frameID.

        When called with an increased frame ID, will create a task that can be waited for.

        Args:
            frame_id (int): Frame ID
        """
        self.cv.acquire()
        try:
            if frame_id > self.current_frame_id:
                self.current_frame_id = frame_id
                self.scene_setup_completed = False
                self.scene_setup_timer = Timer(
                    self.scene_setup_duration_ms / 1000.0, lambda: self.scene_setup_timer_done()
                )
                self.scene_setup_timer.start()
        finally:
            self.cv.release()

    def wait_setup_scene_completion(self) -> None:
        """Waits for the scene setup to be completed."""
        self.cv.acquire()
        try:
            self.cv.wait_for(lambda: self._is_scene_setup_completed())
        finally:
            self.cv.release()

    def _is_scene_setup_completed(self) -> bool:
        if self.scene_setup_completed:
            self.scene_setup_completed = False
            return True
        else:
            return False

    def reset(self) -> None:
        """Resets the world to its original state"""
        self.cv.acquire()
        try:
            self.current_frame_id = -1
            self.scene_setup_completed = True
            if self.scene_setup_timer is not None:
                self.scene_setup_timer.cancel()
        finally:
            self.cv.release()

    def scene_setup_timer_done(self):
        self.cv.acquire()
        try:
            self.scene_setup_completed = True
            self.cv.notify()
        finally:
            self.cv.release()


def example_event_exposure_end():
    device_list = ic4.DeviceEnum.devices()
    for i, dev in enumerate(device_list):
        print(f"[{i}] {dev.model_name} ({dev.serial}) [{dev.interface.display_name}]")
    print(f"Select device [0..{len(device_list) - 1}]: ", end="")
    selected_index = int(input())
    dev_info = device_list[selected_index]

    grabber = ic4.Grabber()
    grabber.device_open(dev_info)

    # GigEVision and USB3Vision devices can send asynchronous events to applications
    # This example shows how to receive events EventExposureEnd, which indicates that the camera has completed
    # the integration time and will start transmitting the image data.
    #
    # This event is useful when syncronizing real-world activities with camera operation, for example moving
    # something while the image is being transmitted.
    #
    # Events are configured and received through the device property map.
    # The following shows an excerpt from the device property map of a device supporting EventExposureEnd:
    #
    # - EventControl
    #   - EventSelector
    #   - EventNotification[EventSelector]
    #   - EventExposureEndData
    #     - EventExposureEnd
    #     - EventExposureEndTimestamp
    #     - EventExposureEndFrameID
    #
    # To receive notifications for a specific event, two steps have to be taken:
    #
    # First, the device has to be configured to send generate the specific event. To enable the EventExposureEnd event, set the
    # "EventSelector" enumeration property to "EventExposureEnd", and then set the "EventNotification" enumeration property to "On".
    #
    # Second, a property notification handler has to be registered for the property representing the event.
    # The EventExposureEnd is represented by the integer property "EventExposureEnd". This property has no function other
    # than being invalidated and thus having its notification raised when the device sends the event.
    #
    # Event parameters are grouped with the event property in a property category with "Data" appended to the event's name,
    # in our case "EventExposureEndData". The category contains the integer properties "EventExposureEndTimestamp"
    # and "EventExposureEndFrameID", which provide the time stamp and frame ID of the event.
    # Event argument properties should only be read inside the event notification function to avoid data races.

    try:
        event_exposure_end = grabber.device_property_map.find_integer(ic4.PropId.EVENT_EXPOSURE_END)
    except:
        print("EventExposureEnd is not supported by this device")
        return

    # Reset all camera settings to default so that prior configuration does not interfere with this program
    grabber.device_property_map.set_value(ic4.PropId.USER_SET_SELECTOR, "Default")
    grabber.device_property_map.execute_command(ic4.PropId.USER_SET_LOAD)

    # Configure a constant exposure time
    grabber.device_property_map.set_value(ic4.PropId.EXPOSURE_AUTO, "Off")
    grabber.device_property_map.set_value(ic4.PropId.EXPOSURE_TIME, 1000.0)

    # Enable trigger mode
    grabber.device_property_map.set_value(ic4.PropId.TRIGGER_MODE, "On")

    # Create our "real world" with a next-frame setup time of 40 ms
    real_world = RealWorld(40)

    # Run test without supplying EventExposureEnd event
    print("Test WITHOUT EventExposureEnd")
    run_test(grabber, real_world, 50, None)

    real_world.reset()

    # Run test with registered notification handler for EventExposureEnd
    # This time, the real-world simulation is notified to setup the next scene at an earlier point in time than before,
    # leading to a reduced cycle time.
    print("Test WITH EventExposureEnd")
    run_test(grabber, real_world, 50, event_exposure_end)


def run_test(grabber: ic4.Grabber, real_world: RealWorld, num_cycles: int, event_exposure_end: Optional[ic4.Property]):
    if event_exposure_end is not None:

        def handle_exposure_end(_: ic4.Property):
            # Extract frame ID from event data
            fid = grabber.device_property_map.get_value_int(ic4.PropId.EVENT_EXPOSURE_END_FRAME_ID)

            # Request real world scene-setup for next frame
            # At this time, exposure is complete, but the image is still being transmitted.
            # If we waited for this call until the image is transmitted completely, we would waste time.
            real_world.begin_setup_scene(fid + 1)

        token = event_exposure_end.event_add_notification(handle_exposure_end)

        # Enable EventExposureEnd event notification
        grabber.device_property_map.set_value(ic4.PropId.EVENT_SELECTOR, "ExposureEnd")
        grabber.device_property_map.set_value(ic4.PropId.EVENT_NOTIFICATION, "On")
    else:
        # Disable EventExposureEnd event notification
        grabber.device_property_map.set_value(ic4.PropId.EVENT_SELECTOR, "ExposureEnd")
        grabber.device_property_map.set_value(ic4.PropId.EVENT_NOTIFICATION, "Off")

    # An event to wait for an image being received by the sink.
    image_received = Event()
    # This event indicates that a new image can be triggered, therefore it is initially set.
    image_received.set()

    class SinkListener(ic4.QueueSinkListener):
        def sink_connected(self, sink: ic4.QueueSink, image_type: ic4.ImageType, min_buffers_required: int) -> bool:
            return True

        def frames_queued(self, sink: ic4.QueueSink):
            buffer = sink.pop_output_buffer()

            # Notify the test thread that an image was received, and it can proceed.
            image_received.set()

            print(".", end="")

            fid = buffer.meta_data.device_frame_number

            # Request real world scene-setup for next frame.
            # This call will be ignored if the setup was already requested by the EventExposureEnd notification handler.
            real_world.begin_setup_scene(fid + 1)

        def sink_disconnected(self, sink: ic4.QueueSink):
            pass

    listener = SinkListener()
    sink = ic4.QueueSink(listener)

    # Setup stream
    grabber.stream_setup(sink)

    # Request real-world scene for first frame.
    real_world.begin_setup_scene(0)

    print(f"Running {num_cycles} cycles...")
    start_time = time()
    for i in range(0, num_cycles):
        image_received.wait()
        image_received.clear()

        real_world.wait_setup_scene_completion()

        grabber.device_property_map.execute_command(ic4.PropId.TRIGGER_SOFTWARE)
    end_time = time()
    dt = end_time - start_time

    grabber.stream_stop()

    print()
    print(f"Processed {num_cycles} cycles in {dt*1000} ms. {num_cycles / dt} cycles/sec")

    if event_exposure_end is not None:
        event_exposure_end.event_remove_notification(token)


if __name__ == "__main__":
    ic4.Library.init()

    try:
        example_event_exposure_end()
    finally:
        ic4.Library.exit()
