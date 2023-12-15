from PySide6.QtWidgets import QApplication, QWidget, QMainWindow, QLabel

import imagingcontrol4 as ic4


def _window_handle(wnd: QWidget) -> int:
    # Helper function to get window handle from a QWidget
    return wnd.winId().__int__()


class MainWindow(QMainWindow):
    # The main window for the simple test application

    def __init__(self):
        QMainWindow.__init__(self)
        self.setWindowTitle("IC Imaging Control 4 Python Library - First Steps")
        self.resize(1024, 768)

        # Create a widget to use as the target for video display
        video_widget = QLabel("No device selected")
        self.setCentralWidget(video_widget)

        # Create a Grabber object to communicate with a video capture device
        self.grabber = ic4.Grabber()

        # Show the builtin dialog to let the user select a device
        # If the user selects a device, the function opens it in the grabber object
        if not ic4.Dialogs.grabber_select_device(self.grabber, _window_handle(self)):
            return

        # Create an IC4 EmbeddedDisplay that is using video_widget from above as presentation area
        display = ic4.EmbeddedDisplay(_window_handle(video_widget))
        # Configure the display to neatly stretch and center the live image
        display.set_render_position(ic4.DisplayRenderPosition.STRETCH_CENTER)

        try:
            self.grabber.device_property_map.set_value(ic4.PropId.USER_SET_SELECTOR, "Default")
            self.grabber.device_property_map.execute_command(ic4.PropId.USER_SET_LOAD)
        except ic4.IC4Exception:
            # The driver/device might not support this, ignore and move on
            pass

        # Start a data stream from the device to the display
        self.grabber.stream_setup(None, display)


if __name__ == "__main__":
    # This is just the default Qt main function template, with added calls to Library.init() and Library.exit()
    from sys import argv

    app = QApplication(argv)

    ic4.Library.init()

    wnd = MainWindow()
    wnd.show()

    app.exec()

    ic4.Library.exit()
