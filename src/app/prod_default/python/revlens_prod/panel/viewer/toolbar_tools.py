
from rlp import QtCore, QtGui
import rlp.gui as RlpGui

class ViewerToolsToolbar(RlpGui.GUI_ItemBase):

    TB_HEIGHT = 35

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.layout = RlpGui.GUI_HLayout(self)
        self.layout.setSpacing(5)

        self.volButton = RlpGui.GUI_ToggleSvgButton(self.layout, ':misc/mute.svg', ':misc/volume.svg', 20)
        self.layout.addItem(self.volButton, 7)

        self.setWidth(parent.width())
        self.setHeight(self.TB_HEIGHT)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(self.TB_HEIGHT)


    def paintItem(self, painter):

        b = QtGui.QBrush(QtGui.QColor(40, 40, 40))

        painter.setBrush(b)
        painter.drawRect(self.boundingRect())


def createToolbar(viewer):
    return ViewerToolsToolbar(viewer)