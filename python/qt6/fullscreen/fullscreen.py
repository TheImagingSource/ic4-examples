from sys import argv
import os
from PySide6.QtWidgets import QApplication, QWidget, QMainWindow, QHBoxLayout, QMessageBox
import glob

import imagingcontrol4 as ic4


class CameraWidget(ic4.pyside6.DisplayWidget):
    """
    Widget for live display of a camera. The camera to open is specified in a JSON file passed
    to the constructor of this class.
    """

    def __init__(self, device_descriptor: str):
        """
        Create a display widget, open a camera and start the live stream.

        :param device_descriptor:   Either JSON file name created by IC4 Demo App which specifies
                                    the camera to be used or a device identifier.
        """
        ic4.pyside6.DisplayWidget.__init__(self)
        self.grabber = ic4.Grabber()

        self.display = self.as_display()
        self.display.set_render_position(ic4.DisplayRenderPosition.STRETCH_CENTER)

        if os.path.exists(device_descriptor):
            try:
                self.grabber.device_open_from_state_file(device_descriptor)
                self.grabber.stream_setup(None, self.display)
            except ic4.IC4Exception as e:
                QMessageBox.information(self, None, f"Loading device-state file {device_descriptor} failed: {e}")
        else:
            try:
                self.grabber.device_open(device_descriptor)
                self.grabber.stream_setup(None, self.display)
            except ic4.IC4Exception as e:
                QMessageBox.information(self, None, f"Starting {device_descriptor} failed: {e}")


class MainWindow(QMainWindow):
    """This is the main window for the simple test application."""

    def __init__(self):
        QMainWindow.__init__(self)

        # Create the user interface.
        main_widget = QWidget()
        main_layout = QHBoxLayout()
        main_layout.setSpacing(0)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_widget.setLayout(main_layout)
        self.setCentralWidget(main_widget)

        # Enumerate all json files from working directory
        # and create a CameraWidget for each of the.
        # Add the new CameraWidget to the main_layout.
        # If there are no JSON files, then all available
        # cameras will be opened.

        device_state_file_list = glob.glob("*.json")

        if device_state_file_list:
            for device_state_file in device_state_file_list:
                main_layout.addWidget(CameraWidget(device_state_file))
        else:
            for device_info in ic4.DeviceEnum.devices():
                main_layout.addWidget(CameraWidget(device_info.unique_name))

        if main_layout.isEmpty():
            QMessageBox.information(self, None, "No JSON files or cameras found.")


def fullscreen_demo():
    app = QApplication(argv)
    app.setApplicationName("IC4 Fullscreen Demo")

    wnd = MainWindow()
    wnd.showFullScreen()

    app.exec()


if __name__ == "__main__":
    with ic4.Library.init_context():
        fullscreen_demo()
