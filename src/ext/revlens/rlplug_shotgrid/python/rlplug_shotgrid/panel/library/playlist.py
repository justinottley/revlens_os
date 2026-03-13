
import rlp.gui as RlpGui
from rlp.gui.buttongroup import GUI_ButtonGroup

from revlens_prod.panel.library.review.main import PlaylistMediaWidget

class ShotgridPlaylistGroup(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        # self.playlist = PlaylistMediaWidget(self, 'Playlist')

        # self.navGroup = GUI_ButtonGroup(self)
        # self.navGroup.addButton('Playlist', '', view=self.playlist).setVisible(True)

        # self.navGroup.selectButton('Playlist')

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)