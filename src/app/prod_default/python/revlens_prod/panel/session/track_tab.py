
import os
import json
import traceback

from rlp import QtCore, QtGui

import rlp.core.util as rlp_core_util

import rlp.util as RlpUtil
import rlp.gui as RlpGui
from rlp.gui.style import GUI_Style

import revlens



class TrackTab(RlpGui.GUI_ItemBase):

    BASE_HEIGHT = 50

    def __init__(self, sessionPanel, parent, track):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.sessionPanel = sessionPanel

        self.setItemClipsChildren(True)
        self.setItemAcceptsDrops(True)
        self.setItemAcceptsHoverEvents(True)

        self.selected = QtCore.PY_Signal(self)
        self.pinnedChanged = QtCore.PY_Signal(self)

        self.track = track
        self.track.mediaRemoved.connect(self.onMediaRemoved)

        self.name_label = RlpGui.GUI_Label(self, '')
        nlf = self.name_label.font()
        nlf.setPixelSize(nlf.pixelSize() + 2)
        nlf.setBold(True)
        self.name_label.setFont(nlf)
        self.name_label.setText(self.track.name())
        self.name_label.setPos(30, 14)

        self.pinnedButton = RlpGui.GUI_SvgButton(':misc/pin.svg', self, 20)
        self.pinnedButton.setPos(10, 15)
        self.pinnedButton.buttonPressed.connect(self.onPinnedChanged)
        self.pinnedButton.setVisible(False)

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
        self._isPinned = False
        self._inDrag = False
        self._inHover = False

        self.setHeight(self.BASE_HEIGHT)
        self.onParentSizeChangedItem(parent.width(), parent.height())


    def isSelected(self):
        return self._selected


    def setSelected(self, isSelected, pushUpdate=None):
        self._selected = isSelected
        self.update()


    def select(self, doSync=True):
        '''
        Just for a clear API to select this tab
        '''
        self.mousePressEventItem(None, doSync=doSync)


    def isPinned(self):
        return self._isPinned

    def setPinned(self, isPinned, forceVisButton=False):

        self._isPinned = isPinned
        
        if self._isPinned:
            self.pinnedButton.icon().setBgColour(QtGui.QColor(200, 100, 100))
            self.pinnedButton.icon().setFgColour(QtGui.QColor(200, 100, 100))
            self.pinnedButton.icon().update()

            if forceVisButton:
                self.pinnedButton.setVisible(True)

        else:
            self.pinnedButton.icon().setBgColour(GUI_Style.IconBg)
            self.pinnedButton.icon().setFgColour(GUI_Style.IconBg)
            self.pinnedButton.icon().update()


        if not self._isPinned and not self._inHover and not self.pinnedButton.inHover():
            self.pinnedButton.setVisible(False)


    def onPinnedChanged(self, md):

        self.setPinned(not self._isPinned)

        self.pinnedChanged.emit({
            'tab': self,
            'track_uuid': self.track.uuid().toString(),
            'value': self._isPinned
        })


    def mousePressEventItem(self, evt, doSync=True):

        if not evt or evt.leftButton():

            self._selected = True
            self._inDrag = False

            revlens.GUI.hideMenuPanes()
            self.update()

            # We need user_press to prevent crashes on linux
            userPress = evt is not None
            self.selected.emit({
                'tab': self,
                'sync': doSync,
                'user_press': userPress
            })


        elif evt.rightButton():

            epos = evt.scenePosition()
            self.contextMenu.setPos(epos.x() + 20, epos.y())
            self.contextMenu.setVisible(not self.contextMenu.isVisible())


    def hoverEnterEventItem(self, evt):
        self._inHover = True
        self.pinnedButton.setVisible(True)


    def hoverLeaveEventItem(self, evt):
        self._inHover = False
        if not self.pinnedButton.inHover() and not self._isPinned:
            self.pinnedButton.setVisible(False)


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

        # print('DROP EVENT')
        md = evt.mimeData()

        # print(md.formats())

        pathList = None

        if md.hasFormat('text/json'):
            pathList = []
            mdPayload = json.loads(md.dataAsText('text/json'))
            mdTypes = mdPayload['typeMap']
            filenameKey = None
            loadCallback = None
            if 'text/filename' in mdTypes:
                filenameKey = mdTypes['text/filename']

            if 'func/callback' in mdTypes:
                loadCallback = mdTypes['func/callback']


            # print('---- DROP EVENT ----')
            # import pprint
            # pprint.pprint(mdPayload)

            showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
            cbMediaDataFormatter = showDataObj.formatter('code')

            if not mdPayload['selection']:
                mdPayload['selection'] = [mdPayload['data']]

            if filenameKey:
                for mdEntry in mdPayload['selection']:

                    mediaName, mediaMd = cbMediaDataFormatter(mdEntry)

                    # print('')
                    # print('got: {} \n{}'.format(mediaName, pprint.pformat(mediaMd)))

                    # mdData = mdPayload['data']

                    pathList.append({
                        'media.video': mdEntry[filenameKey],
                        'media.frame_count': mdEntry['frame_count'],
                        'media.name': mediaName, # mdEntry['name'],
                        'media.metadata': mediaMd # mdEntry['metadata']
                    })

            elif loadCallback:
                try:
                    
                    revlens.CNTRL.emitNoticeEvent(
                        'track.request_load',
                        {'item_count': len(mdPayload['selection'])}
                    )

                    func = rlp_core_util.import_function(loadCallback)
                    func(self.track, mdPayload['selection'])

                except:
                    print('Error running load callback')
                    print(traceback.format_exc())


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
                            print('Update Done')
                            print(result)

                        showDataObj = RlpUtil.APPGLOBALS.value('prod_data').getShowData()
                        showDataObj.dataSource('playlist').requestUpdatePlaylist(_updateDone,
                            showDataObj.projectEntity['code'], playlistId, playlistName, updateIdList)






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

            newTrack = self._sessionPanel.onAddTabRequested({'text': trackName})
            revlens.CNTRL.getMediaManager().loadMediaList(
                loadMedia,
                newTrack.index(),
                "",
                newTrack.getMaxFrameNum() + 1,
                False,
                True
            )

        elif action == 'remove_current_item':
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


