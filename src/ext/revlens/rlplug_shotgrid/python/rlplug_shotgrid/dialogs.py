#
# Copyright 2014-2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

from rlp import QtGui
from rlp.Qt import QStr

import rlp.gui as RlpGui
import revlens.gui as RevlensGui


class ShotgridLoadDialog(RlpGui.GUI_ItemBase):

    def __init__(self, floating_pane, media_title, load_path, thumb_path):
        RlpGui.GUI_ItemBase.__init__(self, floating_pane.body())

        self._fpane = floating_pane

        self.title = RlpGui.GUI_Label(self, media_title)
        self.thumb = RlpGui.GUI_IconButton(thumb_path, self, 120, 4)
        self.thumb.setWidth(self._fpane.width())
        self.thumb.setHoverColour(self.thumb.outlineColour())

        self.loader = RevlensGui.GUI_ItemLoader(self)
        self.loader.setLoadPath(load_path)
        self.loader.loadComplete.connect(self.onLoadComplete)
        self.loader.onParentSizeChanged(
            floating_pane.width(),
            floating_pane.height()
        )

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(10)
        self.layout.addItem(self.title, 5)
        self.layout.addSpacer(5)
        self.layout.addItem(self.thumb, 65)
        self.layout.addSpacer(5)
        self.layout.addItem(self.loader, 0)
        self.layout.setWidth(self._fpane.width())
        self.layout.setHeight(self._fpane.height())

        self.loader.onParentSizeChanged(
            floating_pane.width(),
            floating_pane.height()
        )

        self._fpane.setText('Load Media')
        self._fpane.setTextBold(True)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)


    def onLoadComplete(self):
        self._fpane.requestClose()
