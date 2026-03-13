

import rlp.gui as RlpGui
from rlp.gui.expandable import GUI_ExpandableWidget


_PANELS = []

class ControlsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.label = RlpGui.GUI_Label(self, "Controls")

