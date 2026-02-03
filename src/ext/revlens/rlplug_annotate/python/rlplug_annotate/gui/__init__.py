

import rlp.gui as RlpGui
import rlp.util as RlpUtil


ag = RlpUtil.APPGLOBALS.globals()
ICON_SIZE = 20
if ag['app.platform'] == 'ios':
    ICON_SIZE = 25

class PaintBrushButton(RlpGui.GUI_IconButton):

    def __init__(self, parent):
        RlpGui.GUI_IconButton.__init__(self, ':/Freepik/White/pencil.png', parent, ICON_SIZE - 5, 4)

        self.setToolTipText('Paint Brush')

        self.category = 'deevad'
        self.name = 'pen-note'


class EraserBrushButton(RlpGui.GUI_IconButton):
    def __init__(self, parent):
        RlpGui.GUI_IconButton.__init__(self, ':/Freepik/White/rubber.png', parent, ICON_SIZE - 5, 4)
        self.setToolTipText('Eraser')

        self.category = 'classic'
        self.name = 'ink_eraser'

