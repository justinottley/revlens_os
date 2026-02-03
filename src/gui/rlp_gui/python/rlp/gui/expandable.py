
from rlp import QtCore
from rlp import QtGui
import rlp.gui as RlpGui

class GUI_ExpandableWidget(RlpGui.GUI_ItemBase):

    EXPAND_HEIGHT = 120

    def __init__(self, parent, label,
                 expandIcon=':feather/plus-circle.svg',
                 collapseIcon=':feather/minus-circle.svg',
                 iconSize=30,
                 baseHeight=60,
                 labelXPos=50,
                 labelYPos=20,
                 buttonYPos=15,
                 **kwargs):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._destructed = False

        self._baseHeight = baseHeight

        self._labelXPos = labelXPos
        self._labelYPos = labelYPos

        self.toggleChanged = QtCore.PY_Signal(self)

        self._toggled = False

        self.label = RlpGui.GUI_Label(self, label)
        self.label.setPos(labelXPos, self._labelYPos)

        self.toggle_expand_button = RlpGui.GUI_SvgButton(
            expandIcon, self, iconSize
        )
        self.toggle_expand_button.setPos(10, buttonYPos)
        self.toggle_expand_button.icon().setBgColour(QtGui.QColor(100, 100, 100))
        self.toggle_expand_button.buttonPressed.connect(self.onToggleExpandRequested)

        # self.toggle_expand_button.setToolTipText("Show Room Details")

        self.toggle_collapse_button = RlpGui.GUI_SvgButton(
            collapseIcon, self, iconSize
        )
        self.toggle_collapse_button.hideItem()
        self.toggle_collapse_button.setPos(10, buttonYPos)
        self.toggle_collapse_button.icon().setBgColour(QtGui.QColor(100, 100, 100))
        self.toggle_collapse_button.buttonPressed.connect(self.onToggleCollapseRequested)
        # self.toggle_collapse_button.setToolTipText("Hide Room Details")

        self.slayout = RlpGui.GUI_VLayout(self)
        self.slayout.setPos(0, 50)
        self.slayout.setVisible(False)

        self.setWidth(parent.width())
        self.setHeight(self.baseHeight)

        self.onParentSizeChangedItem(self.width(), self.height())


    @property
    def widgetHeight(self):
        if self._toggled:
            return self.EXPAND_HEIGHT

        return self.baseHeight

    @property
    def baseHeight(self):
        return self._baseHeight

    @property
    def expandHeight(self):
        return self.EXPAND_HEIGHT

    def destruct(self):
        self._destructed = True


    def hide(self):
        if self._destructed:
            return

        self.setVisible(False)
        self.setHeight(0)


    def show(self):
        if self._destructed:
            return

        # print('show: {} {}'.format(self.label.text(), self._lastHeight))
        self.setVisible(True)
        self.setHeight(self.widgetHeight) # self._lastHeight)


    def onToggleExpandRequested(self, md=None):
        if self._destructed:
            return

        self._toggled = True

        self.toggle_expand_button.setVisible(False)
        self.toggle_collapse_button.setVisible(True)

        self.slayout.setVisible(True)

        self.setHeight(self.expandHeight)

        self.toggleChanged.emit({'val': self._toggled, 'height': self.expandHeight})


    def onToggleCollapseRequested(self, md=None):
        if self._destructed:
            return

        self._toggled = False

        self.toggle_expand_button.setVisible(True)
        self.toggle_collapse_button.setVisible(False)

        self.slayout.setVisible(False)

        self.setHeight(self.baseHeight)

        self.toggleChanged.emit({'val': self._toggled, 'height': self.baseHeight})


    def onParentSizeChangedItem(self, width, height):
        if self._destructed:
            return

        self.setWidth(width - 25)


    def paintItem(self, painter):
        if self._destructed:
            return

        painter.save()

        p = QtGui.QPen()
        p.setColor(QtGui.QColor(0, 0, 0))

        b = QtGui.QBrush(QtGui.QColor(60, 60, 60))

        painter.setPen(p)
        painter.setBrush(b)

        painter.drawRect(self.boundingRect())

        painter.restore()

