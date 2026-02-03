
import os
import json
import pprint

from rlp import QtCore, QtGui

import rlp.gui as RlpGui
import RlpProdGUImodule as RlpProd

from rlp.core.net.websocket import RlpClient

import revlens

import revlens_prod.cmds
from revlens_prod.prod_data import ShowData


class TrackMediaWidget(RlpGui.GUI_ItemBase):

    DEPTS = [
        'editorial',
        'layout',
        'animation',
        'lighting'
    ]

    def __init__(self, parent, sessionPanel):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        # self.currTrack = None

        self.sessionPanel = sessionPanel

        self.grid = RlpProd.GUI_PlaylistView(self)
        self.grid.scrollArea().setHScrollBarVisOffset(-1)
        # self.grid.requestAddToPlaylist.connect(self.onAddToPlaylistRequested)
        self.grid.setPos(0, 5)

        self.contextMenu = RlpGui.GUI_MenuPane(self)
        self.contextMenu.setVisible(False)
        self.contextMenu.menuItemSelected.connect(self.onContextMenuItemSelected)
        self.contextMenu.menuShown.connect(self.onContextMenuShown)

        self.contextMenu.addItem('New Sub Track', {'action': 'new_sub_track'}, False)
        shotContextMenu = self.contextMenu.addItem('Context', {}, False)
        shotContextMenu.menu().menuItemSelected.connect(self.onContextMenuItemSelected)
        shotContextMenu.addItem('+/- 1', {'action': 'load_context', 'inc': 1})
        shotContextMenu.addItem('+/- 2', {'action': 'load_context', 'inc': 2})

        self.statusText = RlpGui.GUI_Label(self, '')

        self.grid.toolbar().layout().addSpacer(5)
        self.grid.toolbar().layout().addItem(self.statusText, 4)

        self.rowUuidMap = {}

        colModel = self.grid.colModel()
        colModel.addCol({
            "col_name": "name",
            "display_name": "Name",
            "col_type": "str",
            "metadata": {},
            "width": 120
        }, -1)

        colModel.addCol({
            "col_name": "image",
            "display_name": "Thumbnail",
            "col_type": "image_b64",
            "metadata": {},
            "width": 80
        }, -1)

        colModel.addCol({
            'col_name': 'dept',
            'display_name': 'Department',
            'col_type': 'str',
            'metadata': {
                'widgets': {
                    'choice': {}
                }
            },
            'width': 120
        }, -1)

        colModel.addCol({
            'col_name' : 'version',
            'display_name': 'Version',
            'col_type': 'str',
            'metadata': {
                'widgets': {
                    'choice': {}
                }
            },
            'width': 80
        }, -1)

        colModel.addCol({
            'col_name' : 'testid',
            'display_name': 'TID',
            'col_type': 'str',
            'metadata': {
                'widgets': {
                    'choice': {}
                },
            },
            'width': 80
        }, -1)

        # colModel.addCol({
        #     "col_name": "frame",
        #     "display_name": "Frame",
        #     "col_type": "str",
        #     "metadata": {},
        #     "width": 100
        # }, -1)

        self.grid.updateHeader()
        self.grid.modelDataChanged.connect(self.onGridModelDataChanged)
        self.grid.selectionChanged.connect(self.onGridSelectionChanged)
        self.grid.cellWidgetEvent.connect(self.onGridCellWidgetEvent)

        self.onParentSizeChangedItem(parent.width(), parent.height())

        self.currSelected = []

        # TODO FIXME: this should not be necessary, side effect of python object garbage collection
        self.rowWidgets = {}


    @property
    def edbc(self):
        return RlpClient.instance()


    @property
    def currTrack(self):
        return self.sessionPanel.selectedTrack


    def refresh(self, mediaList):
        pass


    def mousePressEventItem(self, event):
        self.grid.clearSelection()


    def requestMediaLoad(self, md):

        # pprint.pprint(md)

        verSelected = md['text']
        verChoices = md.get('row_data', {}).get('metadata', {}).get('version.choices', {})

        tidVal = ''
        if verSelected in verChoices:
            verInfo = verChoices[verSelected]
            tidVal = verInfo.get('testid', '')

        tidCell = self.grid.rowAt(md['row_idx']).cell('testid')
        if tidCell:
            tidCell.insertValue('value', tidVal)
            tidCell.update()


        mdInfo = md['data']
        loadInfo = {
            'media.frame_count': int(mdInfo['media_info']['rate'] * mdInfo['media_info']['duration']),
            'media.video': mdInfo['path'],
            'media.name': mdInfo['pri']
        }

        nodeFrame = md['row_data']['frame']
        node = self.currTrack.getNodeByFrame(nodeFrame)
        dlNode = node.getNodeProperty('graph.node.deferred_load')
        if dlNode:
            dlNode.loadMedia(loadInfo)


    def requestDeptVersions(self, md, loadLatestVersion=False):

        rowMd = md['row_data']['metadata']

        def _resultsReady(result):
            # return
            # print('GOT DEPT VERSIONS: {}'.format(len(result)))
            # print('----')
            # import pprint
            # pprint.pprint(result)
            # print('----')

            verMap = {}
            tidMap = {}
            for entry in result:
                verNum = int(entry['version'])
                verMap[verNum] = entry
                if 'testid' in entry and entry['testid']:
                    tidMap[int(entry['testid'])] = {'version': verNum}

            row = self.grid.rowAt(md['row_idx'])
            for cellName in ['version', 'testid']:
                cell = row.cell(cellName)
                if cell:
                    menu = cell.widget('choice').menuButton().menu()
                    menu.clear()



            nodeUuid = md['row_data']['uuid']
            node = revlens.CNTRL.session().getNodeByUuid(nodeUuid)
            nodeMd = {}
            if node.hasNodeProperty('media.metadata'):
                nodeMd = node.getNodeProperty('media.metadata')

            nodeMd['version.choices'] = verMap
            node.insertProperty('media.metadata', nodeMd)

            if loadLatestVersion:

                # select latest version
                verList = list(reversed(sorted(verMap.keys())))
                latestVer = verList[0]
                verTid = tidMap.get(latestVer, '')

                verCell = row.cell('version')
                verCell.insertValue('value', str(latestVer))
                verCell.update()

                tidCell = row.cell('testid')
                tidCell.insertValue('value', str(verTid))
                tidCell.update()

                self.requestMediaLoad({
                    'text': str(latestVer),
                    'row_data': md['row_data'],
                    'row_idx': md['row_idx'],
                    'data': verMap[latestVer]
                })



        if 'seq' in rowMd and 'shot' in rowMd:

            self.edbc.ioclient('sg').call(
                _resultsReady,
                'sg.search',
                [
                    "seq='{}'".format(rowMd['seq']),
                    "shot='{}'".format(rowMd['shot']),
                    "dept='{}'".format(md['text'])
                ]
            ).run()


    def loadContextTrack(self, mediaList):

        name = '{} Context'.format(shotCode)
        trackUuid = QtCore.QUuid.createUuid()

        track = revlens.CNTRL.session().addTrack(name, "Media", trackUuid, "", True)

        #   def _shotContextReady()


    def onContextMenuItemSelected(self, md):

        print('menu items elected')
        print(md)
        print(self.currSelected)

        action = md['data'].get('action')
        if action != 'load_context':
            return

        contextSize = md['data']['inc']
        if len(self.currSelected) != 1:
            print('MULTIPLE selection NOT SUPPORTED')
            return

        nodeUuid = self.currSelected[0]
        node = revlens.CNTRL.session().getNodeByUuid(nodeUuid)
        if node:
            nodeMd = node.getNodeProperty('media.metadata')
            print('MEDIA METDATA')

            seqCode = nodeMd.get('seq')
            shotNum = nodeMd.get('shot')
            dept = nodeMd.get('dept')
            shotCode = '{}/{}'.format(seqCode, shotNum)

            pdShow = ShowData.instance()

            if seqCode:

                def _callback(pdSeq):
                    shot = pdSeq.getShot(shotCode)

                    shotContextList = []
                    prevShot = shot
                    for inc in range(contextSize):

                        prevShot = prevShot.prev()
                        if prevShot:
                            prevShot.values['metadata']['is_context'] = True
                            shotContextList.insert(0, prevShot)

                    shot.values['metadata']['is_context_main'] = True

                    shotContextList.append(shot)

                    nextShot = shot
                    for inc in range(contextSize):

                        nextShot = nextShot.next()
                        if nextShot:
                            nextShot.values['metadata']['is_context'] = True
                            shotContextList.append(nextShot)

                    searchExprList = []
                    for contextShot in shotContextList:
                        searchExprList.append("shot='{}'".format(contextShot.values['metadata']['shot']))
                    
                    searchExpr = ' or '.join(searchExprList)
                    searchExpr = '( {} )'.format(searchExpr)
                    print(searchExpr)

                    def _shotMediaResultReady(shotMediaResult):
                        print('GOT SHOT MEDIA RESULT: {}'.format(len(shotMediaResult)))
                        # print(shotMediaResult)
                        pdSeq = pdShow.getSequence(seqCode)
                        if not pdSeq:
                            print('ERROR: INVALID SHOW DATA SEQUNCE {}'.format(seqCode))
                            return

                        pdSeq.loadMedia(shotMediaResult)
                        bestMedia = []

                        mediaList = []
                        for shotEntry in shotContextList:

                            useVer = None
                            verMap = shotEntry.getMediaByIndex('version')
                            verList = list(reversed(sorted(verMap.keys())))
            
                            if 'is_context_main' in shotEntry.values['metadata']:
                                
                                useVer = int(nodeMd['version'])
                                if useVer not in verMap:
                                    print('WARNING: existing version not available?? {}'.format(useVer))
                                    pprint.pprint(verMap)
                                    if verList:
                                        useVer = verList[0]
                                

                            else:
                                
                                if verList:
                                    useVer = verList[0]

                            if useVer:
                                media = verMap.get(useVer)
                                if not media:
                                    print('ERROR: NO MEDIA FOR VERSION {}'.format(useVer))
                                    pprint.pprint(verMap)

                                mediaMd = media.values['metadata']

                                for cKey in ['is_context', 'is_context_main']:
                                    if shotEntry.values['metadata'].get(cKey):
                                        mediaMd[cKey] = True
                                frameCount = int(media.values['media_info']['rate'] * media.values['media_info']['duration'])
                                

                                mediaList.append({
                                    'media.video': media.values['path'],
                                    'media.name': mediaMd.get('entity.name', media.values['name']),
                                    'media.frame_count': frameCount,
                                    'media.metadata': mediaMd
                                })

                            # print(verList)


                        if mediaList:
                            name = '{} Context'.format(shotCode)
                            trackUuid = QtCore.QUuid.createUuid()

                            track = revlens.CNTRL.session().addTrack(name, "Media", trackUuid, "", True)
                            revlens.CNTRL.getMediaManager().loadMediaList(
                                mediaList,
                                track.index(),
                                "",
                                track.getMaxFrameNum() + 1,
                                False,
                                True
                            )

                        # pprint.pprint(mediaList)
                        # print(len(shotMediaResult))


                    self.edbc.ioclient('sg').call(
                        _shotMediaResultReady,
                        'sg.search',
                        [
                            "seq='{}'".format(seqCode),
                            "dept='{}'".format(dept),
                            searchExpr
                        ]
                    ).run()
                    # self.buildContextTrack(shotCode, result, contextSize=contextSize)


                pdShow.loadSequence(_callback, seqCode)


    def onContextMenuShown(self, md):
        print(md)


    def onGridCellWidgetEvent(self, evtName, evtInfo):

        # print('cell widget event: {} {}'.format(evtName, evtInfo))

        if (evtName == 'choice.menu_item_selected'):

            colName = evtInfo['col_name']
            newValue = evtInfo['text']

            cell = self.grid.rowAt(evtInfo['row_idx']).cell(colName)
            cell.insertValue('value', newValue)
            cell.update()

            # pprint.pprint(evtInfo)
            nodeUuid = evtInfo['row_data']['uuid']
            node = revlens.CNTRL.session().getNodeByUuid(nodeUuid)
            nodeMd = {}
            if node.hasNodeProperty('media.metadata'):
                nodeMd = node.getNodeProperty('media.metadata')

            # pprint.pprint(nodeMd)
            prevValue = nodeMd.get(colName)
            if not prevValue:
                print('Error: Missing metadata on media, missing "{}"'.format(colName))
                return


            nodeMd[colName] = newValue
            node.insertProperty('media.metadata', nodeMd)

            if (colName == 'dept' and ((newValue != prevValue) or ('version.choices' not in nodeMd))):

                for cellName in ['version', 'testid']:
                    cell = self.grid.rowAt(evtInfo['row_idx']).cell(cellName)
                    if cell:
                        cell.insertValue('value', '')
                        cell.update()


                self.requestDeptVersions(evtInfo, loadLatestVersion=True)

            elif colName == 'version':
                self.requestMediaLoad(evtInfo)


        elif (evtName == 'choice.menu_button_pressed'):

            colName = evtInfo['col_name']

            row = self.grid.rowAt(evtInfo['row_idx'])
            rowData = row.rowData().values()
            cmenu = row.cell(colName).widget('choice').menuButton().menu()

            if cmenu.itemCount() != 0:
                return

            node = revlens.CNTRL.session().getNodeByUuid(rowData['uuid'])
            nodeMd = node.getNodeProperty('media.metadata')
            if 'version.choices' in nodeMd:

                verMap = nodeMd['version.choices']

                if colName == 'version':
                    verList = list(reversed(sorted([int(k) for k in verMap.keys()])))
                    for verNum in verList:
                        verEntry = verMap[str(verNum)]
                        cmenu.addItem(str(verNum), verEntry, False)

                elif colName == 'testid':
                    tidMap = {}
                    for verNum, verInfo in verMap.items():
                        if 'testid' in verInfo and verInfo['testid']:
                            tidMap[int(verInfo['testid'])] = {'version': verNum}

                    tidList = list(reversed(sorted(tidMap.keys())))
                    for tidEntry in tidList:
                        tidInfo = tidMap[tidEntry]
                        cmenu.addItem(str(tidEntry), tidInfo, False)

                cmenu.updateScrolling()
                cmenu.update()



        elif (evtName == "cell.button_pressed"):

            print('CELL BUTTON PRESED')
            # print(evtInfo)
            if evtInfo['button'] == QtGui.LEFT_BUTTON:
                self.contextMenu.setVisible(False)

            elif evtInfo['button'] == QtGui.RIGHT_BUTTON:
                self.contextMenu.setPos(
                    evtInfo['scene_pos.x'],
                    evtInfo['scene_pos.y']
                )
                self.contextMenu.setVisible(True)


    def onGridModelDataChanged(self):
        # print('MODEL DATA CHAGNED')

        contextCol = QtGui.QColor(109, 76, 76) # (100, 60, 60)
        contextMainCol = QtGui.QColor(57, 88, 53) # 0, 100, 60)

        for rowIdx in range(self.grid.rowCount()):
            row = self.grid.rowAt(rowIdx)
            if not row:
                print('WARNING: invalid row at {}'.format(rowIdx))
                continue

            rowData = row.rowData().values()

            nodeUuid = rowData['uuid']
            nodeMd = {}
            node = revlens.CNTRL.session().getNodeByUuid(nodeUuid)
            if node.hasNodeProperty('media.metadata'):
                nodeMd = node.getNodeProperty('media.metadata')

            if 'is_context' in nodeMd:
                row.setBgColour(contextCol)

            elif 'is_context_main' in nodeMd:
                row.setBgColour(contextMainCol)

    
            deptCell = row.cell('dept')
            if deptCell: #  and nodeMd:
                deptChoice = deptCell.widget('choice')
                deptMenu = deptChoice.menuButton().menu()
                for dept in self.DEPTS:
                    deptMenu.addItem(dept, {}, False)

                if 'metadata' in rowData and 'version.choices' not in nodeMd:
                    
                    # seed it so the data is fetched only once
                    nodeMd['version.choices'] = {}
                    node.insertProperty('media.metadata', nodeMd)

                    
                    self.requestDeptVersions({
                        'text': rowData['dept'],
                        'row_data': rowData,
                        'row_idx': rowIdx
                    })


            # Load prefetched version choices out of node if available
            #
            if 'version.choices' in nodeMd:
                pass
                # verMap = nodeMd['version.choices']
                # tidMap = {}
                # for verNum, verInfo in verMap.items():
                #     if 'testid' in verInfo and verInfo['testid']:
                #         tidMap[int(verInfo['testid'])] = {'version': int(verNum)}

                # verMenu = row.cell('version').widget('choice').menuButton().menu()
                # tidMenu = row.cell('testid').widget('choice').menuButton().menu()

                # # the keys are stored as int, why are they not ints here?
                # verList = list(reversed(sorted([int(k) for k in verMap.keys()])))
                # for verNum in verList:
                #     verEntry = verMap[str(verNum)]
                #     verMenu.addItem(str(verNum), verEntry, False)

                # tidList = list(reversed(sorted(tidMap.keys())))
                # for tidNum in tidList:
                #     tidEntry = tidMap[tidNum]
                #     tidMenu.addItem(str(tidNum), tidEntry, False)




    def onGridSelectionChanged(self, selMode, selInfo):
        # print('Selection: {}'.format(selInfo))

        selInfo = selInfo.toPy()

        # print('onGridSelected: {} {}'.format(selMode, selInfo))

        if selMode == 'set':
            self.currSelected = []


        for tlViewName in revlens.TL_CNTRL.viewNames():
            tlView = revlens.TL_CNTRL.getTimelineView(tlViewName)
            tlTrack = tlView.trackManager().getTrackByUuid(self.currTrack.uuid().toString())
            if not tlTrack:
                print('Warning: no track at {}'.format(self.currTrack.uuid().toString()))
                continue
            
            if selMode == 'set':
                tlTrack.clearSelection()

            for selItem in selInfo:
                mediaUuid = selItem['uuid']
                tlItem = tlTrack.getItemByUuid(mediaUuid)
                if tlItem:

                    tlUuid = tlItem.uuid().toString()

                    if selMode == 'set':
                        tlItem.setSelected(True)
                        self.currTrack.setNodeEnabled(mediaUuid, True)
                        self.currSelected.append(tlUuid)

                    elif selMode == 'rem':
                        tlItem.setSelected(False)
                        if tlUuid in self.currSelected:
                            self.currSelected.remove(tlUuid)

                        else:
                            print('WARNING: NOT FOUND IN CURRENT SELECTION: {}'.format(tlUuid))

                    tlItem.updateItem()


                else:
                    print('Warning: no timeline item at {}'.format(mediaUuid))
                    continue


        revlens.CNTRL.emitNoticeEvent(
            'playlist.selection_updated',
            {'selection': self.currSelected, 'mode': selMode, 'sel_info': selInfo}
        )


        session = revlens.CNTRL.session()
        if session.getProperty('bookends.sticky'):

            if not self.currSelected:
                revlens.cmds.clear_in_out_frames()
                revlens.CNTRL.session().resetNodeEnabled()

            else:
                revlens_prod.cmds.bookend_selected_shots()

        if selMode == 'set' and len(selInfo) == 1:
            frameNum = selInfo[0]['frame']
            revlens.cmds.goto_frame(frameNum)

        else:
            revlens.cmds.goto_frame(revlens.cmds.get_current_frame_num())


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height - 40)

        self.grid.setWidth(width - 20)
        self.grid.setHeight(height - 40)



class TrackMediaData(QtCore.QObject):

    def __init__(self, track):
        QtCore.QObject.__init__(self)

        self.mediaListReady = QtCore.PY_Signal(self)

        self.track = track


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
            nodeMd = nodeProps.get('media.metadata', {})


            gridData = {
                'name': nodeProps['media.name'],
                'frame': frameNum,
                'length': nodeProps['media.frame_count'],
                'uuid': nodeUuid,
                'dept': nodeMd.get('dept', ''),
                'version': nodeMd.get('version', ''),
                'testid': nodeMd.get('testid', ''),
                'metadata': nodeMd,
                'source_info': nodeProps['media.source_info']
            }
            if 'media.thumbnail.data' in nodeProps:
                gridData['image'] = nodeProps['media.thumbnail.data']

            itemList.append(gridData)
            self.rowUuidMap[nodeUuid] = rowIdx
            rowIdx += 1

        # self.grid.setModelDataList(itemList)
        self.mediaListReady.emit({'media': itemList})


    def disconnectPySlots(self):
        self.mediaListReady.disconnectAll()



class TrackTab(RlpGui.GUI_ItemBase):

    BASE_HEIGHT = 60

    def __init__(self, parent, track):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.setItemClipsChildren(True)
        self.setItemAcceptsDrops(True)

        self.selected = QtCore.PY_Signal(self)

        self.track = track
        self.track.mediaRemoved.connect(self.onMediaRemoved)

        self.name_label = RlpGui.GUI_Label(self, '')
        nlf = self.name_label.font()
        nlf.setPixelSize(nlf.pixelSize() + 2)
        nlf.setBold(True)
        self.name_label.setFont(nlf)
        self.name_label.setText(self.track.name())
        self.name_label.setPos(20, 10)

        self.contextMenu = RlpGui.GUI_MenuPane(self)
        self.contextMenu.setVisible(False)
        self.contextMenu.menuItemSelected.connect(self.onContextMenuItemSelected)

        for (item, itemMd) in [
            ('Duplicate', {'action': 'duplicate'}),
            ('Rename..', {'action': 'rename'}),
            ('', {}),
            ('Remove Item at Current Frame', {'action': 'remove_current_item'}),
            ('Clear', {'action': 'clear'}),
            ('Delete', {'action': 'delete'})
        ]:
            if not item:
                self.contextMenu.addSeparator()

            else:
                self.contextMenu.addItem(item, itemMd, False)



        self._selected = False
        self._inDrag = False

        self.setHeight(self.BASE_HEIGHT)
        self.onParentSizeChangedItem(parent.width(), parent.height())


    def isSelected(self):
        return self._selected


    def setSelected(self, isSelected, pushUpdate=True):
        self._selected = isSelected

        if not self.track:
            return

        if pushUpdate:
            session = self.track.session()
            for trackIdx in range(session.numTracks()):
                strack = session.getTrackByIndex(trackIdx)
                if strack.trackType() != revlens.TRACK_TYPE_MEDIA:
                    continue
                if strack != self.track:
                    strack.setEnabled(not isSelected, False)

            self.track.setEnabled(isSelected, False)

            revlens.CNTRL.onTrackDataChanged("set_enabled", "", {})

        self.update()


    def select(self, doSync=True):
        '''
        Just for a clear API to select this tab
        '''
        self.mousePressEventItem(None, doSync=doSync)


    def mousePressEventItem(self, evt, doSync=True):

        if not evt or evt.leftButton():

            self._selected = True
            self._inDrag = False
            self.selected.emit({'tab': self, 'sync': doSync})

            revlens.GUI.hideMenuPanes()
            self.update()


        elif evt.rightButton():

            epos = evt.scenePosition()
            self.contextMenu.setPos(epos.x() + 20, epos.y())
            self.contextMenu.setVisible(not self.contextMenu.isVisible())


    def dragEnterEventItem(self, evt):
        self._inDrag = True
        evt.setAccepted(True)

        self.update()

    
    def dragLeaveEventItem(self, evt):
        self._inDrag = False
        evt.setAccepted(True)
        self.update()


    def dropEventItem(self, evt):

        self._inDrag = False

        md = evt.mimeData()

        # print(md.formats())

        pathList = None

        if md.hasFormat('text/json'):
            pathList = []
            mdPayload = json.loads(md.dataAsText('text/json'))
            mdTypes = mdPayload['typeMap']
            filenameKey = None
            if 'text/filename' in mdTypes:
                filenameKey = mdTypes['text/filename']

            print('---- DROP EVENT ----')
            # pprint.pprint(mdPayload)

            if filenameKey:
                if not mdPayload['selection']:
                    mdPayload['selection'] = [mdPayload['data']]


                for mdEntry in mdPayload['selection']:
                    mdData = mdPayload['data']
                    pathList.append({
                        'media.video': mdEntry[filenameKey],
                        'media.frame_count': mdEntry['frame_count'],
                        'media.name': mdEntry['name'],
                        'media.metadata': mdEntry['metadata']
                    })


        elif md.hasFormat('text/filename'):
            fsPath = md.dataAsText('text/filename')
            pathList = [{
                'media.video': fsPath,
                'media.audio': fsPath,
                'media.name': os.path.basename(fsPath),
                'media.frame_count': 48
            }]

        elif md.hasFormat('text/uri-list'):

            path = md.dataAsText('text/uri-list')
            pathList = []
            for entry in path.split():
                pathList.append(entry.replace('file://', ''))

        # print('drop event: got pathlist: {}'.format(pathList))
        if pathList:
    
            revlens.CNTRL.getMediaManager().loadMediaList(
                pathList,
                self.track.index(),
                "",
                self.track.getMaxFrameNum() + 1,
                False,
                True
            )

            # TODO FIXME: MOVE THIS SOMEWHERE ELSE?
            #
            if self.track.hasMetadataValue('playlist.id') and self.track.hasMetadataValue('playlist.name'):

                playlistId = self.track.metadataValue('playlist.id')
                playlistName = self.track.metadataValue('playlist.name')

                if playlistId and playlistName:
                    trackMediaList = self.track.getNodeFrameList()

                    updateIdList = []
                    for trackMediaInfo in trackMediaList:
                        nodeUuid = trackMediaInfo['node_uuid']
                        node = revlens.CNTRL.session().getNodeByUuid(nodeUuid)
                        if node:
                            nodeMd = node.getNodeProperty('media.metadata')
                            if 'id' in nodeMd:
                                updateIdList.append(nodeMd['id'])


                    # print(trackMediaList)
                    # print(updateIdList)

                    if len(trackMediaList) == len(updateIdList):

                        def _updateDone(result):
                            print('UPDATE DOEN')
                            print(result)

                        SD = ShowData.instance()
                        SD.dataSource('playlist').requestUpdatePlaylist(_updateDone,
                            SD.projectEntity['code'], playlistId, playlistName, updateIdList)






        # TODO FIXME: Cannot annotate after loading using this. WHY??
        #
        # mmgr = revlens.CNTRL.getMediaManager()

        # mediaIn = {
        #     'media.video': path,
        #     'media.audio': path,
        #     'media.name': os.path.basename(path)
        # }
        # result = mmgr.identifyMedia(mediaIn) # factory.identifyMedia2(mediaIn, revlens.NODE_DATA_VIDEO)
        # import pprint
        # pprint.pprint(result)
        # if result:
        #     node = mmgr.getMediaFactory().createMedia(result, True)
        #     # pprint.pprint(node.getProperties())
        #     self.track.insertNode(node, self.track.getMaxFrameNum() + 1)

        #     # revlens.cmds.goto_frame(revlens.cmds.get_current_frame_num())


        self.update()


    def onMediaRemoved(self, node, uuid, startFrame, emitSyncAction):
        self.select()


    def onContextMenuItemSelected(self, md):

        action = md['data']['action']

        if action == 'duplicate':
            from revlens_prod.panel.playlist.main import SessionPan

            trackName = self.track.name()

            trackMedia = self.track.getNodeFrameList()
            loadMedia = []
            for mediaEntry in trackMedia:
                node = revlens.CNTRL.session().getNodeByUuid(mediaEntry['node_uuid'])
                if node:
                    nodeSourceInfo = node.getNodeProperty('media.source_info')
                    if node.hasNodeProperty('media.metadata'):
                        nodeMd = node.getNodeProperty('media.metadata')
                        nodeSourceInfo['media.metadata'] = nodeMd
                    
                    loadMedia.append(nodeSourceInfo)

            pl = SessionPanel.instance()
            newTrack = pl.onAddTabRequested({'text': trackName})
            revlens.CNTRL.getMediaManager().loadMediaList(
                loadMedia,
                newTrack.index(),
                "",
                newTrack.getMaxFrameNum() + 1,
                False,
                True
            )


        if action == 'remove_current_item':
            revlens.gui.timeline.request_remove_item_at_current_frame(self.track)

        elif action == 'clear':
            revlens.gui.timeline.request_clear_track(self.track)

        elif action == 'rename':
            revlens.gui.timeline.request_rename_track(self.track)

        elif action == 'delete':
            revlens.gui.timeline.request_delete_track(self.track.uuid().toString())


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width - 15)


    def paintItem(self, painter):

        p = QtGui.QPen()
        
        selCol = QtGui.QColor(0, 0, 0)

        if self._inDrag:
            selCol = QtGui.QColor(200, 40, 40)
            p.setWidth(4)

        elif self._selected:
            selCol = QtGui.QColor(200, 200, 200)
            p.setWidth(4)

        else:
            p.setWidth(1)

        p.setColor(selCol)

        b = QtGui.QBrush(QtGui.QColor(60, 60, 60))

        painter.setPen(p)
        painter.setBrush(b)

        painter.drawRect(self.boundingRect())


