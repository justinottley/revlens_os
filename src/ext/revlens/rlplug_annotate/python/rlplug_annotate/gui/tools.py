

from rlp import QtGui
import rlp.gui as RlpGui

from rlp.gui.buttongroup import GUI_ButtonGroup

import rlplug_annotate

class BrushToolSettings(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.setOpacity(0.8)

        self.modeLayout = RlpGui.GUI_HLayout(self)
        self.modeLabel = RlpGui.GUI_Label(self, 'Mode:')
        self.modeSel = GUI_ButtonGroup(self, GUI_ButtonGroup.MODE_HORIZONTAL)
        self.modeSel.addButton('View').setMetadata('val', 0)
        self.modeSel.addButton('Edit').setMetadata('val', 1)
        self.modeSel.selectButton('View')
        self.modeSel.buttonPressed.connect(self.onModeChanged)

        self.modeLayout.addItem(self.modeLabel, 10)
        self.modeLayout.addItem(self.modeSel, 5)


        self.sizeLabel = RlpGui.GUI_Label(self, 'Size:')
        self.brushSizeSlider = RlpGui.GUI_Slider(self, RlpGui.GUI_Slider.O_SL_HORIZONTAL, 200)
        self.brushSizeSlider.setBgOpacity(0)

        self.sizeLayout = RlpGui.GUI_HLayout(self)
        self.sizeLayout.addItem(self.sizeLabel, 10)
        self.sizeLayout.addItem(self.brushSizeSlider, 0)

        self.layout = RlpGui.GUI_HLayout(self)
        self.layout.addItem(self.modeLayout, 0)
        self.layout.addSpacer(int(self.modeSel.buttonWidth() + 10))
        self.layout.addItem(self.sizeLayout, 0)

        self.drawController.brushInfoChanged.connect(self.onBrushInfoChanged)

        self.onParentSizeChangedItem(parent.width(), parent.height())


    @property
    def drawController(self):
        return rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ

    def onParentSizeChangedItem(self, width, height):

        self.setWidth(500)
        self.setHeight(50)
        self.setPos(0, height - 50)


    def onModeChanged(self, md):

        settings = {
            'mode': md['val']
        }
        self.drawController.setBrushSettings(settings, "", True)


    def onBrushInfoChanged(self, md):
        if 'mode' not in md:
            return

        modeVal = md['mode']
        if modeVal == 0:
            self.modeSel.selectButton('View')
        else:
            self.modeSel.selectButton('Edit')


    def paintItem(self, painter):

        # p = QtGui.QPen(QtGui.QColor(160, 60, 60))
        b = QtGui.QBrush(QtGui.QColor(40, 40, 40))

        # painter.setPen(p)
        painter.setBrush(b)
        # painter.setOpacity(0.8)
        painter.drawRect(self.boundingRect())