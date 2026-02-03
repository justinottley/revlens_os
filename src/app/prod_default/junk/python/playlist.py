class TrackMedia(RlpGui.GUI_ItemBase):

    def __init__(self, parent, track):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.track = track

        # TODO FIXME: there is a problem with this sig-slot connection,
        # adding the second data param fails to have the signal delivered
        # through the binding.. why???
        # self.track.mediaDataReady.connect(self.onMediaDataReady)

        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)
        revlens.CNTRL.getMediaManager().mediaDataReady.connect(self.onMediaDataReady)
        revlens.CNTRL.getMediaManager().itemSelected.connect(self.onMediaItemSelected)

        self.grid = RlpProd.GUI_PlaylistView(self)
        self.grid.requestAddToPlaylist.connect(self.onAddToPlaylistRequested)
        self.grid.setPos(0, 5)

        # self.trackLabel = RlpGui.GUI_Label(self, track.name())

        # self.grid.toolbar().layout().addSpacer(5)
        # self.grid.toolbar().layout().addItem(self.trackLabel, 4)

        self.rowUuidMap = {}

        colModel = self.grid.colModel()
        colModel.addCol({
            "col_name": "name",
            "display_name": "Name",
            "col_type": "str",
            "metadata": {},
            "width": 140
        }, -1)

        colModel.addCol({
            "col_name": "image",
            "display_name": "Thumbnail",
            "col_type": "image_b64",
            "metadata": {},
            "width": 80
        }, -1)

        colModel.addCol({
            "col_name": "frame",
            "display_name": "Frame",
            "col_type": "str",
            "metadata": {},
            "width": 100
        }, -1)

        self.grid.updateHeader()

        self.grid.selectionChanged.connect(self.onGridSelectionChanged)
        self.refresh()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onGridSelectionChanged(self, selMode, selInfo):

        # TODO FIXME: python binding
        selInfo = selInfo.toPy()

        print('{} {}'.format(selMode, selInfo))

        for tlViewName in revlens.TL_CNTRL.viewNames():
            tlView = revlens.TL_CNTRL.getTimelineView(tlViewName)
            tlTrack = tlView.trackManager().getTrackByUuid(self.track.uuid().toString())
            if not tlTrack:
                print('Warning: no track at {}'.format(self.track.uuid().toString()))
                continue

            tlTrack.clearSelection()

            for selItem in selInfo:

                mediaUuid = selItem['uuid']
                tlItem = tlTrack.getItemByUuid(mediaUuid)
                if not tlItem:
                    print('Warning: no timeline item at {}'.format(mediaUuid))

                if selMode == 'set':
                    tlItem.setSelected(True)
                elif selMode == 'rem':
                    tlItem.setSelected(False)


                tlItem.updateItem()


        frameNum = selInfo['frame']
        revlens.cmds.goto_frame(frameNum)


    def onAddToPlaylistRequested(self, mediaPayload):

        media_in = {
            'media.video': mediaPayload['sg_path_to_frames'],
            'media.name': mediaPayload['code'],
            'media.frame_count': mediaPayload['frame_count'],
            'media.metadata': {
                'entity.name': mediaPayload['code']
            }
        }

        revlens.CNTRL.getMediaManager().loadMediaList(
            [media_in], self.track.index(), "", self.track.getMaxFrameNum() + 1, False)


    def onMediaDataReady(self, evtName, evtInfo):

        if evtName == 'thumbnail_ready':

            if evtInfo.get('track_idx') == self.track.index():
                incomingUuid = evtInfo['graph.uuid'].toString()
                if incomingUuid in self.rowUuidMap:
                    rowIdx = self.rowUuidMap[incomingUuid]
                    # for rowIdx in range(self.grid.rowCount()):
                    row = self.grid.rowAt(rowIdx)
                    rowUuid = row.modelValue('uuid')
                    if rowUuid == incomingUuid:
                        nodeProps = revlens.CNTRL.session().getNodeByUuid(rowUuid).getProperties()
                        if 'media.thumbnail.data' in nodeProps:
                            cell = row.cell("image")
                            cell.insertValue("value", nodeProps['media.thumbnail.data'])
                            cell.removeValue("image") # not confusing at all...
                            cell.update()

    def onMediaItemSelected(self, itemInfo):

        if itemInfo['track_uuid'] != self.track.uuid().toString():
            return

        mediaUuid = itemInfo['media_uuid']
        if mediaUuid in self.rowUuidMap:
            rowIdx = self.rowUuidMap[mediaUuid]
            row = self.grid.rowAt(rowIdx)
            row.setRowSelected(True, 0)

        nodeFrame = self.track.getNodeFrameByUuid(mediaUuid)
        revlens.cmds.goto_frame(nodeFrame)


    def onNoticeEvent(self, evtName, evtInfo):

        if evtName == 'load_complete':
            self.refresh()


    def forceUpdate(self):
        self.grid.updateRows(True)


    def refresh(self):

        # self.trackLabel.setText(self.track.name())
        itemList = []
        nodeFrameList = self.track.getNodeFrameList()

        self.rowUuidMap = {}

        rowIdx = 0
        for trackItem in nodeFrameList:

            frameNum = trackItem['frame']
            node = self.track.getNodeByFrame(frameNum)
            nodeProps = node.getProperties()
            nodeUuid = nodeProps['graph.uuid'].toString()
            # print(trackItem)
            # print(nodeProps)
            gridData = {
                'name': nodeProps['media.name'],
                'frame': frameNum,
                'length': nodeProps['media.frame_count'],
                'uuid': nodeUuid
            }
            if 'media.thumbnail.data' in nodeProps:
                gridData['image'] = nodeProps['media.thumbnail.data']

            itemList.append(gridData)
            self.rowUuidMap[nodeUuid] = rowIdx
            rowIdx += 1

        self.grid.setModelDataList(itemList)


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)

        self.grid.setWidth(width - 20)
        self.grid.setHeight(height)

class ReviewlistTrackMedia(TrackMedia):

    def __init__(self, parent, track):
        TrackMedia.__init__(self, parent, track)

        self.reviewListSelector = RlpGui.GUI_IconButton("", self, 20, 4)
        self.reviewListSelector.setOutlined(True)
        self.reviewListSelector.setText("Reviewlist:")
        self.reviewListSelector.setSvgIconPath(':/feather/chevron-down.svg', 20, True)

        revlistMenu = self.reviewListSelector.menu()
        revlistMenu.setBgColour(QtGui.QColor(30, 30, 30))

        self.revlistMenuItem = RlpGui.GUI_MenuItem("", revlistMenu.itemWrapper())
        self.revlistMenuItem.setWidth(420)
        self.revlistMenuItem.setHeight(500)

        self.revlistView = ReviewListView(self.revlistMenuItem)
        self.revlistView.grid_view.selectionChanged.connect(self.onReviewlistSelected)
        
        revlistMenu.addWidgetItem(self.revlistMenuItem)

        self.loadButton = RlpGui.GUI_IconButton("", self, 20, 4)
        self.loadButton.setText("Load")
        self.loadButton.setOutlined(True)
        self.loadButton.buttonPressed.connect(self.onLoadRequested)

        self.spacer = RlpGui.GUI_ItemBase(self)
        self.spacer.setWidth(20)

        self.grid.toolbar().layout().addItemAtStart(self.loadButton, 0)
        self.grid.toolbar().layout().addItemAtStart(self.spacer, 0)
        self.grid.toolbar().layout().addItemAtStart(self.reviewListSelector, 0)
        self.grid.toolbar().layout().addItemAtStart(self.spacer, 0)


    def onReviewlistSelected(self, selMode, selInfo):

        if selMode != 'set':
            return

        if 'parent' not in selInfo:
            return

        # TODO FIXME: python binding
        selInfo = selInfo.toPy()

        self.selReview = selInfo[0]
        self.selReview['type'] = 'Playlist'
        self.selReview['code'] = os.path.basename(selInfo['path'])

        text = '{} : {}'.format(selInfo['parent'], selInfo['path'])
        self.reviewListSelector.setText(text)
        self.reviewListSelector.menu().setVisible(False)
        self.reviewListSelector.update()



    def onLoadRequested(self, md):
        print(md)

        import rlplug_shotgrid.cmds
        rlplug_shotgrid.cmds.request_load(
            'Playlist',
            self.selReview,
            title=self.selReview['code'],
            track_idx=self.track.index(),
            media_callback=_media_callback
        )


class SequenceMediaView(TableView):

    def __init__(self, parent):
        TableView.__init__(self, parent)

        self.load_button.setVisible(False)

        grid_toolbar = self.grid_view.toolbar()
        grid_toolbar.colSelector().setWidth(0)
        grid_toolbar.colSelector().setVisible(False)

        self.filter_textarea = RlpGui.GUI_TextEdit(grid_toolbar, 230, 30)
        self.filter_textarea.textChanged.connect(self.onFilterChanged)
        self.filter_textarea.setTempHintText('Filter:')
        grid_toolbar.layout().addItemAtStart(self.filter_textarea, 0)

        # self.seqMediaView.grid_view.toolbar().setVisible(False)
        # self.seqMediaView.grid_view.toolbar().setHeight(0)
        self.grid_view.header().setVisible(False)
        # self.seqMediaView.grid_view.header().setHeight(0)
        self.grid_view.scrollArea().setPos(0, 40)


    def onFilterChanged(self, text):
        self.grid_view.setFilterString(text, ['code', 'description'])


class SequenceTrackMedia(TrackMedia):

    SEQ_LIST = None

    def __init__(self, parent, track):
        TrackMedia.__init__(self, parent, track)

        # ThumbnailCache.instance().thumbRequestReady.connect(self.onThumbRequestReady)
        self.http_client = RlpCore.CoreNet_HttpClient()
        self.http_client.requestReady.connect(self.onThumbRequestReady)

        self.seqSelector = RlpGui.GUI_IconButton("", self, 20, 4)
        self.seqSelector.setOutlined(True)
        self.seqSelector.setText("Sequence:")
        self.seqSelector.setSvgIconPath(':/feather/chevron-down.svg', 20, True)

        seqMenu = self.seqSelector.menu()
        seqMenu.setBgColour(QtGui.QColor(30, 30, 30))

        self.viewMenuItem = RlpGui.GUI_MenuItem("", seqMenu.itemWrapper()) # GUI_ItemBase(seqMenu)
        self.viewMenuItem.setWidth(420)
        self.viewMenuItem.setHeight(500)

        self.seqMediaView = SequenceMediaView(self.viewMenuItem)
        self.seqMediaView.grid_view.selectionChanged.connect(self.onSequenceSelected)

        seqMenu.addWidgetItem(self.viewMenuItem)

        # seqMenu.setWidth(self.seqMediaView.width())
        # seqMenu.setHeight(self.seqMediaView.height())
        seqMenu.menuItemSelected.connect(self.onSequenceSelected)

        self.loadButton = RlpGui.GUI_IconButton("", self, 20, 4)
        self.loadButton.setText("Load")
        self.loadButton.setOutlined(True)
        self.loadButton.buttonPressed.connect(self.onLoadRequested)

        self.macroSelector = RlpGui.GUI_IconButton("", self, 20, 4)
        self.macroSelector.setOutlined(True)
        self.macroSelector.setText("Load Macro:")
        self.macroSelector.setSvgIconPath(':/feather/chevron-down.svg', 20, True)

        self.spacer = RlpGui.GUI_ItemBase(self)
        self.spacer.setWidth(20)

        self.grid.toolbar().layout().addItemAtStart(self.loadButton, 0)
        self.grid.toolbar().layout().addItemAtStart(self.spacer, 0)
        self.grid.toolbar().layout().addItemAtStart(self.macroSelector, 0)
        self.grid.toolbar().layout().addItemAtStart(self.spacer, 0)
        self.grid.toolbar().layout().addItemAtStart(self.seqSelector, 0)
        self.grid.toolbar().layout().addItemAtStart(self.spacer, 0)

        self.initSequences()

    @property
    def project_entity(self):
        ag = RlpUtil.get_app_globals()
        if 'project.info' in ag:
            return {'type': 'Project', 'id': ag['project.info']['id']}

    @property
    def edbc(self):
        return RlpClient.instance()


    def initSequences(self):

        def _seq_result_ready(result):
            print('SEQ RESULT READY')
            self.__class__.SEQ_LIST = result

            self.seqMediaView.refresh(result)

            for media_entry in result:
                if media_entry.get('image'):
                    self.http_client.requestImage(media_entry['image'], str(media_entry['id']))


        if self.__class__.SEQ_LIST is not None:
            return _seq_result_ready(self.__class__.SEQ_LIST)
        
        filters= [
            ['project', 'is', self.project_entity],
            ['sg_sequence_type', 'not_in', ['Test', 'CrowdCycle']]
        ]
        return_fields = ['code', 'image', 'description']
        order_by = [{'field_name': 'code', 'direction': 'asc'}]

        self.edbc.sg_find(_seq_result_ready, 'Sequence',
            filters, return_fields, order=order_by).run()


    def onThumbRequestReady(self, data):
        self.seqMediaView.onThumbImageReady(data['run_id'], data['image'])


    def onSequenceSelected(self, rowInfo):
        print(rowInfo)

        self.selRow = rowInfo
        self.seqSelector.setText(rowInfo['code'])
        self.seqSelector.menu().setVisible(False)
        self.seqSelector.update()

        # HACK refresh for right aligned toolbar widgets
        self.grid.onParentSizeChanged(self.grid.width() - 1, self.grid.height())
        self.grid.onParentSizeChanged(self.grid.width() + 1, self.grid.height())


    def onLoadRequested(self, md):

        import rlplug_shotgrid.cmds
        rlplug_shotgrid.cmds.request_load(
            'Sequence',
            self.selRow,
            title=self.selRow['code'],
            filters=[],
            pref_dept_list=['lighting', 'animation', 'layout', 'editorial'],
            pref_order='most_recent',
            track_idx=self.track.index(),
            media_callback=_media_callback
        )

