

import pprint

from rlp import QtCore
import rlp.core as RlpCore
import rlp.gui as RlpGui
import rlp.util as RlpUtil

from rlp.core.net.websocket import RlpClient

import revlens

from ...widgets.breadcrumb import BreadcrumbWidget


class MediaThumbItem(RlpGui.GUI_ItemBase):

    _utext = RlpCore.UTIL_Text()
    _thumbf = RlpGui.GUI_ThumbnailItem()

    def __init__(self, media_entry, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.requestLoad = QtCore.PY_Signal(self)

        self._media_entry = media_entry

        if 'thumb_frame_data' in media_entry:
            self.thumb = self._thumbf.fromBase64(
                self,
                media_entry['thumb_frame_data'],
                230
            )
        else:
            self.thumb = self._thumbf.fromPath(
                self,
                ':misc/thumb_image.png',
                230
            )

        self.thumb.setData(media_entry)

        otext = media_entry.get('title', '-')
        ntext = self._utext.clipToWidth(
            otext, 170)

        if ntext != otext:
            ntext += " ..."


        self.thumb.setLabel(ntext)

        self.thumb.hoverEnter.connect(self.on_thumb_hover_enter)
        self.thumb.hoverLeave.connect(self.on_thumb_hover_leave)

        self.load_button = RlpGui.GUI_IconButton(
            '', self.thumb, 20, 4)

        self.load_button.setText("Load..")
        self.load_button.setOutlined(True)  
        self.load_button.setZValue(self.z() + 1)
        self.load_button.buttonPressed.connect(self.onLoadRequested)

        thumb_width = self.thumb.width()
        thumb_height = self.thumb.height()

        self.load_button.setPos(
            (thumb_width / 2) - 30,
            (thumb_height / 2) - 10
        )

        self.load_button.hideItem()

        self.setItemHeight(self.thumb.height() + 30)
        self.setItemWidth(self.thumb.width())


    @property
    def text(self):
        return self._media_entry.get('title', '-')


    def on_thumb_hover_enter(self, payload):
        self._selection = payload
        self.load_button.showItem()


    def on_thumb_hover_leave(self, payload):
        pass
        
        if not self.load_button.inHover():
            self.load_button.hideItem()


    def onLoadRequested(self, md):

        # sel_data = self.get_selection()
        # if not sel_data:
        #     self.LOG.error('nothing selected, cannot load')

        #     # NOTE: EARLY RETURN
        #     return

        self.requestLoad.emit(self._media_entry)



class ThumbView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.requestLoad = QtCore.PY_Signal(self)

        self._mpanel = parent

        self.layout = RlpGui.GUI_FlowLayout(self)
        self.layout.setSpacing(20)

        self._selection = None
        self._active = False

        self._thumbs = {}


    def refresh(self, media_results):

        self.layout.clear()
        self._thumbs = {}

        for media_entry in media_results:
            # if media_entry.get('create_date'):
            #     media_entry['create_date'] = QtCore.QDateTime.fromSecsSinceEpoch(
            #         media_entry['create_date']).toString()


            # print(media_entry)
            media_entry['title'] = media_entry['code']
            thumbId = str(media_entry['id'])
            thumbw = MediaThumbItem(media_entry, self)
            thumbw.requestLoad.connect(self.onLoadRequested)
            self.layout.addItem(thumbw)
            self._thumbs[thumbId] = thumbw

        self.layout.updateItems()


    def forceUpdate(self):
        self.layout.updateItems()


    def get_selection(self):
        return self._selection


    def setFilterString(self, text):

        print('THUMB FILTER {} '.format(text))


        ltext = text.lower()

        for thumb in self._thumbs.values():
            if not text:
                thumb.setVisible(True)

            else:

                if ltext in thumb.text.lower():
                    thumb.setVisible(True)

                else:
                    thumb.setVisible(False)

        self.forceUpdate()



    def onThumbImageReady(self, runId, image):
        if runId in self._thumbs:
            self._thumbs[runId].thumb.setThumbImage(image, 230)
            self._thumbs[runId].thumb.update()


    def onLoadRequested(self, md):

        # pprint.pprint(md)
        self.requestLoad.emit(md)

        # self.requestLoad.emit({
        #     'sequence_id': md['id'],
        #     'code': md['code'],
        #     'type': md['type']
        # })



    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width - 10)
        self.setHeight(height - 100)

        self.layout.onParentSizeChanged(width - 10, height - 100)


class TableView(RlpGui.GUI_ItemBase):

    COLMODEL_SEQ = [
        {   
            "col_name": "_image",
            "display_name": "Thumbnail",
            "col_type": "image_b64",
            "metadata": {},
            "width": 80
        },
        {
            "col_name": "code",
            "display_name": "Name",
            "col_type": "str",
            "metadata": {'selectable': True},
            "width": 120
        },
        {
            "col_name": "description",
            "display_name": "Description",
            "col_type": "str",
            "metadata": {},
            "width": 300
        }
    ]

    COLMODEL_SHOT = COLMODEL_SEQ[:]
    COLMODEL_VERSION = COLMODEL_SEQ[:]
    COLMODEL_VERSION += [
        {
            "col_name": "sg_path_to_frames",
            "display_name": "Path to Media",
            "col_type": "str",
            "metadata": {},
            "width": 300
        },
        {
            "col_name": "frame_count",
            "display_name": "Length",
            "col_type": "int",
            "metadata": {},
            "width": 120
        }
    ]

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.status = QtCore.PY_Signal(self)
        self.requestLoad = QtCore.PY_Signal(self)

        self.entity_type = None

        self.grid_toolbar_init = RlpGui.GUI_GRID_ToolbarInit()
        self.grid_toolbar_init.setPyCallback(self._initGridToolbar)

        self.grid_view = RlpGui.GUI_GRID_View(self, self.grid_toolbar_init) # EntityPanelGridView(self, 'Media')
        self.grid_view.setPos(10, 0)
        self.grid_view.scrollArea().setHScrollBarVisible(False)

        colModel = self.grid_view.colModel()

        for colInfo in self.COLMODEL_SEQ:
            colModel.addCol(colInfo, -1)

        self.grid_view.updateHeader()
        # grid_toolbar = self.grid_view.toolbar()

        self._rowIdxMap = {}

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def _initGridToolbar(self, grid_toolbar):

        self.load_button = RlpGui.GUI_IconButton("", grid_toolbar, 20, 4)
        self.load_button.setText('Load..')
        self.load_button.setOutlined(True)
        self.load_button.buttonPressed.connect(self.onLoadRequested)

        self.statusText = RlpGui.GUI_Label(self, '')

        toolbar_layout = grid_toolbar.layout()
        toolbar_layout.setSpacing(5)
        toolbar_layout.addItem(grid_toolbar.colSelector(), 0)
        toolbar_layout.addItem(self.load_button, 0)
        toolbar_layout.addItem(self.statusText, 5)

        self.status.connect(self.onStatusReceived)


    def refresh(self, mediaType, mediaResults=[]):


        self.grid_view.clearData()

        self._rowIdxMap = {}
        typeMap = {}

        if mediaType == 'Version':
            typeMap['text/filename'] = 'sg_path_to_frames'
    
        elif mediaType == 'Shot':
            typeMap['func/callback'] = 'revlens_prod.cmds.load_shots'

        showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
        cbMediaDataFormatter = showDataObj.formatter('code')

        mediaMapForSort = {}
        mediaListOther = []
        for mediaInfo in mediaResults:

            mediaName, mediaMd = cbMediaDataFormatter(mediaInfo, nameKey='code')
            if 'version' in mediaMd:
                verNum = int(mediaMd['version'])
                mediaMapForSort[verNum] = mediaInfo

            else:
                mediaListOther.append(mediaInfo)

        modelData = RlpCore.CoreDs_ModelDataItem.makeItem()

        idx = 0
        for sortedKey in reversed(sorted(mediaMapForSort.keys())):

            entry = mediaMapForSort[sortedKey]

            self._rowIdxMap[str(entry['id'])] = idx
            idx += 1

            row = RlpCore.CoreDs_ModelDataItem.makeItem()
            for modelKey, modelVal in entry.items():
                row.insert(modelKey, modelVal)

            row.setMimeType('__row__', 'text/json')
            row.setMimeTypeKey('__row__')
            row.setMimeData({
                'typeMap': typeMap,
                'data': entry
            })

            modelData.append(row)


        for entry in mediaListOther:

            self._rowIdxMap[str(entry['id'])] = idx
            idx += 1

            row = RlpCore.CoreDs_ModelDataItem.makeItem()
            for modelKey, modelVal in entry.items():
                row.insert(modelKey, modelVal)

            row.setMimeType('__row__', 'text/json')
            row.setMimeTypeKey('__row__')
            row.setMimeData({
                'typeMap': typeMap,
                'data': entry
            })

            modelData.append(row)


        self.grid_view.setModelData(modelData, '', True, True)
        pass


    def onLoadRequested(self, md):
        self.requestLoad.emit(self.selRow)


        # self.requestLoad.emit({
        #     'sequence_id': self.selRow['id'],
        #     'code': self.selRow['code'],
        #     'type': self.selRow['type'],
        #     ''
        # })


    def onThumbImageReady(self, runId, image):

        if runId not in self._rowIdxMap:
            return

        rowIdx = self._rowIdxMap[runId]
        row = self.grid_view.rowAt(rowIdx)
        if not row:
            print('ERROR NO ROW AT IDX {}'.format(rowIdx))
            return

        cell = row.cell("_image")

        cell.insertValue("image", image)
        cell.insertValue("image.original", image)
        cell.insertValue("value", "")
        cell.update()


    def onStatusReceived(self, info):
        self.statusText.setText(info['msg'])
        self.statusText.update()


    def forceUpdate(self):
        self.grid_view.updateRows(True)


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)
        
        self.grid_view.setWidth(width - 20)
        self.grid_view.setHeight(height - 140)



class SequenceTableView(TableView):

    def __init__(self, parent):
        TableView.__init__(self, parent)

        self.grid_view.selectionChanged.connect(self.onSequenceSelected)

        self.project_entity = {'type': 'Project', 'id': 822}

        self.loadCount = 0
        self.loadIdx = 0

        self.selRow = None

        self.onParentSizeChangedItem(parent.width(), parent.height())


    @property
    def edbc(self):
        return RlpClient.instance()


    def onSequenceSelected(self, selMode, selInfo):
        # print(rowInfo)

        if selMode != 'set':
            return

        selInfo = selInfo.toPy()

        self.selRow = selInfo[0]

        # self.seqSelector.setText(rowInfo['code'])
        # self.seqSelector.menu().setVisible(False)
        # self.seqSelector.update()

        # HACK refresh for right aligned toolbar widgets
        # self.grid_view.onParentSizeChanged(self.grid_view.width() - 1, self.grid_view.height())
        # self.grid_view.onParentSizeChanged(self.grid_view.width() + 1, self.grid_view.height())



class SequenceMediaView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.status = QtCore.PY_Signal(self)


        self.http_client = RlpCore.CoreNet_HttpClient()
        self.http_client.requestReady.connect(self.onThumbRequestReady)

        self.project_entity = None
        projectInfo = RlpUtil.APPGLOBALS.value('project.info')
        if projectInfo:
            self.project_entity = {
                'type': 'Project',
                'id': projectInfo['id'],
                'code': projectInfo['code']
            }

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":feather/refresh-cw.svg",
            self,
            18
        )

        self.thumb_button = RlpGui.GUI_SvgButton(
            ":feather/grid.svg",
            self,
            18
        )

        self.table_button = RlpGui.GUI_SvgButton(
            ":feather/align-justify.svg",
            self,
            22
        )

        self.filter_textarea = RlpGui.GUI_TextEdit(self, 230, 30)
        self.filter_textarea.textChanged.connect(self.onFilterChanged)
        self.filter_textarea.setTempHintText('Filter:')


        self.breadcrumb = BreadcrumbWidget(self)
        self.breadcrumb.setHierarchy([
            {
                'type': 'Sequence',
                'field_link': 'project',
                'return_fields': ['code', 'image'],
                'filters': [
                    ['sg_status_list', 'is', 'ip'],
                    ['sg_sequence_type', 'not_in', ['Test', 'CrowdCycle']]],
                'name_field': 'code',
                'order': [{'field_name': 'code', 'direction': 'asc'}]
            },
            {
                'type': 'Shot',
                'field_link':'sg_sequence',
                'return_fields': ['code', 'image', 'description', 'sg_sequence'],
                'filters':[

                ],
                'name_field': 'code',
                'display_field': 'code',
                'order': [{'field_name': 'code', 'direction': 'asc'}]
            },
            {
                'type': 'Version',
                'field_link': 'entity',
                'return_fields': ['code', 'image', 'description', 'user', 'sg_department', 'sg_path_to_frames', 'frame_count'],
                'name_field': 'code',
                'display_field': 'code',
                'selectable': False,
                'order': [{'field_name': 'updated_at', 'direction': 'desc'}]
            }
        ])
        self.breadcrumb.navItemsReady.connect(self.onBreadcrumbNavItemsReady)

        self.statusText = RlpGui.GUI_Label(self, '')

        self.toolbar_layout = RlpGui.GUI_HLayout(self) # grid_toolbar.layout()
        self.toolbar_layout.setSpacing(5)

        self.toolbar_layout.addItem(self.refresh_button, 5)
        self.toolbar_layout.addDivider(3, 25, 2)
        self.toolbar_layout.addItem(self.table_button, 3)
        self.toolbar_layout.addItem(self.thumb_button, 5)
        self.toolbar_layout.addDivider(3, 25, 3)
        self.toolbar_layout.addItem(self.filter_textarea, 0)
        self.toolbar_layout.addItem(self.breadcrumb, 3)
        self.toolbar_layout.addItem(self.statusText, 5)

        self.thumbView = ThumbView(self)
        self.thumbView.requestLoad.connect(self.onLoadRequested)
        self.thumbView.setPos(0, 40)
        self.thumbView.setVisible(False)

        self.tableView = SequenceTableView(self)
        self.tableView.requestLoad.connect(self.onLoadRequested)
        self.tableView.grid_view.cellWidgetEvent.connect(self.onCellWidgetEvent)
        self.tableView.setPos(0, 40)

        self.refresh_button.buttonPressed.connect(self.initSequences)
        self.table_button.buttonPressed.connect(self.onTableViewRequested)
        self.thumb_button.buttonPressed.connect(self.onThumbViewRequested)

        self.status.connect(self.onStatusReceived)

        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)

        if self.edbc.ioclient('sg').ready:
            self.initSequences()



    @property
    def edbc(self):
        return RlpClient.instance()

    @property
    def dataSource(self):
        showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
        return showDataObj.dataSource('sequence')


    def onTableViewRequested(self, md):
        self.thumbView.setVisible(False)
        self.tableView.setVisible(True)
        self.tableView.forceUpdate()


    def onThumbViewRequested(self, md):
        self.tableView.setVisible(False)
        self.thumbView.setVisible(True)
        self.thumbView.forceUpdate()


    def onFilterChanged(self, text):
        self.tableView.grid_view.setFilterString(text, ['code', 'description'])
        self.thumbView.setFilterString(text)


    def onBreadcrumbNavItemsReady(self, md):

        colInfo = None
        mediaType = None
        if md['type'] == 'Sequence':
            colInfo = self.tableView.COLMODEL_SHOT
            mediaType = 'Shot'

        elif md['type'] == 'Shot':
            colInfo = self.tableView.COLMODEL_VERSION
            mediaType = 'Version'

        # TODO FIXME: DOESNT WORK
        #
        # if colInfo:
        #     colModel = self.tableView.grid_view.colModel()
        #     colModel.clear()
        #     for col in colInfo:
        #         colModel.addCol(col, -1)

        #     self.tableView.grid_view.updateHeader()


        self.refresh(mediaType, md['data'])


    def onStatusReceived(self, info):
        self.statusText.setText(info['msg'])
        self.statusText.update()


    def onThumbRequestReady(self, data):
        
        self.tableView.onThumbImageReady(data['run_id'], data['image'])
        self.thumbView.onThumbImageReady(data['run_id'], data['image'])


    def refresh(self, mediaType, result):
        for media_entry in result:
            if media_entry.get('image'):
                self.http_client.requestImage(media_entry['image'], str(media_entry['id']))

        self.tableView.refresh(mediaType, result)
        self.thumbView.refresh(result)


    def initSequences(self, md=None):

        if self.edbc is None:
            return

        self.breadcrumb.reset()
        self.statusText.setText('Getting Sequences..')

        def _seq_result_ready(result):
            # print('SEQ RESULT READY')
            # print(result)
            # self.__class__.SEQ_LIST = result

            self.refresh('Sequence', result)

            self.loadCount = len(result)
            self.loadIdx = 0

            hbutton = self.breadcrumb.hierarchyButtons['Sequence']
            hbuttonMenu = hbutton.menu()
            hbuttonMenu.clear()

            for media_entry in result:
                # if media_entry.get('image'):
                #     self.http_client.requestImage(media_entry['image'], str(media_entry['id']))

                hbuttonMenu.addItem(
                    media_entry['code'],
                    media_entry,
                    False
                )

            hbuttonMenu.updateScrolling()
            # self.grid_view.onParentSizeChanged(self.grid_view.width() - 1, self.grid_view.height())
            # self.grid_view.onParentSizeChanged(self.grid_view.width() + 1, self.grid_view.height())

            self.statusText.setText('')

            self._seqLoading = True


        filters= [
            ['project', 'is', self.project_entity],
            ['sg_sequence_type', 'not_in', ['Test', 'CrowdCycle']]
        ]
        return_fields = ['code', 'image', 'description']
        order_by = [{'field_name': 'code', 'direction': 'asc'}]

        iosg = self.edbc.ioclient('sg')
        if iosg:
            iosg.call(_seq_result_ready, 'cmds.find', 'Sequence',
                filters, return_fields, order=order_by).run()

        # if self.dataSource:

        #     # print('QUERYING SEQUENCES!!!!!')

        #     # pprint.pprint(ag)
        #     # print(self.project_entity)
        #     def _sequencesReady(result):
        #         print('SEQUENCES READy')
        #         print(result)


        #     self.dataSource.requestSequences(_sequencesReady,
        #         self.project_entity['code'])


    def loadSequence1(self, md):

        def _versionsReady(result):

            showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
            cbMediaDataFormatter = showDataObj.formatter('code')

            for ver_entry in result:
                mediaName, mediaMd = cbMediaDataFormatter(
                    ver_entry,
                    nameKey='media.name',
                    mergeMetadata='media.metadata'
                )
                ver_entry['media.name'] = mediaName
                ver_entry['media.metadata'] = mediaMd

            self.status.emit({'msg': ''})

            revlens.CNTRL.getMediaManager().loadMediaList(
                result, -1, md['code'], 1, True, True
            )

        self.status.emit({'msg': 'Loading {}..'.format(md['code'])})

        def _progress(info):
            print(info)
            if 'idx' in info['extra']:
                self.status.emit({'msg':'Loading {} - {} / {}'.format(
                    info['message'], info['extra']['idx'], info['extra']['count'])})

            else:
                self.status.emit({'msg': info['message']})


        future = self.edbc.ioclient('sg').call(_versionsReady,
            'cmds.get_sequence_media',
            self.project_entity['id'],
            md['id']
        )
        future.progress_callback = _progress
        future.run()


    def loadSequence(self, md):

        def _sequenceMediaReady(result):

            showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
            cbMediaDataFormatter = showDataObj.formatter('code')

            mediaList = []
            for mediaInfo in result:

                mediaName, mediaMd = cbMediaDataFormatter(
                    mediaInfo,
                    nameKey='pri')

                loadMedia = {
                    'media.name': mediaName,
                    'media.metadata': mediaMd,
                    'media.frame_count': int(mediaInfo['media_info']['rate'] * mediaInfo['media_info']['duration']),
                    'media.video': mediaInfo['path']
                }
                mediaList.append(loadMedia)


            self.status.emit({'msg': ''})

            revlens.CNTRL.getMediaManager().loadMediaList(
                mediaList, -1, md['code'], 1, True, True
            )


        prod = self.project_entity['code']
        seqCode = md['code']

        self.status.emit({'msg': 'Loading {}..'.format(seqCode)})

        self.dataSource.requestSequenceMedia(_sequenceMediaReady, prod, seqCode)


    def loadShot(self, md):
        showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
        cbMediaDataFormatter = showDataObj.formatter('code')

        seqName = md['sg_sequence']['name']
        shotName = md['code']
        if '/' in shotName:
            shotName = shotName.split('/')[1]

        
        print('GOT SHOT NAME: {}'.format(shotName))
        seqDataObj = showDataObj.getSequence(seqName, autocreate=True)
        shotDataObj = seqDataObj.getShot(shotName, autocreate=True)
        

        def _latestMediaReady(mediaInfo):
            print('got media')


            mediaName, mediaMd = cbMediaDataFormatter(
                mediaInfo,
                nameKey='pri')
            
            loadMedia = {
                'media.name': mediaName,
                'media.metadata': mediaMd,
                'media.frame_count': int(mediaInfo['media_info']['rate'] * mediaInfo['media_info']['duration']),
                'media.video': mediaInfo['path']
            }

            revlens.CNTRL.getMediaManager().loadMediaList(
                [loadMedia], -1, mediaName, 1, True, True
            )

        shotDataObj.requestLatestMedia(_latestMediaReady)



    def loadVersion(self, md):
        
        print('LOAD VERSION')

        showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
        cbMediaDataFormatter = showDataObj.formatter('code')
        mcode, mediaMd = cbMediaDataFormatter(md)

        mediaList = [{
            'media.name': mcode,
            'media.video': md['sg_path_to_frames'],
            'media.metadata': mediaMd,
            'media.frame_count': md['frame_count']
        }]

        # pprint.pprint(mediaList)

        revlens.CNTRL.getMediaManager().loadMediaList(
            mediaList, -1, mcode, 1, True, True
        )


    def onLoadRequested(self, md):

        # print('SequenceMEdiaVide on load requested')
        # pprint.pprint(md)
        if md['type'] == 'Sequence':
            self.loadSequence(md)

        elif md['type'] == 'Shot':
            self.loadShot(md)

        elif md['type'] == 'Version':
            self.loadVersion(md)


    def onCellWidgetEvent(self, evtName, evtInfo):

        if evtName != 'cell.button_pressed':
            return

        if not evtInfo['in_item_select']:
            return

        rowData = evtInfo['row_data']
        self.breadcrumb.selectItem(rowData)


    def onNoticeEvent(self, evtName, evtInfo):
        if evtName == 'ioservice_event':
            if evtInfo['method'] == 'queue_ready':

                qName = evtInfo['result']['queue_name']
                if qName == 'iosg':
                    self.initSequences()




    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)

        self.tableView.onParentSizeChangedItem(width, height)
        self.tableView.forceUpdate()

        self.thumbView.onParentSizeChangedItem(width, height)
        self.thumbView.forceUpdate()