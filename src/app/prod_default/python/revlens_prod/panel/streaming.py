

import rlp.gui as RlpGui

import RlpProdGUImodule as revlens_prod

_PANELS = []

'''
class StreamingPanel(RlpGui.GUI_ItemBase):
    
    _PANELS = []

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)
        StreamingPanel._PANELS.append(self)

        self.w = rlplug_qtbuiltin.GUI_StreamingPane(self)
        self.w.onParentSizeChanged(parent.width(), parent.height())

    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)
    
'''


def create(parent):
    print('streaming create!!!!')
    panel = revlens_prod.GUI_StreamingPane(parent)

    global _PANELS
    _PANELS.append(panel)

    return panel

    # return StreamingPanel(parent)
