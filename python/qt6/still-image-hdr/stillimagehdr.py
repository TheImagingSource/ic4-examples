from typing import Any
import PySide6.QtCore
from PySide6.QtWidgets import QApplication, QWidget, QMainWindow, QHBoxLayout
from PySide6.QtWidgets import QVBoxLayout, QPushButton, QMessageBox
from PySide6.QtWidgets import QRadioButton
from PySide6.QtCore import QStandardPaths, QDir, QFileInfo, QEvent
import PySide6
import time
import threading
import cv2
import numpy as np
import sliderfloat as sf
import imagingcontrol4 as ic4

DEVICE_LOST_EVENT = QEvent.Type(QEvent.Type.User + 1)


class Listener(ic4.QueueSinkListener):
    buffer_list: list[ic4.ImageBuffer]

    def __init__(self):
        self.counter = 0
        self.frames_to_capture = 0
        self.capture = False
        self.buffer_list = []
        self.capture_end_event = threading.Event()

    def sink_connected(
        self,
        sink: ic4.QueueSink,
        image_type: ic4.ImageType,
        min_buffers_required: int,
    ) -> bool:
        sink.alloc_and_queue_buffers(min_buffers_required + 8)
        return True

    def start_capture(self, frames: int):
        """Start the image capture into the buffer_list
        :param frames: Number of frames to be saved in self.buffer_list
        """
        self.counter = 0
        self.buffer_list.clear()
        self.frames_to_capture = frames
        self.capture_end_event.clear()
        self.capture = True

    def frames_queued(self, sink: ic4.QueueSink):
        """If self.capture is true, the wanted number of images
        are stored into self.buffer_list
        """
        buffer = sink.pop_output_buffer()

        if self.capture:
            self.counter = self.counter + 1
            print(f"image {self.counter}/{self.frames_to_capture}")
            self.buffer_list.append(buffer)
            # End capture after desired number of frames.
            if self.counter >= self.frames_to_capture:
                self.capture_end_event.set()
                self.capture = False
                print("End Capture")


class MainWindow(QMainWindow):
    # The main window for the simple test application

    def __init__(self):
        QMainWindow.__init__(self)
        self.setWindowTitle("Still Image HDR")
        self.create_gui()

        self.check_for_devices()

        # Create a Grabber object to communicate with a video capture device
        self.grabber = ic4.Grabber()
        self.grabber.event_add_device_lost(
            lambda g: QApplication.postEvent(self, QEvent(DEVICE_LOST_EVENT))
        )

        # The buffer pool is used to display the resulting HDR image
        self.pool = ic4.BufferPool()

        self.listener = Listener()

        self.queue_sink = ic4.QueueSink(self.listener, [ic4.PixelFormat.BGR8])

        self.create_state_file_names()

        # Restore the last used video capture device.
        if QFileInfo.exists(self.device_file):
            try:
                self.grabber.device_open_from_state_file(self.device_file)
                self.on_start()
            except ic4.IC4Exception as e:
                QMessageBox.information(
                    self,
                    "",
                    f"Loading last used device failed: {e}",
                    QMessageBox.StandardButton.Ok,
                )

        self.update_controls()

    def check_for_devices(self) -> None:
        """Show a warning, if no interfaces are found by
        IC Imaging Control 4. This indicates, that no IC4
        GenTL Producers are installed.
        """
        if len(ic4.DeviceEnum.interfaces()) == 0:
            QMessageBox.warning(
                self,
                "Still Image HDR",
                "No interfaces found.\nIs an IC4 GenTL Producer from\n"
                + "https://www.theimagingsource.com/en-us/support/download/\n"
                + "installed?",
                QMessageBox.StandardButton.Ok,
            )

    def create_gui(self):
        """Create the user interface"""
        main_widget = QWidget()
        mainlayout = QHBoxLayout()
        btn_layout = QVBoxLayout()

        # Create a widget to use as the target for video display
        self.video_widget = ic4.pyside6.DisplayWidget()
        self.display = self.video_widget.as_display()
        self.display.set_render_position(
            ic4.DisplayRenderPosition.STRETCH_CENTER
        )

        self.result_widget = ic4.pyside6.DisplayWidget()
        self.result_display = self.result_widget.as_display()
        self.result_display.set_render_position(
            ic4.DisplayRenderPosition.STRETCH_CENTER
        )

        mainlayout.addWidget(self.video_widget)
        mainlayout.addWidget(self.result_widget)

        self.btn_device = QPushButton("Device")
        self.btn_properties = QPushButton("Properties")
        self.btn_start = QPushButton("Start")
        self.btn_snap = QPushButton("Snap Image")
        self.radio_frames2 = QRadioButton("2 Frames")
        self.radio_frames4 = QRadioButton("4 Frames")
        self.radio_frames4.setChecked(True)

        self.btn_device.setMaximumWidth(100)
        self.btn_properties.setMaximumWidth(100)
        self.btn_start.setMaximumWidth(100)
        self.btn_snap.setMaximumWidth(100)

        self.radio_frames2.setMaximumWidth(100)
        self.radio_frames4.setMaximumWidth(100)

        self.btn_device.clicked.connect(self.on_select_device)
        self.btn_properties.clicked.connect(self.on_device_properties)
        self.btn_start.clicked.connect(self.on_start)
        self.btn_snap.clicked.connect(self.on_snap)

        self.radio_frames2.clicked.connect(self.on_frames_clicked)
        self.radio_frames4.clicked.connect(self.on_frames_clicked)

        btn_layout.setAlignment(PySide6.QtCore.Qt.AlignTop)
        btn_layout.addWidget(self.btn_device)
        btn_layout.addWidget(self.btn_properties)
        btn_layout.addWidget(self.btn_start)
        btn_layout.addWidget(self.btn_snap)
        btn_layout.addWidget(self.radio_frames2)
        btn_layout.addWidget(self.radio_frames4)

        # create the sliders for the exposure time factors:
        self.factors = []
        self.factors.append(sf.Sliderfloat(0.01, 32.0, 0.5, 0.01, self))
        self.factors.append(sf.Sliderfloat(0.01, 32.0, 2.0, 0.01, self))
        self.factors.append(sf.Sliderfloat(0.01, 32.0, 8.0, 0.01, self))
        self.factors.append(sf.Sliderfloat(0.01, 32.0, 32.0, 0.01, self))

        for sld in self.factors:
            btn_layout.addWidget(sld)

        mainlayout.addLayout(btn_layout)
        main_widget.setLayout(mainlayout)

        self.setCentralWidget(main_widget)
        self.resize(1920, 540)

    def update_controls(self):
        """En- or disable the buttons."""
        if self.grabber.is_device_valid:
            self.btn_properties.setEnabled(True)
            self.btn_start.setEnabled(True)
            if self.grabber.is_streaming:
                self.btn_start.setText("Stop")
                self.btn_snap.setEnabled(True)
            else:
                self.btn_start.setText("Start")
                self.btn_snap.setEnabled(False)
        else:
            self.btn_properties.setEnabled(False)
            self.btn_start.setEnabled(False)
            self.btn_start.setText("Start")
            self.btn_snap.setEnabled(False)

    def create_state_file_names(self):
        """Create directory for the device state
        and make the name of the device state file.
        """
        appdata_directory = QStandardPaths.writableLocation(
            QStandardPaths.AppDataLocation
        )
        QDir(appdata_directory).mkpath(".")
        self.device_file = appdata_directory + "/stillimagehdr.json"

    def on_select_device(self):
        """Show the IC 4 device selection dialog."""
        dlg = ic4.pyside6.DeviceSelectionDialog(self.grabber, self)
        if dlg.exec() == PySide6.QtWidgets.QDialog.Accepted:
            pass
        self.update_controls()

    def on_device_properties(self):
        """Show the property dialog for the selected device."""
        if self.grabber.is_device_valid:
            dlg = ic4.pyside6.PropertyDialog(self.grabber, self)
            dlg.exec()

    def on_start(self):
        """Start and stop the live video."""
        if self.grabber.is_device_valid:
            if self.grabber.is_streaming:
                self.grabber.stream_stop()
            else:
                self.grabber.stream_setup(self.queue_sink, self.display)

        self.update_controls()

    def on_frames_clicked(self):
        """Show the exposure factor sliders depending on
        whether 2 or 4 frames are to be captured.
        """
        if self.radio_frames4.isChecked():
            self.factors[1].setVisible(True)
            self.factors[2].setVisible(True)
        else:
            self.factors[1].setVisible(False)
            self.factors[2].setVisible(False)

    def on_snap(self):
        """Start the snap and HDR calculation process"""
        self.snap_and_process()

    def customEvent(self, ev: QEvent):
        """Handle the Device Lost custom event"""
        if ev.type() == DEVICE_LOST_EVENT:
            self.on_device_lost()

    def on_device_lost(self):
        """Show a message box, if the current camera disconnected."""
        QMessageBox.warning(
            self,
            "Still Image HDR Capture",
            "The video capture device is lost!",
            QMessageBox.StandardButton.Ok,
        )
        self.update_controls()

    def closeEvent(self, event):
        """Called by PySide6 when closing the application."""

        if self.grabber.is_streaming:
            self.grabber.stream_stop()

        if self.grabber.is_device_valid:
            self.grabber.device_save_state_to_file(self.device_file)
        time.sleep(1)
        print("The End")

    def calc_exposure_times(self, exposure_time: float) -> list[float]:
        """Create a list of exposure times for the images to be captured.

        Args:
            exposure_time (float): Current exposure time

        Returns:
            list[float]: Array containing the exposure times
        """
        times = []

        if self.radio_frames2.isChecked():
            times.append(exposure_time * self.factors[0].get_value())
            times.append(exposure_time * self.factors[3].get_value())
        else:
            times = [exposure_time * f.get_value() for f in self.factors]

        return times

    def acquire_multi_frame_output_mode(
        self, prop_map: ic4.PropertyMap, exposure_times: list[float]
    ) -> list[ic4.ImageBuffer]:
        """Setup the multi frame output mode and capture images. The number
        of captured images is determined by length of exposure_times array.

        Args:
            prop_map (ic4.PropertyMap): The camera's device property map
            exposure_times (list[float]): Array of exposure times to be used

        Returns:
            list[ic4.ImageBuffer]: List of the captured images
        """
        buffer_list = []
        prop_map.set_value(ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_ENABLE, True)

        prop_map.set_value(
            ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_EXPOSURE_TIME0,
            exposure_times[0],
        )
        prop_map.set_value(
            ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_EXPOSURE_TIME1,
            exposure_times[1],
        )

        if len(exposure_times) == 4:
            prop_map.set_value(
                ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_FRAME_COUNT, "4 Frames"
            )
            prop_map.set_value(
                ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_EXPOSURE_TIME2,
                exposure_times[2],
            )
            prop_map.set_value(
                ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_EXPOSURE_TIME3,
                exposure_times[3],
            )
        else:
            prop_map.set_value(
                ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_FRAME_COUNT, "2 Frames"
            )

        prop_map.set_value(
            ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_CUSTOM_GAIN, False
        )

        # We need to wait for three images to be sure, the new settings
        # are effective in the camera.
        self.listener.start_capture(3)
        self.listener.capture_end_event.wait(3)

        self.listener.start_capture(len(exposure_times))

        success = self.listener.capture_end_event.wait(5)

        if success:
            for buffer in self.listener.buffer_list:
                buffer_list.append(buffer)
        else:
            print("Timeout")

        prop_map.set_value(ic4.PropId.MULTI_FRAME_SET_OUTPUT_MODE_ENABLE, False)

        return buffer_list

    def snap_single_frame(
        self,
        exposure_time: float,
        prop_map: ic4.PropertyMap,
        buffer_list: list[ic4.ImageBuffer],
    ) -> None:
        """Snap a single frame on software trigger.

        Args:
            exposure_time (float): Exposure time to be used for the frame.
            prop_map (ic4.PropertyMap): The device property map of self.grabber.
            buffer_list (list[ic4.ImageBuffer]): The list of buffers,
            that receives the image
        """
        prop_map.set_value(ic4.PropId.EXPOSURE_TIME, exposure_time)
        self.listener.start_capture(1)
        prop_map.execute_command(ic4.PropId.TRIGGER_SOFTWARE)

        success = self.listener.capture_end_event.wait(2)

        if success:
            buffer_list.append(self.listener.buffer_list[0])
        else:
            print("timeout!")

    def acquire_software_trigger(
        self, prop_map: ic4.PropertyMap, exposure_times: list[float]
    ) -> list[ic4.ImageBuffer]:
        """Capture a number of images using software trigger.

        Args:
            prop_map (ic4.PropertyMap): The cameras's device property map.
            exposure_times (list[float]): The list of exposure times to use. Its
            length determines the number of images to capture.

        Returns:
            list[ic4.ImageBuffer]: A list of captured images.
        """
        buffer_list = []
        prop_map.set_value(ic4.PropId.TRIGGER_MODE, "On")
        # Wait a moment for the camera getting ready for triggering.
        fps = prop_map.get_value_float(ic4.PropId.ACQUISITION_FRAME_RATE)
        time.sleep(2.0 / fps)

        for exposure in exposure_times:
            self.snap_single_frame(exposure, prop_map, buffer_list)

        prop_map.set_value(ic4.PropId.TRIGGER_MODE, "Off")

        return buffer_list

    def enable_automatics(self, prop_map: ic4.PropertyMap, value: str) -> None:
        """Turn the automatics of the camera off or on

        Args:
            prop_map (ic4.PropertyMap): The device property map of the camera
            value (str): Value to be set. Can be "Off" or "Continuous"
        """
        prop_map.set_value(ic4.PropId.EXPOSURE_AUTO, value)
        prop_map.set_value(ic4.PropId.GAIN_AUTO, value)
        prop_map.try_set_value(ic4.PropId.BALANCE_WHITE_AUTO, value)

        # Iris is on motorized zoom cameras only.
        prop_map.try_set_value(ic4.PropId.IRIS_AUTO, value)

    def snap_and_process(self):
        """Snap an image with different exposure times and process them to an HDR image.
        Steps are:
        - Disable camera automatics and get current exposure time.
        - Calculate the different exposure times. That indicates the count of images to capture too.
        - Capture images, try to use multi frame output mode. If that fails, use software trigger instead.
        - Process the images into an HDR image.
        - Save and display the images.
        - Enable camera automatics.
        """
        start = time.time()
        prop_map = self.grabber.device_property_map

        self.enable_automatics(prop_map, "Off")

        current_exposure_time = prop_map.get_value_float(
            ic4.PropId.EXPOSURE_TIME
        )

        exposure_times = self.calc_exposure_times(current_exposure_time)

        try:
            buffer_list = self.acquire_multi_frame_output_mode(
                prop_map, exposure_times
            )

        except ic4.IC4Exception:
            buffer_list = self.acquire_software_trigger(
                prop_map, exposure_times
            )

        wrap_list = [b.numpy_wrap() for b in buffer_list]
        print(f"Time taken to capture images was {time.time()-start} seconds")

        # Merge Mertens is used as HDR image merger
        merger = cv2.createMergeMertens()
        res_merger = merger.process(wrap_list)
        res_8bit = np.clip(res_merger * 255, 0, 255).astype("uint8")

        print(f"Time taken to run the code was {time.time()-start} seconds")

        # Save the captured and processed images
        for i, b in enumerate(buffer_list):
            b.save_as_jpeg(f"{i+1}.jpg")

        cv2.imwrite(r"fusion_mertens.jpg", res_8bit)

        self.show_hdr_image(res_8bit)

        # Restore previous exposure time that was determined by exposure auto
        prop_map.set_value(ic4.PropId.EXPOSURE_TIME, current_exposure_time)

        self.enable_automatics(prop_map, "Continuous")

    def show_hdr_image(self, image: np.typing.NDArray[Any]):
        """Show the numpy image on the result display.
        An ic4 display is used for this, therefore
        the image must be copied into a buffer of a
        buffer pool

        Args:
            image (np.typing.NDArray[Any]): The numpy array containing the image
        """
        poolbuffer = self.pool.get_buffer(
            ic4.ImageType(ic4.PixelFormat.BGR8, image.shape[1], image.shape[0])
        )
        matdest = poolbuffer.numpy_wrap()

        cv2.copyTo(image, None, matdest)

        self.result_display.display_buffer(poolbuffer)


if __name__ == "__main__":
    # This is just the default Qt main function template, with added calls to Library.init() and Library.exit()
    from sys import argv

    app = QApplication(argv)

    ic4.Library.init()

    wnd = MainWindow()
    wnd.show()

    app.exec()

    # ic4.Library.exit()
