
import rlp.gui as RlpGui

from revlens_prod.widgets.stream import StreamView


class TabletStreamPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.streamView = StreamView(self)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)

        self.streamView.setWidth(width - 20)



def create(parent):
    return TabletStreamPanel(parent)