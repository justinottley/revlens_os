

from PySide6 import QtCore, QtWidgets

import rlp.gui as RlpGui

from RlpGuiSBKPYMODULE import GUI_QWidgetWrapper

class GUI_QWidgetPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.qw = GUI_QWidgetWrapper(self)
        self.qw.setWidth(parent.width())
        self.qw.setHeight(parent.height())

        self.widget = QtWidgets.QWidget()

        self.qw.setPySideWidget(self.widget)


    def mousePressEventItem(self, event):
        self.qw.forwardMouseEventToWidget(event)

    def mouseReleaseEventItem(self, event):
        self.qw.forwardMouseEventToWidget(event)

    def mouseMoveEventItem(self, event):
        self.qw.forwardMouseEventToWidget(event)

    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)