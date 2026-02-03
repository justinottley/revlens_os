
import rlp.gui as RlpGui

import revlens

from revlens_prod.panel.viewer.toolbar_display import ZoomPanWidget

class TabletDisplayPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.layout = RlpGui.GUI_VLayout(self)
        self.zoomPan = None

        revlens.CNTRL.getVideoManager().displayRegistered.connect(self.onDisplayRegistered)

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onDisplayRegistered(self, display):

        if self.zoomPan is None:
            self.zoomPan = ZoomPanWidget(self, display)
            self.layout.addItem(self.zoomPan, 5)



    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)

        if self.zoomPan:
            self.zoomPan.setWidth(width)
            self.zoomPan.resetButton.setPos(width - 40, 12)



def create(parent):
    return TabletDisplayPanel(parent)