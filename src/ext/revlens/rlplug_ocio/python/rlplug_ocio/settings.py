
import rlp.gui as RlpGui

_PANELS = []

class OCIOSettingsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent.body())

        self.version_label = RlpGui.GUI_Label(self, 'Version: 2.3.0')

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(40)

        self.layout.addItem(self.version_label, 0)

        self.setItemWidth(300)
        self.setItemHeight(300)


def create(parent):
    settings_panel = OCIOSettingsPanel(parent)
    _PANELS.append(settings_panel)
    return settings_panel