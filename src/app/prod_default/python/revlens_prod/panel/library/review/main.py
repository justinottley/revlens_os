

from rlp import QtGui

import rlp.core as RlpCore
import rlp.gui as RlpGui

from rlp.gui.buttongroup import GUI_ButtonGroup

import RlpProdGUImodule as RlpProd

from .playlist import PlaylistView



class PlaylistMediaWidget(RlpGui.GUI_ItemBase):

    def __init__(self, parent, label):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        # self.label = RlpGui.GUI_Label(self, label)


        self.grid = RlpProd.GUI_PlaylistView(self)
        self.grid.scrollArea().setHScrollBarVisOffset(-1)
        self.grid.toolbar().setVisible(False)
        self.grid.toolbar().setHeight(0)


        # self.grid.setPos(0, 5)
        # self.previewButton = RlpGui.GUI_IconButton('', self, 20, 4)
        # self.previewButton.setText('Preview')
        # self.previewButton.setOutlined(True)
        # self.previewButton.buttonPressed.connect(self.onLoadRequested)

        # self.grid.toolbar().layout().addItemAtStart(self.label, 0)
        # self.grid.toolbar().layout().addItem(self.previewButton, 0)


        colModel = self.grid.colModel()
        colModel.addCol({
            "col_name": "name",
            "display_name": "Name",
            "col_type": "str",
            "metadata": {},
            "width": 140
        }, -1)

        colModel.addCol({
            "col_name": "artist",
            "display_name": "Artist",
            "col_type": "str",
            "metadata": {},
            "width": 140
        }, -1)

        colModel.addCol({
            "col_name": "image_path",
            "display_name": "Thumbnail",
            "col_type": "image_path",
            "metadata": {},
            "width": 80
        }, -1)

        colModel.addCol({
            "col_name": "description",
            "display_name": "Description",
            "col_type": "str",
            "metadata": {},
            "width": 200
        }, -1)

        # colModel.addCol({
        #     "col_name": "frame",
        #     "display_name": "Frame",
        #     "col_type": "str",
        #     "metadata": {},
        #     "width": 100
        # }, -1)

        colModel.updateCols()
        self.grid.updateHeader()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onLoadRequested(self, md):
        print('LOAD')

        selRow = self.grid.fullSelectionValues()
        print(selRow)
        selPath = selRow[0]['sg_path_to_frames']
        print('Loading in preview player: {}'.format(selPath))


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height - 100)

        self.grid.setWidth(width - 20)
        self.grid.setHeight(height - 100)



class ReviewMainWidget(RlpGui.GUI_ItemBase):

    _mediaWidgetCls = None

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.playlistWidget = PlaylistView(self)
        self.playlistWidget.gridView.selectionChanged.connect(self.onPlaylistSelected)

        self.mediaWidget = self._mediaWidgetCls(self)

        self.layoutSa = RlpGui.GUI_ScrollArea(self)
        self.layoutSa.setOutlined(True)
        self.layoutSa.setBgColour(RlpGui.GUI_Style.PaneBg)

        self.layout = RlpGui.GUI_HLayout(self.layoutSa.content())

        self.layout.addItem(self.playlistWidget, 10)
        self.layout.addSpacer(10)
        self.layout.addItem(self.mediaWidget, 10)


        self.onParentSizeChangedItem(parent.width(), parent.height())


    @classmethod
    def setMediaWidget(cls, mediaWidgetCls):
        print('Got Media Widget: {}'.format(mediaWidgetCls))
        cls._mediaWidgetCls = mediaWidgetCls


    def onPlaylistSelected(self, selMode, selInfo):

        selInfo = selInfo.toPy()
        reviewName = selInfo[0]['name']
        playlistMediaButton = self.mediaWidget.navGroup.buttons[1]
        playlistMediaButton.setText('{} '.format(reviewName))
        playlistMediaButton.setVisible(True)

        self.playlistWidget.dataSource.requestPlaylistContents(
            self.mediaWidget.onPlaylistDataReady,
            selInfo[0]['full_path']
        )


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)

        self.playlistWidget.setWidth(self.playlistWidget.gridView.colModel().colWidth() + 10)

        self.layout.onItemSizeChanged()
        self.layout.setWidth(width)
        self.layout.setHeight(height)
