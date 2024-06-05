#!/usr/bin/env python3

"""
IC4 Gtk-3 Display Example

This examples shows how to display a live stream with Gtk-3 widgets.
"""

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk

import imagingcontrol4 as ic4
import display as ic4display


class MainWindow(ic4display.DisplayWindow):
    def __init__(self):
        super().__init__(title="IC4 Gtk-3 Display Example")

        self.grabber = ic4.Grabber()
        self.grabber.device_open(ic4.DeviceEnum.devices()[0])
        self.grabber.device_property_map.set_value(ic4.PropId.WIDTH, 640)
        self.grabber.device_property_map.set_value(ic4.PropId.HEIGHT, 480)
        self.grabber.device_property_map.set_value(ic4.PropId.PIXEL_FORMAT,
                                                   ic4.PixelFormat.BayerBG12p)

        self.grabber.stream_setup(display=super().as_display())


if __name__ == "__main__":

    with ic4.Library.init_context(api_log_level=ic4.LogLevel.INFO,
                                  log_targets=ic4.LogTarget.STDERR):

        win = MainWindow()
        win.connect("destroy", Gtk.main_quit)
        win.show_all()
        Gtk.main()

        print(win.display.as_display().statistics)

        del win
