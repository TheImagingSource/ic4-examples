
from PySide6.QtCore import QEvent, Qt
from PySide6.QtGui import QIntValidator
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

    def zoomSliderChanged(self, val):
        # The zoom slider changed: Set new value and update text
        self.zoom.value = val
        self.zoom_edit.setText(str(val))

    def zoomEditDone(self):
        # If the zoom text was changed, set new value and update slider
        val = int(self.zoom_edit.text())
        if self.zoom.value != val:
            self.zoom.value = val
            self.zoom_slider.setValue(val)

    def focusSliderChanged(self, val):
        # The focus slider changed: Set new value
        self.focus.value = val
        # No need to update edit box, focus change notification will do that

    def focusEditDone(self):
        # If the focus text was changed, set the new value
        val = int(self.focus_edit.text())
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
        self.focus_auto.execute()
        if not self.focus_auto.is_done:
            self.run_auto_focus.setEnabled(False)

    def onUpdateFocusAuto(self):
        # This function is called when the notification event for the FocusAuto feature was raised
        # If the command is completed, re-enable the button and log
        if self.focus_auto.is_done and not self.run_auto_focus.isEnabled():
            self.run_auto_focus.setEnabled(True)
            val = self.grabber.device_property_map.find_integer(ic4.PropId.FOCUS).value
            self.event_log.appendPlainText(f"FocusAuto completed (at {val})")

    def onIrcutChanged(self, state: Qt.CheckState):
        # Change movable IR-Cut filter state
        self.ircut.value = (state == Qt.CheckState.Checked)

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
                # Get FocusAuto property
                self.focus_auto = self.grabber.device_property_map.find_command(ic4.PropId.FOCUS_AUTO)
                # FocusAuto can indicate when it is done, so register a notification handler
                # The notification is run on a separate thread, which should not interact with UI elements
                # Therefore, post an event to the main thread
                self.focus_auto.event_add_notification(lambda p: QApplication.postEvent(self, QEvent(UPDATE_FOCUS_AUTO_EVENT)))

                # Create FocusAuto button
                self.run_auto_focus = QPushButton("Execute")
                self.run_auto_focus.pressed.connect(self.onFocusAuto)

                # Add to layout
                self.props_layout.addRow(QLabel("Auto-Focus"), self.run_auto_focus)                
                create_event_log = True
            except:
                self.focus_auto = None

            try:
                # Get IRCutFilterEnable property
                self.ircut = self.grabber.device_property_map.find_boolean(ic4.PropId.IR_CUT_FILTER_ENABLE)

                # Create checkbox
                ircut_check = QCheckBox("Enabled")
                ircut_check.setChecked(self.ircut.value)
                ircut_check.checkStateChanged.connect(self.onIrcutChanged)

                # Add to layout
                self.props_layout.addRow(QLabel("IR-Cut Filter"), ircut_check)
            except:
                self.ircut = None

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