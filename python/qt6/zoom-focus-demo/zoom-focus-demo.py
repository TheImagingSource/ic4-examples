
import imagingcontrol4 as ic4

from PySide6.QtWidgets import QApplication

from mainwindow import MainWindow

def zoomfocus_main():
    # This application shows the use of Zoom and Focus-related features of 39G cameras

    with ic4.Library.init_context(ic4.LogLevel.DEBUG, ic4.LogLevel.DEBUG, ic4.LogTarget.STDERR):
        app = QApplication()
        app.setApplicationName("ic4-zoom-focus-demo")
        app.setApplicationDisplayName("IC4 Zoom+Focus Demo Application")
        app.setStyle("fusion")

        w = MainWindow()
        w.show()

        app.exec()

if __name__ == "__main__":
    zoomfocus_main()