
from PySide6.QtCore import QEvent, Qt
from PySide6.QtGui import QIntValidator, QCloseEvent
from PySide6.QtWidgets import QApplication, QMainWindow, QDialog, QMessageBox
from PySide6.QtWidgets import QVBoxLayout, QHBoxLayout, QFormLayout
from PySide6.QtWidgets import QPushButton, QLabel, QSplitter, QFrame, QSlider, QLineEdit, QCheckBox, QPlainTextEdit

import imagingcontrol4 as ic4

# Define application events
UPDATE_FOCUS_EVENT = QEvent.Type(QEvent.Type.User + 1)
UPDATE_FOCUS_AUTO_EVENT = QEvent.Type(QEvent.Type.User + 2)
ZOOM_MOVE_COMPLETED_EVENT = QEvent.Type(QEvent.Type.User + 3)
FOCUS_MOVE_COMPLETED_EVENT = QEvent.Type(QEvent.Type.User + 4)
DEVICE_LOST_EVENT = QEvent.Type(QEvent.Type.User + 5)
UPDATE_IRIS_EVENT = QEvent.Type(QEvent.Type.User + 6)

class MainWindow(QMainWindow):
    def __init__(self):
        QMainWindow.__init__(self)
        self.setMinimumSize(1280, 700)

        # Create the grabber object that will control the camera
        self.grabber = ic4.Grabber()
        # Register event notification handler for device-lost event
        # The notification is run on a separate thread, which should not interact with UI elements
        # Therefore, post an event to the main thread
        self.grabber.event_add_device_lost(lambda g: QApplication.postEvent(self, QEvent(DEVICE_LOST_EVENT)))

        # Create display widget
        self.display_widget = ic4.pyside6.DisplayWidget()
        self.display_widget.setMinimumSize(640, 480)
        self.display = self.display_widget.as_display()
        self.display.set_render_position(ic4.DisplayRenderPosition.STRETCH_CENTER)

        # Add device configuration buttons
        self.buttons = QHBoxLayout()
        select_device = QPushButton("Select Device")
        select_device.pressed.connect(self.onSelectDevice)
        device_properties = QPushButton("Device Properties")
        device_properties.pressed.connect(self.onDeviceProperties)
        self.buttons.addWidget(select_device)
        self.buttons.addWidget(device_properties)

        # Add a layout that will contain dynamically created controls
        self.props_layout = QFormLayout()

        # Create right-side property area
        right = QVBoxLayout()
        right.addLayout(self.buttons)
        right.addLayout(self.props_layout)
        right.addStretch()
        right_frame = QFrame()
        right_frame.setLayout(right)

        # Top-level is organized by a splitter
        splitter = QSplitter()
        splitter.setChildrenCollapsible(False)
        splitter.addWidget(self.display_widget)
        splitter.addWidget(right_frame)
        splitter.setSizes([960, 320])
        self.setCentralWidget(splitter)

        self.onSelectDevice()

    def customEvent(self, ev: QEvent):
        # Dispatch application events posted from property notification functions
        if ev.type() == UPDATE_FOCUS_EVENT:
            self.onUpdateFocus()
        if ev.type() == UPDATE_FOCUS_AUTO_EVENT:
            self.onUpdateFocusAuto()
        if ev.type() == ZOOM_MOVE_COMPLETED_EVENT:
            self.onZoomMoveCompleted()
        if ev.type() == FOCUS_MOVE_COMPLETED_EVENT:
            self.onFocusMoveCompleted()
        if ev.type() == DEVICE_LOST_EVENT:
            self.onDeviceLost()
        if ev.type() == UPDATE_IRIS_EVENT:
            self.onUpdateIris()

    def closeEvent(self, ev: QCloseEvent):
        # Some drivers don't like their stream being stopped late
        if self.grabber.is_streaming:
            self.grabber.stream_stop()

    def zoomSliderChanged(self, val):
        # The zoom slider changed: Set new value and update text
        # This could also be self.zoom.value = val, but let's write without a property object
        self.grabber.device_property_map.set_value(ic4.PropId.ZOOM, val)
        self.zoom_edit.setText(str(val))

    def zoomEditDone(self):
        # If the zoom text was changed, set new value and update slider
        val = int(self.zoom_edit.text())
        # Only write property when the text was actually changed
        if self.zoom.value != val:
            self.zoom.value = val
            self.zoom_slider.setValue(val)

    def focusSliderChanged(self, val):
        # The focus slider changed: Set new value
        # This could also be self.focus.value = val, but let's write without a property object
        self.grabber.device_property_map.set_value(ic4.PropId.FOCUS, val)
        # No need to update edit box, focus change notification will do that

    def focusEditDone(self):
        # If the focus text was changed, set the new value
        val = int(self.focus_edit.text())
        # Only write property when the text was actually changed
        if self.focus.value != val:
            self.focus.value = val
            # No need to update slider, focus change notification will do that

    def onUpdateFocus(self):
        # This function is called when the notification event for the Focus feature was raised
        # It updates both the text box and slider
        val = self.focus.value
        self.focus_edit.setText(str(val))
        self.focus_slider.blockSignals(True)
        self.focus_slider.setValue(val)
        self.focus_slider.blockSignals(False)

    def onFocusAuto(self):
        # Run auto focus
        if isinstance(self.focus_auto, ic4.PropCommand):
            self.focus_auto.execute()

            if not self.focus_auto.is_done:
                self.run_auto_focus.setEnabled(False)
        else:
            self.focus_auto.value = "Once"

    def onUpdateFocusAuto(self):
        # This function is called when the notification event for the FocusAuto feature was raised
        # If the command is completed, re-enable the button and log

        if isinstance(self.focus_auto, ic4.PropCommand):
            if self.focus_auto.is_done and not self.run_auto_focus.isEnabled():
                self.run_auto_focus.setEnabled(True)
                val = self.grabber.device_property_map.find_integer(ic4.PropId.FOCUS).value
                self.event_log.appendPlainText(f"FocusAuto completed (at {val})")
        else:
            if self.focus_auto.value != "Once":
                val = self.grabber.device_property_map.find_integer(ic4.PropId.FOCUS).value
                self.event_log.appendPlainText(f"FocusAuto completed (at {val})")


    def irisSliderChanged(self, val):
        # The iris slider changed: Set new value
        # This could also be self.iris.value = val, but let's write without a property object
        self.grabber.device_property_map.set_value(ic4.PropId.IRIS, val)
        # No need to update edit box, iris change notification will do that

    def irisEditDone(self):
        # If the iris text was changed, set the new value
        val = int(self.iris_edit.text())
        # Only write property when value was actually changed
        if self.iris.value != val:
            self.iris.value = val
            # No need to update slider, iris change notification will do that

    def onUpdateIris(self):
        # This function is called when the notification event for the Iris feature was raised
        # It updates both the text box and slider
        val = self.iris.value
        self.iris_edit.setText(str(val))
        self.iris_edit.setEnabled(not self.iris.is_locked)
        self.iris_slider.blockSignals(True)
        self.iris_slider.setValue(val)
        self.iris_slider.setEnabled(not self.iris.is_locked)
        self.iris_slider.blockSignals(False)

    def onIrisAutoChanged(self, state: Qt.CheckState):
        # Change iris auto state
        self.grabber.device_property_map.set_value(ic4.PropId.IRIS_AUTO, state == Qt.CheckState.Checked)

    def onIrcutChanged(self, state: Qt.CheckState):
        # Change movable IR-Cut filter state
        self.grabber.device_property_map.set_value(ic4.PropId.IR_CUT_FILTER_ENABLE, state == Qt.CheckState.Checked)

    def onZoomMoveCompleted(self):
        # This function is called when the notification event for the ZoomMoveCompleted feature was raised
        # Read the associated zoom value and log
        val = self.grabber.device_property_map.find_integer(ic4.PropId.EVENT_ZOOM_MOVE_COMPLETED_ZOOM).value
        self.event_log.appendPlainText(f"Zoom move completed (at {val})")

    def onFocusMoveCompleted(self):
        # This function is called when the notification event for the FocusMoveCompleted feature was raised
        # Read the associated focus value and log
        val = self.grabber.device_property_map.find_integer(ic4.PropId.EVENT_FOCUS_MOVE_COMPLETED_FOCUS).value
        self.event_log.appendPlainText(f"Focus move completed (at {val})")

    def onSelectDevice(self):
        # Let the user select a camera
        dlg = ic4.pyside6.DeviceSelectionDialog(self.grabber, self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            # Update UI for the new device
            self.onDeviceOpened()
            # Start stream with display
            self.grabber.stream_setup(display=self.display)

    def onDeviceProperties(self):
        # Build appropriate title
        title = f"{self.grabber.device_info.model_name} Properties" if self.grabber.is_device_open else "No Device Selected"
        # Show property dialog
        dlg = ic4.pyside6.PropertyDialog(self.grabber, self, title)
        dlg.exec()

    def onDeviceOpened(self):
        # Clear out controls generated for the previous device
        while self.props_layout.count() > 0:
            self.props_layout.removeRow(0)

        if self.grabber.is_device_valid:

            # This flag tracks whether a control is present that can use logging
            create_event_log = False

            try:
                # Get Zoom property
                self.zoom = self.grabber.device_property_map.find_integer(ic4.PropId.ZOOM)

                # Create Zoom slider
                self.zoom_slider = QSlider(Qt.Orientation.Horizontal)
                self.zoom_slider.setMinimum(self.zoom.minimum)
                self.zoom_slider.setMaximum(self.zoom.maximum)
                self.zoom_slider.setValue(self.zoom.value)
                self.zoom_slider.valueChanged.connect(self.zoomSliderChanged)

                # Create Zoom textbox
                self.zoom_edit = QLineEdit()
                self.zoom_edit.setMinimumWidth(60)
                self.zoom_edit.setMaximumWidth(60)
                self.zoom_edit.setValidator(QIntValidator(self.zoom.minimum, self.zoom.maximum))
                self.zoom_edit.setText(str(self.zoom.value))
                self.zoom_edit.editingFinished.connect(self.zoomEditDone)

                # Add to layout
                zoom_group = QHBoxLayout()
                zoom_group.addWidget(self.zoom_slider)
                zoom_group.addWidget(self.zoom_edit)
                self.props_layout.addRow(QLabel("Zoom"), zoom_group)
            except:
                self.zoom = None

            try:
                # Get Focus property
                self.focus = self.grabber.device_property_map.find_integer(ic4.PropId.FOCUS)
                # Focus can be changed by FocusAuto, so register a notification handler
                # The notification is run on a separate thread, which should not interact with UI elements
                # Therefore, post an event to the main thread
                self.focus.event_add_notification(lambda p: QApplication.postEvent(self, QEvent(UPDATE_FOCUS_EVENT)))

                # Create Focus slider
                self.focus_slider = QSlider(Qt.Orientation.Horizontal)
                self.focus_slider.valueChanged.connect(self.focusSliderChanged)
                self.focus_slider.setMinimum(self.focus.minimum)
                self.focus_slider.setMaximum(self.focus.maximum)
                self.focus_slider.setValue(self.focus.value)

                # Create Focus textbox
                self.focus_edit = QLineEdit()
                self.focus_edit.setMinimumWidth(60)
                self.focus_edit.setMaximumWidth(60)
                self.focus_edit.setValidator(QIntValidator(self.focus.minimum, self.focus.maximum))
                self.focus_edit.setText(str(self.focus.value))
                self.focus_edit.editingFinished.connect(self.focusEditDone)

                # Add to layout
                focus_group = QHBoxLayout()
                focus_group.addWidget(self.focus_slider)
                focus_group.addWidget(self.focus_edit)
                self.props_layout.addRow(QLabel("Focus"), focus_group)
            except:
                self.focus = None

            try:
                # Get FocusAuto property for 39G series camera
                self.focus_auto = self.grabber.device_property_map.find_command(ic4.PropId.FOCUS_AUTO)
                create_event_log = True
            except:
                self.focus_auto = None

            if self.focus_auto is None:
                try:
                    # Get FocusAuto property for software zoom in Z series cameras
                    self.focus_auto = self.grabber.device_property_map.find_enumeration(ic4.PropId.FOCUS_AUTO)
                    create_event_log = True
                except:
                    self.focus_auto = None
                
            if self.focus_auto is None:
                try:
                    # Get FocusOnePush property for software zoom via DirectShow producer
                    self.focus_auto = self.grabber.device_property_map.find_command("Focus_One_Push")
                except:
                    self.focus_auto = None

            if self.focus_auto is not None:
                # FocusAuto can indicate when it is done, so register a notification handler
                # The notification is run on a separate thread, which should not interact with UI elements
                # Therefore, post an event to the main thread
                self.focus_auto.event_add_notification(lambda p: QApplication.postEvent(self, QEvent(UPDATE_FOCUS_AUTO_EVENT)))

                # Create FocusAuto button
                self.run_auto_focus = QPushButton("Execute")
                self.run_auto_focus.pressed.connect(self.onFocusAuto)

                # Add to layout
                self.props_layout.addRow(QLabel("Auto-Focus"), self.run_auto_focus)

            try:
                # Get iris property
                self.iris = self.grabber.device_property_map.find_integer(ic4.PropId.IRIS)

                # Iris can be changed by IrisAuto, so register a notification handler
                # The notification is run on a separate thread, which should not interact with UI elements
                # Therefore, post an event to the main thread
                self.iris.event_add_notification(lambda p: QApplication.postEvent(self, QEvent(UPDATE_IRIS_EVENT)))

                # Create Iris slider
                self.iris_slider = QSlider(Qt.Orientation.Horizontal)
                self.iris_slider.valueChanged.connect(self.irisSliderChanged)
                self.iris_slider.setMinimum(self.iris.minimum)
                self.iris_slider.setMaximum(self.iris.maximum)
                self.iris_slider.setValue(self.iris.value)
                self.iris_slider.setEnabled(not self.iris.is_locked)

                # Create Iris textbox
                self.iris_edit = QLineEdit()
                self.iris_edit.setMinimumWidth(60)
                self.iris_edit.setMaximumWidth(60)
                self.iris_edit.setValidator(QIntValidator(self.iris.minimum, self.iris.maximum))
                self.iris_edit.setText(str(self.iris.value))
                self.iris_edit.editingFinished.connect(self.irisEditDone)
                self.iris_edit.setEnabled(not self.iris.is_locked)

                # Add to layout
                iris_group = QHBoxLayout()
                iris_group.addWidget(self.iris_slider)
                iris_group.addWidget(self.iris_edit)
                self.props_layout.addRow(QLabel("Iris"), iris_group)
            except:
                self.iris = None

            try:
                # Get IrisAuto property
                iris_auto = self.grabber.device_property_map.find_boolean(ic4.PropId.IRIS_AUTO)
            except:
                iris_auto = None

            if iris_auto is None:
                try:
                    # Get IrisAuto property
                    iris_auto = self.grabber.device_property_map.find_enumeration(ic4.PropId.IRIS_AUTO)
                except:
                    iris_auto = None

            if iris_auto:
                # Create checkbox
                iris_auto_check = QCheckBox("Enabled")

                if isinstance(iris_auto, ic4.PropEnumeration):
                    iris_auto_check.setChecked(iris_auto.value == "Continuous")
                else:
                    iris_auto_check.setChecked(iris_auto.value)
                iris_auto_check.checkStateChanged.connect(self.onIrisAutoChanged)

                # Add to layout
                self.props_layout.addRow(QLabel("Iris Auto"), iris_auto_check)

            try:
                # Get IRCutFilterEnable property
                ircut = self.grabber.device_property_map.find_boolean(ic4.PropId.IR_CUT_FILTER_ENABLE)

                # Create checkbox
                ircut_check = QCheckBox("Enabled")
                ircut_check.setChecked(ircut.value)
                ircut_check.checkStateChanged.connect(self.onIrcutChanged)

                # Add to layout
                self.props_layout.addRow(QLabel("IR-Cut Filter"), ircut_check)
            except:
                pass

            try:
                # Get ZoomMoveCompleted property
                self.zoom_move_completed = self.grabber.device_property_map.find_integer(ic4.PropId.EVENT_ZOOM_MOVE_COMPLETED)
                # ZoomMoveCompleted indicates when the zoom motor movement is done, so register a notification handler
                # The notification is run on a separate thread, which should not interact with UI elements
                # Therefore, post an event to the main thread
                self.zoom_move_completed.event_add_notification(lambda p: QApplication.postEvent(self, QEvent(ZOOM_MOVE_COMPLETED_EVENT)))

                # Enable generation of ZoomMoveCompleted events
                self.grabber.device_property_map.set_value(ic4.PropId.EVENT_SELECTOR, "ZoomMoveCompleted")
                self.grabber.device_property_map.set_value(ic4.PropId.EVENT_NOTIFICATION, "On")

                create_event_log = True
            except:
                self.zoom_move_completed = None

            try:
                # Get FocusMoveCompleted property
                self.focus_move_completed = self.grabber.device_property_map.find_integer(ic4.PropId.EVENT_FOCUS_MOVE_COMPLETED)
                # FocusMoveCompleted indicates when the focus motor movement is done, so register a notification handler
                # The notification is run on a separate thread, which should not interact with UI elements
                # Therefore, post an event to the main thread
                self.focus_move_completed.event_add_notification(lambda p: QApplication.postEvent(self, QEvent(FOCUS_MOVE_COMPLETED_EVENT)))

                # Enable generation of FocusMoveCompleted events
                self.grabber.device_property_map.set_value(ic4.PropId.EVENT_SELECTOR, "FocusMoveCompleted")
                self.grabber.device_property_map.set_value(ic4.PropId.EVENT_NOTIFICATION, "On")

                create_event_log = True
            except:
                self.focus_move_completed = None

            if create_event_log:
                # Create event log if any property was found that can log useful information
                self.event_log = QPlainTextEdit()
                self.event_log.setReadOnly(True)
                self.event_log.setMinimumHeight(500)

                self.props_layout.addRow(QLabel("Events"), self.event_log)
            else:
                self.event_log = None

    def onDeviceLost(self):
        # The opened camera got disconnected!
        QMessageBox.warning(self, "", f"The video capture device is lost!", QMessageBox.StandardButton.Ok)

        # Clear out controls generated for the previous device
        while self.props_layout.count() > 0:
            self.props_layout.removeRow(0)

        # Close device
        self.grabber.device_close()