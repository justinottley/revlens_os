
import os

import rlp.core as RlpCore
import rlp.gui as RlpGui
import rlp.util as RlpUtil

from rlp.core.net.websocket import RlpClient

import revlens


class PlaylistView(RlpGui.GUI_ItemBase):

    PLAYLIST_LABEL = 'Playlist'
    IOCLIENT_NAME = 'sg'

    def __init__(self, parent, enableHoverLoad=True):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.project_entity = None

        projectInfo = RlpUtil.APPGLOBALS.value('project.info')
        if projectInfo:
            self.project_entity = {
                'type': 'Project',
                'id': projectInfo['id'],
                'code': projectInfo['code']
            }

        self.gridView = RlpGui.GUI_GRID_View(self, None)
        self.gridView.scrollArea().setHScrollBarVisible(False)
        self.gridView.setPos(10, 0)
        self.gridView.selectionChanged.connect(self.onPlaylistSelected)
        self.gridView.cellWidgetEvent.connect(self.onGridCellWidgetEvent)

        self.refreshButton = RlpGui.GUI_SvgButton(':feather/refresh-cw.svg', self, 20)
        self.refreshButton.setToolTipText('Reload {}'.format(self.PLAYLIST_LABEL))
        self.refreshButton.buttonPressed.connect(self.onRefreshRequested)

        self.filter_textarea = RlpGui.GUI_TextEdit(self.gridView.toolbar(), 230, 30)
        self.filter_textarea.textChanged.connect(self.onFilterChanged)
        self.filter_textarea.setTempHintText('Filter:')
        self.gridView.toolbar().layout().addItemAtStart(self.filter_textarea, 0)

        self.gridView.toolbar().layout().addItemAtStart(self.refreshButton, 4)

        # self.loadButton = RlpGui.GUI_IconButton('', self, 20, 4)
        # self.loadButton.setText('Load')
        # self.loadButton.setOutlined(True)
        # self.loadButton.buttonPressed.connect(self.onLoadRequested)

        self.statusText = RlpGui.GUI_Label(self, '')

        # self.gridView.toolbar().layout().addItem(self.loadButton, 0)
        # self.gridView.toolbar().layout().addSpacer(5)
        self.gridView.toolbar().layout().addItem(self.statusText, 5)

        # self.gridView.setAutoResize(False)

        self.fmt = RlpGui.GUI_GRID_CellFormatterFolder()

        widgetInfo = {}
        if enableHoverLoad:
            widgetInfo = {
                'hover_button': {
                    'path': ':feather/external-link.svg',
                    'size': 20,
                    'tooltip': 'Load'
                }
            }

        colModel = self.gridView.colModel()
        colModel.addFormatter('str', self.fmt)
        colModel.addCol(
            {
                "col_name": "name",
                "display_name": "Name",
                "col_type": "str",
                "metadata": {
                    'expander_visible': True,
                    'widgets': widgetInfo
                },
                "width": 350 # parent.width() - 10
            },
            -1
        )

        colModel.updateCols()
        self.gridView.updateHeader()

        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)

        if self.edbc.ioclient(self.IOCLIENT_NAME).ready:
            self.onRefreshRequested()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    @property
    def edbc(self):
        return RlpClient.instance()

    @property
    def dataSource(self):
        showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
        return showDataObj.dataSource('playlist')


    def onRefreshRequested(self, md=None):

        def _playlistDataReady(modelData):

            if not modelData:
                return

            self.gridView.setModelData(modelData, '', True, True)
            self.gridView.updateRows(True)

            self.statusText.setText('')

        self.dataSource.init(_playlistDataReady, force=True)


    def onFilterChanged(self, text):
        self.gridView.expandRowRecursive()
        self.gridView.setFilterString(text, ['name'])


    def onGridCellWidgetEvent(self, evtName, evtInfo):

        print('{} {}'.format(evtName, evtInfo))

        if evtName == 'hover_button.button_pressed':
            print(evtInfo)
            self.selReview = evtInfo['full_path']
            self.onLoadRequested()


    def onPlaylistSelected(self, selMode, selInfo):

        if selMode != 'set':
            print('Warning: invalid selection mode: "{}"'.format(selMode))
            return

        # TODO FIXME: python binding
        selInfo = selInfo.toPy()

        self.selReview = selInfo[0]['full_path']

        # self.selReview = md
        # self.selReview['type'] = 'Playlist'
        # self.selReview['code'] = os.path.basename(md['path'])

        # print(self.selReview)


    def onLoadRequested(self, md=None):


        def _playlistVersionsReady(result):

            # print('GOT RESULT')
            # print(result)
            showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
            cbMediaDataFormatter = showDataObj.formatter('code')
            mediaList = []
            for verEntry in result['playlist']:
                mcode, mediaMd = cbMediaDataFormatter(verEntry)
                mediaList.append({
                    'media.name': mcode,
                    'media.frame_count': verEntry['frame_count'],
                    'media.video': verEntry['sg_path_to_frames'],
                    'media.metadata': mediaMd
                })

            # import pprint
            # pprint.pprint(mediaList)

            trackName = os.path.basename(self.selReview)
            track = revlens.CNTRL.getMediaManager().loadMediaList(
                mediaList, -1, trackName, 1, True, True
            )

            for key, value in result.get('metadata', {}).items():
                plMetadataKeyName = 'playlist.{}'.format(key)
                print('Setting: {} = {}'.format(plMetadataKeyName, value))
                track.setMetadataValue(plMetadataKeyName, value)



        self.dataSource.requestPlaylistContents(_playlistVersionsReady, self.selReview)


    def onNoticeEvent(self, evtName, evtInfo):

        if evtName == 'media.request_start_review_with_playlist': # coming from start review

            playlistPath = evtInfo['playlist_path']
            print('Start Review with {}: {}'.format(
                self.PLAYLIST_LABEL, playlistPath))

            revlens.CNTRL.session().clear('')

            # needed for remote cursors and drawing
            revlens.CNTRL.session().addTrackByType('Annotation') 

            print('LOADING: {}'.format(playlistPath))

            self.selReview = playlistPath
            self.onLoadRequested()


        elif evtName == 'ioservice_event':
            if evtInfo['method'] == 'queue_ready':
                self.onRefreshRequested()


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height - 100)
        
        self.gridView.setWidth(width - 20)
        self.gridView.setHeight(height - 100)

