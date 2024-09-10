
from PySide6.QtGui import QPalette, QIcon
from PySide6.QtCore import QFileSelector

def _is_dark_mode() -> bool:
    default_palette = QPalette()
    return default_palette.color(QPalette.WindowText).lightness() > default_palette.color(QPalette.Window).lightness()

class ResourceSelector:
    
    def __init__(self):
        self.file_selector = QFileSelector()

        if _is_dark_mode():
            self.file_selector.setExtraSelectors(["theme_dark"])
        else:
            self.file_selector.setExtraSelectors(["theme_light"])

    def select(self, item: str) -> str:
        return self.file_selector.select(item)
    
    def loadIcon(self, item: str) -> QIcon:
        return QIcon(self.select(item))


