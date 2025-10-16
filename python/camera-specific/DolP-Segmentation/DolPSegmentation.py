import PySide6.QtCore
from PySide6.QtWidgets import (
    QApplication,
    QWidget,
    QMainWindow,
    QHBoxLayout,
    QVBoxLayout,
    QPushButton,
    QMessageBox,
)

from PySide6.QtCore import QStandardPaths, QDir, QFileInfo, QEvent
import PySide6
import time
import numpy as np

import imagingcontrol4 as ic4
import sliderctrl as sldctrl

WINDOW_TITLE = "DoLP Segmentation"


class Listener(ic4.QueueSinkListener):
    # Listener to demonstrate processing and displaying received images

    def __init__(self, display: ic4.Display):
        self.display = display
        self.buffer_pool = ic4.BufferPool()
        self.threshold_dolp = 30
        self.threshold_intensity = 30

    def sink_connected(
        self,
        sink: ic4.QueueSink,
        image_type: ic4.ImageType,
        min_buffers_required: int,
    ) -> bool:
        sink.alloc_and_queue_buffers(min_buffers_required + 1)
        return True

    def frames_queued(self, sink: ic4.QueueSink):
        # Get the new buffer from the sink
        buffer = sink.pop_output_buffer()
        src_wrap = buffer.numpy_wrap()

        dest_buffer = self.buffer_pool.get_buffer(
            buffer.image_type.with_pixel_format(ic4.PixelFormat.BGR8)
        )

        dest_wrap = dest_buffer.numpy_wrap()

        if buffer.image_type.pixel_format == ic4.PixelFormat.PolarizedADIMono8:
            # Extract DoLP and intensity channels and replicate across RGB
            dolp = np.repeat(src_wrap[:, :, 1:2], 3, axis=2)
            intensity = np.repeat(src_wrap[:, :, 2:3], 3, axis=2)
            dest_wrap[:, :, 0:3] = np.where(
                np.logical_and(
                    dolp >= self.threshold_dolp,
                    intensity >= self.threshold_intensity,
                ),
                [0, 0, 255],
                intensity,
            )
        else:
            # Use polarization channels 1–3 for thresholding and 6–4 (reversed)
            dest_wrap[:, :, 0:3] = np.where(
                np.logical_and(
                    src_wrap[:, :, 1:4] >= self.threshold_dolp,
                    src_wrap[:, :, 6:3:-1] >= self.threshold_intensity,
                ),
                [[[0, 0, 255]]],
                src_wrap[:, :, 6:3:-1],
            )

        self.display.display_buffer(dest_buffer)


class MainWindow(QMainWindow):
    # The main window for the simple test application

    def __init__(self):
        QMainWindow.__init__(self)
        self.setWindowTitle(WINDOW_TITLE)
        self.create_gui()
        # Create a Grabber object to communicate with a video capture device
        self.grabber = ic4.Grabber()
        self.listener = Listener(self.display)
        self.sink = ic4.QueueSink(self.listener)

        # Show a warning, if there are no devices. May no GenTL
        # producer is installed.
        self.check_for_devices()

        self.on_select_device()

        self.update_controls()

    def create_gui(self) -> None:
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

        mainlayout.addWidget(self.video_widget)

        self.btn_device = QPushButton("Device")
        self.btn_properties = QPushButton("Properties")
        self.btn_start = QPushButton("Start")

        self.sld_threshold_dolp = sldctrl.SliderControlInt(
            "DoLP Threshold", 30, 0, 255
        )
        self.sld_threshold_dolp.value_changed.connect(
            self.on_threshold_dolp_changed
        )
        self.sld_threshold_dolp.setMaximumWidth(250)

        self.sld_threshold_intensity = sldctrl.SliderControlInt(
            "Intensity Threshold", 10, 0, 255
        )
        self.sld_threshold_intensity.value_changed.connect(
            self.on_threshold_intensity_changed
        )
        self.sld_threshold_intensity.setMaximumWidth(250)

        self.btn_device.setMaximumWidth(100)
        self.btn_properties.setMaximumWidth(100)
        self.btn_start.setMaximumWidth(100)

        self.btn_device.clicked.connect(self.on_select_device)
        self.btn_properties.clicked.connect(self.on_device_properties)
        self.btn_start.clicked.connect(self.on_start)

        btn_layout.setAlignment(PySide6.QtCore.Qt.AlignTop)
        btn_layout.addWidget(self.btn_device)
        btn_layout.addWidget(self.btn_properties)
        btn_layout.addWidget(self.btn_start)
        btn_layout.addWidget(self.sld_threshold_dolp)
        btn_layout.addWidget(self.sld_threshold_intensity)

        mainlayout.addLayout(btn_layout)

        main_widget.setLayout(mainlayout)

        self.setCentralWidget(main_widget)
        self.resize(1024, 768)

    def check_for_devices(self) -> None:
        """Show a warning, if no interfaces are found by
        IC Imaging Control 4. This could mean that no IC4
        GenTL Producers are installed.
        """
        if len(ic4.DeviceEnum.interfaces()) == 0:
            QMessageBox.warning(
                self,
                WINDOW_TITLE,
                "No interfaces found.\nIs an IC4 GenTL Producer from\n"
                + "https://www.theimagingsource.com/en-us/support/download/\n"
                + "installed?",
                QMessageBox.StandardButton.Ok,
            )

    def update_controls(self) -> None:
        """Enable or disable the controls depending on the device status"""
        if self.grabber.is_device_valid:
            self.btn_properties.setEnabled(True)
            self.btn_start.setEnabled(True)
            if self.grabber.is_streaming:
                self.btn_start.setText("Stop")
            else:
                self.btn_start.setText("Start")
        else:
            self.btn_properties.setEnabled(False)
            self.btn_start.setEnabled(False)
            self.btn_start.setText("Start")

    def is_polarization_camera(self) -> bool:
        """Check, whether a polarization camera is selected.
        Polarization camera names start with "DYK" or "DZK"
        If the current camera is not a polarization camera, a
        warning is shown and the camera is closed.

        Returns:
            bool: True, if a polarization camera is used.
        """
        if self.grabber.is_device_valid:
            if self.grabber.device_info.model_name.startswith(
                "DYK"
            ) or self.grabber.device_info.model_name.startswith("DZK"):
                self.setWindowTitle(
                    WINDOW_TITLE + " " + self.grabber.device_info.model_name
                )

                return True
            else:
                QMessageBox.warning(
                    self,
                    WINDOW_TITLE,
                    "No polarization camera (DYK or DZK) selected.",
                    QMessageBox.StandardButton.Ok,
                )
                self.setWindowTitle(WINDOW_TITLE)

        self.grabber.device_close()
        return False

    def set_polarization_format(self) -> bool:
        """Try to set the mono polarization format first. If this fails
        then there is color polarization format and the RGB polarization
        format is set.

        Returns:
            bool: True on success, False on any error.
        """
        self.grabber.device_property_map.set_value(
            "ProcessedPixelFormatsEnable", True
        )

        try:
            self.grabber.device_property_map.set_value(
                ic4.PropId.PIXEL_FORMAT, ic4.PixelFormat.PolarizedADIMono8
            )
            return True
        except ic4.IC4Exception:
            try:
                self.grabber.device_property_map.set_value(
                    ic4.PropId.PIXEL_FORMAT, ic4.PixelFormat.PolarizedADIRGB8
                )
                return True
            except ic4.IC4Exception as e:
                print(e.message)
        return False

    def on_select_device(self) -> None:
        """Show the IC 4 device selection dialog."""
        dlg = ic4.pyside6.DeviceSelectionDialog(self.grabber, self)
        if dlg.exec() == PySide6.QtWidgets.QDialog.Accepted:
            if self.is_polarization_camera():
                self.set_polarization_format()
                self.on_start()

        self.update_controls()

    def on_device_properties(self) -> None:
        """Show the property dialog for the selected device."""
        if self.grabber.is_device_valid:
            dlg = ic4.pyside6.PropertyDialog(self.grabber, self)
            if dlg.exec() == PySide6.QtWidgets.QDialog.Accepted:
                return

    def on_start(self) -> None:
        """Start and stop the live video."""
        if self.grabber.is_device_valid:
            if self.grabber.is_streaming:
                self.btn_start.setText("Stop")
                self.grabber.stream_stop()
            else:
                self.btn_start.setText("Start")

                self.grabber.stream_setup(self.sink)

        self.update_controls()

    def on_threshold_dolp_changed(self, value: int):
        """Event handler of the DoLP threshold slider. Pass the new
        DoLP Threshold to the self.listener

        Args:
            value (int): The new value from by the slider
        """
        self.listener.threshold_dolp = value

    def on_threshold_intensity_changed(self, value: int):
        """Event handler of the intensity threshold slider. Pass the new
        DoLP Threshold to the self.listener

        Args:
            value (int): The new value from by the slider
        """
        self.listener.threshold_intensity = value * 3

    def closeEvent(self, event: QEvent) -> None:
        """Called by Pyside6, when the window is closed.
        The video stream is stopped and the grabber's
        device state is saved, so the camera will be
        opened automatically at next program start.

        Args:
            event (QEvent): Event.
        """
        if self.grabber.is_streaming:
            self.grabber.stream_stop()


if __name__ == "__main__":
    from sys import argv

    app = QApplication(argv)
    app.setApplicationName(WINDOW_TITLE)
    app.setStyle("fusion")

    ic4.Library.init()

    wnd = MainWindow()
    wnd.show()

    app.exec()

    ic4.Library.exit()
