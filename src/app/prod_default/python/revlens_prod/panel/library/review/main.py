

from rlp import QtGui

import rlp.core as RlpCore
import rlp.gui as RlpGui

from rlp.gui.buttongroup import GUI_ButtonGroup

import RlpProdGUImodule as RlpProd

from .reviewlist import ReviewlistView


from revlens_prod.prod_data import ShowData


class ReviewlistMediaWidget(RlpGui.GUI_ItemBase):

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


class ReviewlistGroup(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.inbox = ReviewlistMediaWidget(self, 'Inbox')
        self.inbox.hideItem()

        self.playlist = ReviewlistMediaWidget(self, 'Playlist')
        self.playlist.hideItem()


        self.navGroup = GUI_ButtonGroup(self)
        self.navGroup.buttonPressed.connect(self.onNavItemRequested)
        self.navGroup.addButton('Inbox', '', view=self.inbox)
        self.navGroup.addButton('Playlist', '', view=self.playlist).setVisible(False)

        self.navGroup.selectButton('Inbox')

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onNavItemRequested(self, md):
        pass


    def onReviewlistDataReady(self, reviewlistData):

        # print('GOT REVIEWLIST MEDIA')
        # import pprint
        # pprint.pprint(reviewlistData)

        # import pprint
        # print(pprint.pformat(reviewlistData))


        if not reviewlistData:
            return

        if 'playlist' not in reviewlistData:
            return

        plModelData = RlpCore.CoreDs_ModelDataItem.makeItem()

        cbMediaDataFormatter = ShowData.instance().formatter('code')
        for entry in reviewlistData['playlist']:
            name, md = cbMediaDataFormatter(entry)

            row = RlpCore.CoreDs_ModelDataItem.makeItem()
            # for modelKey, modelVal in entry.items():
            #     row.insert(modelKey, modelVal)

            dataEntry = {
                'name': name,
                'artist': entry.get('user', {}).get('name', ''),
                'description': entry['description'],
                'sg_path_to_frames': entry['sg_path_to_frames'],
                'frame_count': entry['frame_count'],
                'image_path': entry.get('thumbnail.path', ''),
                'metadata': md
            }

            for dataKey, dataVal in dataEntry.items():
                row.insert(dataKey, dataVal)

            row.setMimeType('__row__', 'text/json')
            row.setMimeTypeKey('__row__')
            row.setMimeData({
                'typeMap': {
                    'text/filename': 'sg_path_to_frames'
                },
                'data': dataEntry
            })

            plModelData.append(row)


        iModelData = RlpCore.CoreDs_ModelDataItem.makeItem()
        for entry in reviewlistData.get('inbox', []):
            name, md = cbMediaDataFormatter(entry)

            row = RlpCore.CoreDs_ModelDataItem.makeItem()
            dataEntry = {
                'name': name,
                'artist': entry.get('user', {}).get('name', ''),
                'description': entry['description'],
                'sg_path_to_frames': entry['sg_path_to_frames'],
                'frame_count': entry['frame_count'],
                'image_path': entry.get('thumbnail.path', ''),
                'metadata': md
            }

            for dataKey, dataVal in dataEntry.items():
                row.insert(dataKey, dataVal)

            row.setMimeType('__row__', 'text/json')
            row.setMimeTypeKey('__row__')
            row.setMimeData({
                'typeMap': {
                    'text/filename': 'sg_path_to_frames'
                },
                'data': dataEntry
            })

            iModelData.append(row)

        self.playlist.grid.setModelData(plModelData)
        self.inbox.grid.setModelData(iModelData)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)




class ReviewMainWidget(RlpGui.GUI_ItemBase):

    _dataSourceCls = None

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.reviewlistWidget = ReviewlistView(self, self._dataSourceCls)
        self.reviewlistWidget.gridView.selectionChanged.connect(self.onReviewlistSelected)

        self.mediaWidget = ReviewlistGroup(self)

        self.layoutSa = RlpGui.GUI_ScrollArea(self)
        self.layoutSa.setOutlined(True)
        self.layoutSa.setBgColour(RlpGui.GUI_Style.PaneBg)

        self.layout = RlpGui.GUI_HLayout(self.layoutSa.content())

        self.layout.addItem(self.reviewlistWidget, 10)
        self.layout.addSpacer(10)
        self.layout.addItem(self.mediaWidget, 10)


        self.onParentSizeChangedItem(parent.width(), parent.height())


    @classmethod
    def setDataSource(cls, dataSourceCls):
        print('Got Data Source: {}'.format(dataSourceCls))
        cls._dataSourceCls = dataSourceCls


    def onReviewlistSelected(self, selMode, selInfo):

        selInfo = selInfo.toPy()
        reviewName = selInfo[0]['name']
        playlistMediaButton = self.mediaWidget.navGroup.buttons[1]
        playlistMediaButton.setText('{} '.format(reviewName))
        playlistMediaButton.setVisible(True)

        self.reviewlistWidget.dataSource.requestPlaylistContents(
            self.mediaWidget.onReviewlistDataReady,
            selInfo[0]['full_path']
        )


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)

        self.reviewlistWidget.setWidth(self.reviewlistWidget.gridView.colModel().colWidth() + 10)

        self.layout.onItemSizeChanged()
        self.layout.setWidth(width)
        self.layout.setHeight(height)
