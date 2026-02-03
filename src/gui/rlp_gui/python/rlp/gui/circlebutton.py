
from rlp import QtCore, QtGui
import rlp.gui as RlpGui

class GUI_CircleButton(RlpGui.GUI_ItemBase):

    SIZE = 16

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.toggled = False

        self.toggleChanged = QtCore.PY_Signal(self)

        self.setWidth(self.SIZE)
        self.setHeight(self.SIZE)

    def mousePressEventItem(self, evt):
        self.toggled = not self.toggled

        self.toggleChanged.emit({'val': self.toggled})
        self.update()


    def paintItem(self, painter):

        # p1 = QtGui.QPen(QtGui.QColor(180, 0, 0))
        # painter.setPen(p1)
        # painter.drawRect(self.boundingRect())

        p = QtGui.QPen(QtGui.QColor(10, 10, 10))
        p.setWidth(2)

        b = QtGui.QBrush(QtGui.QColor(200, 200, 200))

        painter.setPen(p)
        painter.setBrush(b)
        hs = self.SIZE  / 2
        painter.drawCircle(hs, hs, hs - 1)

        if self.toggled:
            b = QtGui.QBrush(QtGui.QColor(10, 10, 10))
            painter.setBrush(b)
            painter.drawCircle(hs, hs, hs - 4)