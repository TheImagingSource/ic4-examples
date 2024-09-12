from PySide6.QtWidgets import QApplication, QMainWindow, QDialog

import imagingcontrol4 as ic4


class MainWindow(QMainWindow):
    # The main window for the simple test application

    def __init__(self):
        QMainWindow.__init__(self)
        self.setWindowTitle("IC Imaging Control 4 Python Library - First Steps")
        self.resize(1024, 768)

        # Create a Grabber object to communicate with a video capture device
        self.grabber = ic4.Grabber()

        # Show the pre-built dialog to let the user select a device
        # If the user selects a device, the function opens it in the grabber object
        dlg = ic4.pyside6.DeviceSelectionDialog(self.grabber, self)
        if not dlg.exec() == QDialog.Accepted:
            return

        # Create a display widget that will show live video
        display_widget = ic4.pyside6.DisplayWidget()
        self.setCentralWidget(display_widget)

        # Get ic4 display object from display widget and configure rendering options
        display = display_widget.as_display()
        display.set_render_position(ic4.DisplayRenderPosition.STRETCH_CENTER)

        try:
            # Reset all camera settings to default
            self.grabber.device_property_map.set_value(ic4.PropId.USER_SET_SELECTOR, "Default")
            self.grabber.device_property_map.execute_command(ic4.PropId.USER_SET_LOAD)
        except ic4.IC4Exception:
            # The driver/device might not support this, ignore and move on
            pass

        # Start a data stream from the device to the display
        self.grabber.stream_setup(None, display)


if __name__ == "__main__":
    # This is just the default Qt main function template, with added call to Library.init_context()
    from sys import argv

    app = QApplication(argv)
    app.setStyle("fusion")

    with ic4.Library.init_context():

        wnd = MainWindow()
        wnd.show()

        app.exec()
