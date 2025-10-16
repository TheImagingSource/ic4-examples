from PySide6.QtWidgets import (
    QApplication, QWidget, QHBoxLayout, QGroupBox, QSlider, QLabel
)
from PySide6.QtCore import Qt, Signal


class SliderControlInt(QWidget):
    # Custom signal to notify parent of value change
    value_changed = Signal(int)

    def __init__(self, title: str, def_value: int, min_value: int, max_value: int, parent=None):
        super().__init__(parent)

        self.group_box = QGroupBox(title)
        self.slider = QSlider(Qt.Horizontal)
        self.label = QLabel()

        self.slider.setRange(min_value, max_value)
        self.slider.setValue(def_value)
        self.label.setText(f"{def_value:3}")

        self.slider.valueChanged.connect(self._on_slider_value_changed)

        group_layout = QHBoxLayout()
        group_layout.addWidget(self.slider)
        group_layout.addWidget(self.label)
        self.group_box.setLayout(group_layout)

        main_layout = QHBoxLayout(self)
        main_layout.addWidget(self.group_box)
        self.setLayout(main_layout)

    def _on_slider_value_changed(self, value: int):
        self.label.setText(f"{value:3}")
        self.value_changed.emit(value)


# Example usage
# if __name__ == "__main__":
#     import sys

#     class TestWindow(QWidget):
#         def __init__(self):
#             super().__init__()
#             self.setWindowTitle("Slider Control Test")

#             self.slider_control = SliderControlInt("My Slider", 0, 180)
#             self.slider_control.value_changed.connect(self.on_slider_changed)

#             layout = QVBoxLayout(self)
#             layout.addWidget(self.slider_control)

#         def on_slider_changed(self, value: int):
#             print(f"Parent received value: {value}")

#     app = QApplication(sys.argv)
#     window = TestWindow()
#     window.show()
#     sys.exit(app.exec())
