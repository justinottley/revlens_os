
import os

import rlp.core as RlpCore
import rlp.gui as RlpGui
import rlp.util as RlpUtil

from rlp.core.net.websocket import RlpClient

import revlens

from revlens_prod.prod_data import ShowData


class ReviewlistView(RlpGui.GUI_ItemBase):

    def __init__(self, parent, dataSourceCls):
        RlpGui.GUI_ItemBase.__init__(self, parent)


        self.project_entity = None
        ag = RlpUtil.get_app_globals()

        if 'project.info' in ag:
            self.project_entity = {
                'type': 'Project',
                'id': ag['project.info']['id'],
                'code': ag['project.info']['code']
            }


        self.gridView = RlpGui.GUI_GRID_View(self, None)
        self.gridView.scrollArea().setHScrollBarVisible(False)
        self.gridView.setPos(10, 0)
        self.gridView.selectionChanged.connect(self.onReviewlistSelected)
        self.gridView.cellWidgetEvent.connect(self.onGridCellWidgetEvent)

        self.refreshButton = RlpGui.GUI_SvgButton(':feather/refresh-cw.svg', self, 20)
        self.refreshButton.setToolTipText('Reload Reviewlists')
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

        colModel = self.gridView.colModel()
        colModel.addFormatter('str', self.fmt)
        colModel.addCol(
            {
                "col_name": "name",
                "display_name": "Name",
                "col_type": "str",
                "metadata": {
                    'expander_visible': True,
                    'widgets': {
                        'hover_button': {
                            'path': ':feather/external-link.svg',
                            'size': 20,
                            'tooltip': 'Load'
                        }
                    }
                },
                "width": 350 # parent.width() - 10
            },
            -1
        )

        colModel.updateCols()
        self.gridView.updateHeader()

        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)

        self.onRefreshRequested()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    @property
    def edbc(self):
        return RlpClient.instance()

    @property
    def dataSource(self):
        return ShowData.instance().dataSource('playlist')


    def onRefreshRequested(self, md=None):

        def _reviewlistDataReady(modelData):

            self.gridView.setModelData(modelData)
            self.gridView.updateRows(True)

            self.statusText.setText('')


        self.dataSource.init(_reviewlistDataReady, force=True)


    def onFilterChanged(self, text):
        self.gridView.expandRowRecursive()
        self.gridView.setFilterString(text, ['name'])


    def onGridCellWidgetEvent(self, evtName, evtInfo):

        print('{} {}'.format(evtName, evtInfo))

        if evtName == 'hover_button.button_pressed':
            print(evtInfo)
            self.selReview = evtInfo['full_path']
            self.onLoadRequested()


    def onReviewlistSelected(self, selMode, selInfo):

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

            cbMediaDataFormatter = ShowData.instance().formatter('code')
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
                track.setMetadata(plMetadataKeyName, value)



        self.dataSource.requestPlaylistContents(_playlistVersionsReady, self.selReview)


    def onNoticeEvent(self, evtName, evtInfo):

        if evtName == 'media.request_start_review_with_reviewlist': # coming from start review

            reviewlistPath = evtInfo['reviewlist_path']
            print('Start Review with Reviewlist: {}'.format(reviewlistPath))

            revlens.CNTRL.session().clear('')

            # needed for remote cursors and drawing
            revlens.CNTRL.session().addTrackByType('Annotation') 

            print('LOADING: {}'.format(reviewlistPath))

            self.selReview = reviewlistPath
            self.onLoadRequested()



    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height - 100)
        
        self.gridView.setWidth(width - 20)
        self.gridView.setHeight(height - 100)


class ShotgunReviewlistDataSource:

    PATH_ROOTS = [
        'Animation',
        'Braintrust',
        'Bloopers',
        'Characters',
        'Crowds',
        'Editorial',
        'Effects',
        'Environments',
        'Env Assets',
        'Layout',
        'LFIN',
        'Lighting',
        'Marketing',
        'Misc',
        'Stereo',
        'Story',
        'Sweatbox',
        'Technical Animation',
        'Testing',
        'Training',
        'Visdev'
    ]

    def __init__(self, reviewlistView):
        self.reviewlistView = reviewlistView


    @property
    def edbc(self):
        return RlpClient.instance()


    def init(self, force=False):

        root = RlpCore.CoreDs_ModelDataItem.makeItem()
        for pathEntry in self.PATH_ROOTS:

            pi = RlpCore.CoreDs_ModelDataItem.makeItem()
            pi.setPopulatePyCallback(self.populatePath)

            pi.insert('path', pathEntry)

            ci = RlpCore.CoreDs_ModelDataItem.makeItem()
            pi.insertItem('__children__', ci)

            root.append(pi)

        # psroot = RlpCore.CoreDs_ModelDataItem.makeItem()
        # psroot.insert('__children__')
        self.reviewlistView.gridView.setModelData(root)
        self.reviewlistView.gridView.updateRows(True)


    def populatePath(self, item):

        data = item.values()

        pathStart = '/{}/'.format(data['path'])
        filters = [
            ['project', 'is', self.project_entity],
            ['sg_status_list', 'is', 'act'],
            ['sg_review_path', 'not_contains', 'Archive'],
            ['sg_review_path', 'starts_with', pathStart]
        ]

        return_fields = ['sg_review_path']
        order_by = [{'field_name': 'updated_at', 'direction': 'asc'}]

        def _results_ready(result):

            if not result:
                return

            ri = RlpCore.CoreDs_ModelDataItem.makeItem()
            for playlistEntry in sorted(result, key=lambda x:x['sg_review_path']):

                playlistFullPath = playlistEntry['sg_review_path']
                pi = RlpCore.CoreDs_ModelDataItem.makeItem()

                playlistName = playlistFullPath.replace(pathStart, '')
                pi.insert('id', playlistEntry['id'])
                pi.insert('path', playlistName)
                pi.insert('parent', data['path'])

                ri.append(pi)

            item.insertItem('__children__', ri)
            item.setPopulated()

        self.edbc.sg_find(_results_ready, 'Playlist',
            filters, return_fields, order=order_by).run()


    def requestPlaylistContents(self, callback):

        self.edbc.ioclient('sg').call(callback,
            'playlist.get_playlist_versions',
            self.reviewlistView.project_entity,
            playlist_path=self.reviewlistView.selReview
        ).run()

