
import os
import json
import pprint

from rlp import QtCore, QtGui

import rlp.util as RlpUtil
import rlp.core as RlpCore
import rlp.gui as RlpGui
import RlpProdGUImodule as RlpProd

from rlp.core.net.websocket import RlpClient

import revlens

import revlens_prod.cmds



class TrackMediaWidget(RlpGui.GUI_ItemBase):

    _handlerCls = None

    def __init__(self, parent, sessionPanel):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.currSelected = []

        # in cases where thumbnails arrive before grid is ready for them
        self._thumbUuidCache = {} 

        # self.currTrack = None
        self.sessionPanel = sessionPanel

        self.update_timer = QtCore.PyTimer()
        self.update_timer.setSingleShot(True)
        self.update_timer.setInterval(1)


        self.grid = RlpProd.GUI_PlaylistView(self)
        self.grid.setVerticalScrollbarPolicy(RlpGui.GRID_VSCROLLBAR_POLICY_PARENT_WIDTH)
        self.grid.scrollArea().setHScrollBarVisOffset(-1)
        # self.grid.requestAddToPlaylist.connect(self.onAddToPlaylistRequested)
        self.grid.setPos(0, 5)

        self.contextMenu = RlpGui.GUI_MenuPane(self)
        self.contextMenu.setVisible(False)
        self.contextMenu.menuShown.connect(self.onContextMenuShown)

        self.contextMenu.addItem('New Sub Track', {'action': 'new_sub_track'}, False)
        shotContextMenu = self.contextMenu.addItem('Context', {}, False)
        shotContextMenu.addItem('+/- 1', {'action': 'load_context', 'inc': 1})
        shotContextMenu.addItem('+/- 2', {'action': 'load_context', 'inc': 2})

        self.greyscaleControlButton = RlpGui.GUI_SvgButton(':misc/dark.svg', self, 25)
        self.greyscaleControlButton.setToolTipText('Toggle Greyscale for Context Shots')
        self.greyscaleControlButton.setVisible(False)

        self.statusText = RlpGui.GUI_Label(self, '')

        tbLayout = self.grid.toolbar().layout()
        tbLayout.setSpacing(5)
        tbLayout.addSpacer(5)
        tbLayout.addItem(self.greyscaleControlButton, 3)
        tbLayout.addItem(self.statusText, 4)

        self.handler = self.__class__._handlerCls(self)

        self.grid.updateHeader()

        self.grid.selectionChanged.connect(self.onGridSelectionChanged)
        self.grid.cellWidgetEvent.connect(self.onGridCellWidgetEvent)

        self.onParentSizeChangedItem(parent.width(), parent.height())



    @classmethod
    def setHandler(cls, handlerCls):
        print('Got TrackMediaWidget Handler: {}'.format(handlerCls))
        cls._handlerCls = handlerCls


    @property
    def edbc(self):
        return RlpClient.instance()


    @property
    def currTrack(self):
        return self.sessionPanel.selectedTrack


    def refresh(self):

        if not self.currTrack:
            return

        modelData = RlpCore.CoreDs_ModelDataItem.makeItem()
        self.grid.setModelData(modelData, '', False, False)


    def stash(self, uuidStr):
        if self.currTrack:
            self.grid.stashCurrentRowList(self.currTrack.uuid().toString())

        else:
            self.grid.setCurrStashedRowListName(uuidStr)

    def mousePressEventItem(self, event):
        self.grid.clearSelection()


    def onContextMenuShown(self, md):
        print(md)


    def onMediaListReady(self, md):

        # print(md)

        def _cbUpdateTimer():

            modelData = RlpCore.CoreDs_ModelDataItem.makeItem()
            for entry in md['media']:
                
                row = RlpCore.CoreDs_ModelDataItem.makeItem()
                for modelKey, modelVal in entry.items():
                    row.insert(modelKey, modelVal)

                # Merge thumbs from thumb cache that may have arrived
                # before we got here
                if 'uuid' in entry:
                    nodeUuid = entry['uuid']
                    if nodeUuid in self._thumbUuidCache:
                        image = self._thumbUuidCache[nodeUuid]
                        row.insert('image', image)

                        del self._thumbUuidCache[nodeUuid]


                # TODO FIXME: DATA MASSAGING...
                if 'source_info' in entry:

                    row.insert('sg_path_to_frames', entry['source_info'].get('media.video',''))
                    row.insert('frame_count', entry['source_info'].get('media.frame_count', 1))
                    row.insert('name', entry['source_info'].get('media.name', 'unknown'))


                row.setMimeType('__row__', 'text/json')
                row.setMimeTypeKey('__row__')
                row.setMimeData({
                    'typeMap': {
                        'text/filename': 'sg_path_to_frames'
                    },
                    'data': entry
                })


                rowWidgets = RlpCore.CoreDs_ModelDataItem.makeItem()
                rowWidgets.insert('media', {})

                row.insertItem('__widgets__', rowWidgets)

                modelData.append(row)


            trackUuidStr = self.currTrack.uuid().toString()
            self.grid.setModelData(modelData, trackUuidStr, False, True)
            self.statusText.setText('')

            # force refresh, items not displaying in some cases..
            self.onParentSizeChangedItem(self.width(), self.height())
            self.grid.forceRefresh()


        self.statusText.setText('Loading..')

        # For snappier UI refresh / Track switching in Session tool
        self.update_timer.connectTimeout(_cbUpdateTimer)
        self.update_timer.start()


    def onMediaDataReady(self, md):

        evtName = md['evt_name']
        evtInfo = md['evt_info']

        if evtName == 'thumbnail_ready':

            # pprint.pprint(evtInfo)

            image = None
            nodeUuidStr = evtInfo['graph.uuid'].toString()
            node = revlens.CNTRL.session().getNodeByUuid(nodeUuidStr)
            if node and node.hasNodeProperty('media.thumbnail.image_full'):
                image = node.getNodeProperty('media.thumbnail.image_full')


            rowIdx = evtInfo['row_idx']
            trackUuid = evtInfo['track_uuid']

            row = None
            if trackUuid == self.grid.currStashedRowList():
                row = self.grid.rowAt(rowIdx)

            else:
                row = self.grid.rowAtStashedRowList(trackUuid, rowIdx)

            if not row and image:

                # it could occur during load that the grid is actually
                # not built yet by the time this is available.
                # Stash the thumbnail for later retrieval in this case
                #
                self._thumbUuidCache[nodeUuidStr] = image
                return


            if image:
                cell = row.cell('image')
                cell.insertValue('value', image)
                cell.removeValue('image')
                cell.update()


    def onGridCellWidgetEvent(self, evtName, evtInfo):

        if (evtName == "cell.button_pressed"):

            if evtInfo['button'] == QtGui.LEFT_BUTTON:
                self.contextMenu.setVisible(False)

            elif evtInfo['button'] == QtGui.RIGHT_BUTTON:
                self.contextMenu.setPos(
                    evtInfo['scene_pos.x'],
                    evtInfo['scene_pos.y']
                )
                self.contextMenu.setVisible(True)


        elif (evtName == 'row.reorder_requested'):

            srcRowIdx = evtInfo['src_idx']
            destRowIdx = evtInfo['dest_idx']

            srcRow = self.grid.rowAt(srcRowIdx)
            srcRowData = srcRow.rowData().values()
            srcNodeUuid = srcRowData['uuid']

            srcNode = self.currTrack.getNodeByUuid(srcNodeUuid)
            if not srcNode:
                print('Error: invalid node at row {} uuid {}'.format(srcRowIdx, destNodeUuid))
                return

            destRow = self.grid.rowAt(destRowIdx)
            destRowData = destRow.rowData().values()
            destNodeUuid = destRowData['uuid']

            destNode = self.currTrack.getNodeByUuid(destNodeUuid)
            if not destNode:
                print('Invalid dest node at row {} uuid {}'.format(destRowIdx, destNodeUuid))
                return

            destNodeFrame = destNode.getNodeProperty('session.frame')
            self.currTrack.moveNodeToFrame(srcNode, srcRowIdx, destNodeFrame, destRowIdx)

        elif (evtName == 'row.insert_requested'):

            destRow = self.grid.rowAt(evtInfo['at_idx'])
            destRowData = destRow.rowData().values()
            destNodeUuid = destRowData['uuid']
            destNode = self.currTrack.getNodeByUuid(destNodeUuid)
            if not destNode:
                print('Invalid dest node at row {} uuid {}'.format(destRowIdx, destNodeUuid))
                return

            destNodeFrame = destNode.getNodeProperty('session.frame')

            if evtInfo['data'].get('sg_path_to_frames'):
                showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
                cbMediaDataFormatter = showDataObj.formatter('code')

                mediaName, mediaMd = cbMediaDataFormatter(evtInfo['data'], nameKey='code')
                mediaList = [{
                    'media.video': evtInfo['data']['sg_path_to_frames'],
                    'media.name': mediaName,
                    'media.frame_count': evtInfo['data']['frame_count'],
                    'media.metadata': mediaMd
                }]

                revlens.CNTRL.getMediaManager().loadMediaList(
                    mediaList, self.currTrack.index(), "", destNodeFrame, False, False
                )

            else:
                import revlens_prod.cmds
                revlens_prod.cmds.load_shots(self.currTrack, [evtInfo['data']], destNodeFrame)



    def onGridSelectionChanged(self, selMode, selInfo):
        # print('Selection: {}'.format(selInfo))

        if not self.currTrack:
            return

        selInfo = selInfo.toPy()

        # check to see if everything has been deselected
        if selMode == 'rem' and len(self.grid.fullSelectionValues()) == 0:
            selMode = 'set'
            selInfo = []


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
        self.setHeight(height)

        self.grid.setWidth(width)
        self.grid.setHeight(height - 40)

