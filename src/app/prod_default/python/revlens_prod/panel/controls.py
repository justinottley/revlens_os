

import rlp.gui as RlpGui

_PANELS = []

class ControlsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.label = RlpGui.GUI_Label(self, "Controls")


def create(parent):
    panel = ControlsPanel(parent)
    _PANELS.append(panel)
    return panel