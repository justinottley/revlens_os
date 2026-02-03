
import os
import json
import pprint

from rlp import QtCore

import rlp.core as RlpCore
import rlp.gui as RlpGui
import rlp.util as RlpUtil

from rlp.core.net.websocket import RlpClient

import RlpProdGUImodule as RlpProd

import revlens
import revlens.cmds
import revlens.media
from revlens.keyboard import KeyBinding

import revlens.gui.timeline


from .track import TrackMediaData, TrackMediaWidget, TrackTab


KeyBinding.register(
    'b',
    'revlens_prod.cmds.toggle_bookends',
    'Bookend shots selected in Playlist'
)

KeyBinding.register(
    'pgup',
    'revlens_prod.cmds.goto_next_shot_in_playlist',
    'Goto Next Shot in Playlist'
)

KeyBinding.register(
    'pgdown',
    'revlens_prod.cmds.goto_previous_shot_in_playlist',
    'Goto Previous Shot in Playlist'
)


class SessionPanel(RlpGui.GUI_ItemBase):

    _INSTANCE = None

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._refresh_enabled = True

        self.update_timer = QtCore.PyTimer()
        self.update_timer.setSingleShot(True)
        self.update_timer.setInterval(5)

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":feather/refresh-cw.svg",
            self,
            20
        )
        self.refresh_button.buttonPressed.connect(self.refresh)


        self.add_tab_button = RlpGui.GUI_SvgButton(
            ':feather/plus-square.svg',
            self,
            20
        )
        self.add_tab_button.connectToMenu()

        add_tab_menu = self.add_tab_button.menu()
        add_tab_menu.addItem('Empty Track', {}, False)


        add_tab_menu.menuItemSelected.connect(self.onAddTabRequested)

        self.stickyBookendsButton = RlpGui.GUI_SvgButton(
            ':misc/cursor_sticky.svg', self, 20
        )
        self.stickyBookendsButton.setToolTipText('Toggle playback follows selection')
        self.stickyBookendsButton.setToggled(True)
        self.stickyBookendsButton.icon().setOutlined(True)
        self.stickyBookendsButton.buttonPressed.connect(self.onToggleStickyBookends)

        session = revlens.CNTRL.session()
        session.setProperty('bookends.sticky', True)

        self.rpane = RlpGui.GUI_Pane(self, "playlist_root", 2)
        self.rpane.split(False)
        self.rpane.setHeaderVisible(False, True)
        self.rpane.splitterPtr(0).setPosPercent(0.3, True)

        self.trackMedia = TrackMediaWidget(self.rpane.panePtr(1), self)


        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.setSpacing(10)

        self.toolbar_layout.addItem(self.refresh_button, 0)
        self.toolbar_layout.addItem(self.add_tab_button, 0)
        self.toolbar_layout.addItem(self.stickyBookendsButton, 0)

        self.tab_layout = RlpGui.GUI_VLayout(self) # .rpane.panePtr(0))

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.toolbar_layout, 0)
        self.layout.addItem(self.tab_layout, 0)

        self._tabs = []
        self._mediaList = {}
        self.__selectedTab = None


        revlens.CNTRL.sessionChanged.connect(self.onSessionChanged)
        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)
        revlens.CNTRL.getMediaManager().itemSelected.connect(self.onMediaItemSelected)

        self.session = revlens.CNTRL.session()
        self.session.trackAdded.connect(self.onTrackAdded)
        self.session.trackDeleted.connect(self.onTrackDeleted)
        self.session.trackDataChanged.connect(self.onTrackDataChanged)
        self.session.sessionCleared.connect(self.onSessionCleared)

        self.refresh()

        self.onParentSizeChangedItem(parent.width(), parent.height())

        SessionPanel._INSTANCE = self


    @classmethod
    def instance(cls):
        return cls._INSTANCE


    @property
    def selectedTab(self):
        return self.__selectedTab

    @property
    def selectedTrack(self):
        return self.__selectedTab.track


    def selectedMedia(self):
        if self.selUuid and self.selUuid in self._mediaList:
            return self.trackMedia.grid.fullSelectionValues()

        return []


    def selectedRows(self):
        if self.selUuid and self.selUuid in self._mediaList:
            return self.trackMedia.grid.fullSelectionRows()

        return []


    def selectPlaylistByName(self, playlistName):
        for tab in self._tabs:
            if tab.track.name() == playlistName:
                tab.select()
                tab.update()
                break


    def selectMediaByIdx(self, mediaIdx):
        self.trackMedia.grid.rowAt(mediaIdx).setRowSelected(True, RlpGui.GRID_SEL_REPLACE)


    def refresh(self, md=None):

        if not self._refresh_enabled:
            return

        print('REFRESH')
        self.tab_layout.clear()

        print(self.session.numTracks())

        # NOTE: ensure clear() is called on the layout first, or else the app will crash
        self._tabs = []
        self._mediaList = {}


        selTab = None
        for trackIdx in reversed(range(self.session.numTracks())):
            track = self.session.getTrackByIndex(trackIdx)

            print('{} {}'.format(track.name(), track.trackType()))
            if track.trackType() != 0: # Media tracks only
                continue

            # track.trackDataChanged.connect(self.onTrackDataChanged)

            trackTab = TrackTab(self.rpane.panePtr(0), track)
            trackTab.selected.connect(self.onTabSelected)
            self._tabs.append(trackTab)

            self.tab_layout.addItem(trackTab, 5)

            trackUuid = track.uuid().toString()
            trackPlaylistType = track.metadata().get('playlist_type')

            if trackUuid not in self._mediaList:
                print('{} CREATING NEW TRACKMEDIA OF TYPE {} - {}'.format(self, trackPlaylistType, track))

                # trackMediaCls = TrackMedia
                # if trackPlaylistType == 'Sequence':
                #     trackMediaCls = SequenceTrackMedia

                # elif trackPlaylistType == 'Reviewlist':
                #     trackMediaCls = ReviewlistTrackMedia
    
                # medialist = trackMediaCls(self.rpane.panePtr(1), track)
                # medialist.setVisible(False)

                trackMediaData = TrackMediaData(track)
                trackMediaData.mediaListReady.connect(self.onMediaListReady)

                self._mediaList[trackUuid] = trackMediaData

                selTab = trackTab

            self._mediaList[trackUuid].refresh()

        if selTab:
            self.__selectedTab = selTab
            selTab.select()
            selTab.update()


    def onToggleStickyBookends(self, md):
        print(md)
        
        newToggledVal = not md['toggled']
        self.stickyBookendsButton.setToggled(newToggledVal)
        self.stickyBookendsButton.icon().setOutlined(newToggledVal)
        self.stickyBookendsButton.icon().update()

        session = revlens.CNTRL.session()
        session.setProperty('bookends.sticky', newToggledVal)


    def onAddTabRequested(self, md):

        tabType = md['text']

        self._refresh_enabled = False

        trackName = '{} {}'.format(tabType, self.session.numTracks() + 1)
        trackUuid = QtCore.QUuid.createUuid()
        trackUuidStr = trackUuid.toString()

        track = self.session.addTrack(
            trackName, "Media", trackUuid, "", True
        )
        track.setMetadata('playlist_type', tabType)

        # if tabType == 'Sequence':
        #     trackMedia = SequenceTrackMedia(self.rpane.panePtr(1), track)

        # elif tabType == 'Reviewlist':
        #     trackMedia = ReviewlistTrackMedia(self.rpane.panePtr(1), track)

        # else:
        #     trackMedia = TrackMedia(self.rpane.panePtr(1), track)

        # trackMedia.setVisible(False)
        trackMediaData = TrackMediaData(track)

        self._mediaList[trackUuidStr] = trackMediaData

        self._refresh_enabled = True
        self.refresh()

        self.selectPlaylistByName(trackName)

        # HACK: REFRESH for right-aligned toolbar item...
        # Ideally natively support right-aligned items in HLayout
        self.onParentSizeChangedItem(self.width() - 1, self.height())
        self.onParentSizeChangedItem(self.width() + 1, self.height())

        return track


    def onTabSelected(self, md):


        selTab = md['tab']
        for tab in self._tabs:
            if tab != selTab:
                tab.setSelected(False, pushUpdate=False)

        self.selUuid = selTab.track.uuid().toString()
        self.__selectedTab = selTab

        if self.selUuid in self._mediaList:
            self._mediaList[self.selUuid].refresh()


        self.rpane.panePtr(1).update()

        # Update timeline UI visibility button state
        #
        tlViewNames = revlens.TL_CNTRL.viewNames()

        for tlViewName in tlViewNames:
            tlMgr = revlens.TL_CNTRL.getTimelineView(tlViewName).trackManager()
            for trackIdx in tlMgr.trackIndexes():

                tlTrack = tlMgr.getTrackByIndex(trackIdx)
                if tlTrack.trackType() != 0:
                    continue

                visButton = tlTrack.getButtonByName("visibility")

                toggled = tlTrack.uuid().toString() != self.selUuid
                visButton.setToggled(toggled)
                visButton.update()


        selTab.setSelected(True, pushUpdate=True)

        evtInfo = {'track_uuid': self.selUuid}
        revlens.CNTRL.emitNoticeEvent('playlist.changed', evtInfo)

        if 'sync' not in md or md['sync']:
            revlens.CNTRL.emitSyncAction('playlist.changed', evtInfo)



    def onSessionChanged(self, session):
        print('session changed')
        print(session)


    def onSessionCleared(self):
        self.refresh()


    def onTrackDataChanged(self, eventName, trackUuid, data):

        print('SessionPanel ontrackdatachanged: {}'.format(eventName))

        if eventName == 'media_added' and trackUuid in self._mediaList:
            self._mediaList[trackUuid].refresh()

        elif eventName == 'set_name':
            for tab in self._tabs:
                if tab.track.uuid().toString() == trackUuid:
                    tab.name_label.setText(data['value.new'])
                    tab.name_label.update()

                    break


    def onTrackAdded(self, uuidStr, trackName, trackType):
        print('TRACK ADDED')
        self.refresh()
        self.selectPlaylistByName(trackName)


    def onTrackDeleted(self, trackUuidStr):

        print('track deleted: {}'.format(trackUuidStr))
        if trackUuidStr in self._mediaList:
            print('deleting trackmedia obj')
            self._mediaList[trackUuidStr].disconnectPySlots()
            del self._mediaList[trackUuidStr]

        self.refresh()


    def onMediaListReady(self, md):

        def _cbUpdateTimer():

            modelData = RlpCore.CoreDs_ModelDataItem.makeItem()
            for entry in md['media']:
                
                
                # print('')
                # print('')
                row = RlpCore.CoreDs_ModelDataItem.makeItem()
                for modelKey, modelVal in entry.items():
                    row.insert(modelKey, modelVal)


                # TODO FIXME: DATA MASSAGING...
                if 'source_info' in entry:
                    row.insert('sg_path_to_frames', entry['source_info']['media.video'])
                    row.insert('frame_count', entry['source_info']['media.frame_count'])
                    row.insert('name', entry['source_info']['media.name'])


                row.setMimeType('__row__', 'text/json')
                row.setMimeTypeKey('__row__')
                row.setMimeData({
                    'typeMap': {
                        'text/filename': 'sg_path_to_frames'
                    },
                    'data': entry
                })

                modelData.append(row)


            self.trackMedia.grid.setModelData(modelData)
            self.trackMedia.statusText.setText('')

            # force refresh, items not displaying in some cases..
            self.trackMedia.onParentSizeChangedItem(self.width(), self.height())

        self.trackMedia.statusText.setText('Loading..')

        # For snappier UI refresh / Track switching in Session tool
        self.update_timer.connectTimeout(_cbUpdateTimer)
        self.update_timer.start()



    def onMediaItemSelected(self, itemInfo):

        print(itemInfo)

        trackUuid = itemInfo['track_uuid']
        print('Incoming Track: {} - my track: {}'.format(trackUuid, self.selectedTrack.uuid().toString()))

        if trackUuid != self.selectedTrack.uuid().toString():
            return

        if trackUuid not in self._mediaList:
            print('Warning: Unknown track UUID: {}'.format(trackUuid))

        mediaUuid = itemInfo['media_uuid']
        if mediaUuid in self._mediaList[trackUuid].rowUuidMap: # handleMediaItemSelected(itemInfo)

            rowIdx = self._mediaList[trackUuid].rowUuidMap[mediaUuid]
            row = self.trackMedia.grid.rowAt(rowIdx)
            row.setRowSelected(True, 0)

        nodeFrame = self.selectedTrack.getNodeFrameByUuid(mediaUuid)
        revlens.cmds.goto_frame(nodeFrame)


    def onNoticeEvent(self, evtName, evtInfo):

        if evtName == 'load_complete':
            print('LOAD COMPLETE')
            # print(evtInfo)
            trackIdx = evtInfo['track_idx']
            track = revlens.CNTRL.session().getTrackByIndex(trackIdx)
            trackUuidStr = track.uuid().toString()

            for tab in self._tabs:
                if tab.track.uuid().toString() == trackUuidStr and tab.isSelected():
                    self._mediaList[trackUuidStr].refresh()

                    revlens.CNTRL.emitNoticeEvent(
                        'playlist.changed', {'track_uuid': trackUuidStr})


        elif evtName == 'playlist.on_changed': # from sync

            print('PLAYLIST ON CHANGED')

            trackUuidStr = evtInfo['track_uuid']
            for tab in self._tabs:
                if tab.track.uuid().toString() == trackUuidStr:
                    tab.select(doSync=False)
                    break



    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)
        pass

        self.rpane.setWidth(width)
        self.rpane.setHeight(height)
        self.rpane.updatePanes(0)


def create(parent):
    return SessionPanel(parent)
