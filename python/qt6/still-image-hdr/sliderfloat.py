from PySide6.QtWidgets import QWidget, QSlider, QLabel, QHBoxLayout
import PySide6.QtCore as qt


class Sliderfloat(QWidget):
    def __init__(self, min: float, max: float, default: float, step: float, parent: QWidget = None):
        '''
        Create a slider with label, which has 100 steps and a 
        range from min to max.
        '''
        QWidget.__init__(self, parent)
        self.min = min
        self.max = max
        self.default = default

        self.STEPS = (max - min) / step

        self.layout = QHBoxLayout()
        self.label = QLabel(f"{self.default:3.2f}")
        self.slider = QSlider(qt.Qt.Orientation.Horizontal)
        self.slider.setRange(1, self.STEPS + 1)
        self.slider.valueChanged.connect(self.on_change)
        self.slider.setValue(int(self.default / (self.max - self.min) * self.STEPS))
        self.layout.addWidget(self.slider)
        self.layout.addWidget(self.label)

        self.setLayout(self.layout)
        self.setMaximumSize(200, 50)

    def get_value(self) -> float:
        return self.slider.value() * (self.max - self.min) / self.STEPS

    def on_change(self):
        self.label.setText(f"{self.get_value():3.2f}")
