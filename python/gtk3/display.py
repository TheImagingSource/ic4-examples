"""
IC4 Gtk-3 Display

This Module contains
- DisplayWidget
- DisplayWindow

This file can be copied into any Gtk-3 project and used as is.
"""

import os
from weakref import ref
import imagingcontrol4 as ic4

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk


class DisplayWidget(Gtk.GLArea):
    """
    Display widget for IC4 live streams.
    """
    _display_ref = None
    _is_initialized: bool = False

    def __init__(self, **kwargs):
        # ic4 renderer requires compatibility mode
        os.environ["MESA_GL_VERSION_OVERRIDE"] = "3.3COMPAT"
        super().__init__(**kwargs)
        self.connect("render", self.render)

    def render(self, area, context):
        """
        GLArea callback function

        See GLArea::render signal for more details.
        """
        area.make_current()

        if self._lazy_initialize():
            w = area.get_allocated_width()
            h = area.get_allocated_height()
            self._display_ref().render(w, h)

        area.queue_render()

        return True

    def _lazy_initialize(self):
        if (not self._is_initialized
                and self._display_ref is not None
                and self._display_ref() is not None):
            self._display_ref().initialize()
            self._is_initialized = True

        return self._is_initialized

    def as_display(self) -> ic4.Display:
        """
        Return widget as IC4::Display
        """

        if self._display_ref is None or self._display_ref() is None:
            display = ic4.ExternalOpenGLDisplay()
            self._display_ref = ref(display)
            return display

        return self._display_ref()


class DisplayWindow(Gtk.Window):
    """
    Convenience wrapper for DisplayWidget for usage as a standalone window.
    """
    display: DisplayWidget

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.display = DisplayWidget()
        self.add(self.display)

    def as_display(self) -> ic4.Display:
        """
        Return widget as IC4::Display
        """
        return self.display.as_display()
