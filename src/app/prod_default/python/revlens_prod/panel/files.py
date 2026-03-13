
import os

import rlp.gui as RlpGui
import revlens.gui as RevlensGui

from revlens.gui.dialogs import LoadDialog

class FilesPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.fileBrowser = RevlensGui.GUI_FileBrowser(self)
        self.fileBrowser.loadRequested.connect(self.onFileLoadRequested)

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onFileLoadRequested(self, loadInfo):
        print('LOAD: {}'.format(loadInfo))
        LoadDialog.create(loadInfo['local_path'])


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)


def create(parent):
    return FilesPanel(parent)