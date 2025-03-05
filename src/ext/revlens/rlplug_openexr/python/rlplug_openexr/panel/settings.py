

from rlp.Qt import QStr
import rlp.gui as RlpGui

import revlens


_PANELS = []

class EXRSettingsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent.body())

        exr_plugin = revlens.CNTRL.getPluginManager().getMediaPlugin('OpenEXR')

        self.version_label = RlpGui.GUI_Label(self, 'Version: 3.2.1')

        self.read_threads_label = RlpGui.GUI_Label(self, 'Reader Threads: {}'.format(
            exr_plugin.getReaderThreads()))
        
        self.imf_threads_label = RlpGui.GUI_Label(self, 'IMF Threads: {}'.format(
            exr_plugin.getImfThreads()))

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(40)
        self.layout.addItem(self.version_label, 10)
        self.layout.addSpacer(10)
        self.layout.addItem(self.read_threads_label, 10)
        self.layout.addItem(self.imf_threads_label, 10)

        self.setItemWidth(parent.width())
        self.setItemHeight(parent.height())
        

def create(parent):
    settings_panel = EXRSettingsPanel(parent)
    _PANELS.append(settings_panel)
    return settings_panel