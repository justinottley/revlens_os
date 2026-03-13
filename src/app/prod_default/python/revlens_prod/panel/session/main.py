
import pprint
import functools

from rlp import QtCore, QtGui

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


from .track_tab import TrackTab
from .track_media import TrackMediaWidget
from .media_data import TrackMediaData


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

    _trackMediaDataCls = TrackMediaData

    HUD_NAME = 'sginfo'

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._refresh_enabled = True

        self.updateSession = QtCore.PY_Signal(self)
        self.updateSession.connect(self.onSessionUpdateRequested)

        self.trackswitch_timer = QtCore.PyTimer()
        self.trackswitch_timer.setSingleShot(True)
        self.trackswitch_timer.setInterval(5)

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
        self.stickyBookendsButton.setToolTipText('Toggle playback follows selection (sticky bookends)')
        self.stickyBookendsButton.setToggled(True)
        self.stickyBookendsButton.icon().setOutlined(True)
        self.stickyBookendsButton.buttonPressed.connect(self.onToggleStickyBookends)

        self.hudButton = RlpGui.GUI_SvgButton(':misc/text.svg', self, 20)
        self.hudButton.setToolTipText('Toggle Heads Up Display')
        self.hudButton.buttonPressed.connect(self.onToggleHud)

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
        self.toolbar_layout.addItem(self.hudButton, 0)

        self.tab_layout = RlpGui.GUI_VLayout(self) # .rpane.panePtr(0))

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.toolbar_layout, 0)
        self.layout.addItem(self.tab_layout, 0)

        self._tabs = []
        self._mediaList = {}
        self.__selectedTab = None
        self.__pinnedTab = None


        revlens.CNTRL.sessionChanged.connect(self.onSessionChanged)
        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)
        revlens.CNTRL.getMediaManager().itemSelected.connect(self.onMediaItemSelected)

        # Maybe migrate to noticeEvent instead to reduce number of signals?
        revlens.CNTRL.getMediaManager().mediaDataReady.connect(self.onMediaDataReady)

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


    @classmethod
    def setTrackMediaData(cls, trackMediaDataCls):
        cls._trackMediaDataCls = trackMediaDataCls

    @property
    def selectedTab(self):
        return self.__selectedTab

    @property
    def pinnedTab(self):
        return self.__pinnedTab

    @property
    def selectedTrack(self):
        if self.__selectedTab:
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


    def selectMediaByIdx(self, mediaIdx):
        row = self.trackMedia.grid.rowAt(mediaIdx)
        if row:
            row.setRowSelected(True, RlpGui.GRID_SEL_REPLACE)


    def refresh(self, md=None):

        if not self._refresh_enabled:
            return

        # print('SESSION PANEL REFRESH')

        self.tab_layout.clear()

        # NOTE: ensure clear() is called on the layout first, or else the app will crash

        pinnedTrackUuid = None
        if self.pinnedTab:
            pinnedTrackUuid = self.pinnedTab.track.uuid().toString()

        self._tabs = []
        self._mediaList = {}


        selTab = None
        for trackIdx in reversed(range(self.session.numTracks())):
            track = self.session.getTrackByIndex(trackIdx)

            # print('{} {}'.format(track.name(), track.trackType()))
            if track.trackType() != 0: # Media tracks only
                continue

            # track.trackDataChanged.connect(self.onTrackDataChanged)

            trackTab = TrackTab(self, self.rpane.panePtr(0), track)
            trackTab.selected.connect(self.onTabSelected)
            trackTab.pinnedChanged.connect(self.onTabPinnedChanged)

            self._tabs.append(trackTab)

            self.tab_layout.addItem(trackTab, 5)

            trackUuid = track.uuid().toString()

            if pinnedTrackUuid == trackUuid:
                trackTab.setPinned(True, forceVisButton=True)
                self.__pinnedTab = trackTab


            if trackUuid not in self._mediaList:

                # print('{} CREATING NEW TRACKMEDIA - {} num items: {}'.format(self, track, track.mediaCount()))

                trackMediaData = self._trackMediaDataCls(track)
                trackMediaData.mediaListReady.connect(self.trackMedia.onMediaListReady)
                trackMediaData.mediaDataReady.connect(self.trackMedia.onMediaDataReady)

                self._mediaList[trackUuid] = trackMediaData

                selTab = trackTab

            # self._mediaList[trackUuid].refresh()

        # if selTab:
        #     self.__selectedTab = selTab
        #     selTab.select()
        #     selTab.update()


    def onToggleHud(self, md):
        print(md)

        if md['button'] == QtGui.LEFT_BUTTON:

            newValue = not md['toggled']
            self.hudButton.setToggled(newValue)
            self.hudButton.icon().setOutlined(newValue)
            self.hudButton.icon().update()

            import rlplug_qtbuiltin.hud
            rlplug_qtbuiltin.hud.setHudVisible(self.HUD_NAME, newValue)


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
        track.setMetadataValue('playlist_type', tabType)


        trackMediaData = self._trackMediaDataCls(track)
        trackMediaData.mediaListReady.connect(self.trackMedia.onMediaListReady)
        trackMediaData.mediaDataReady.connect(self.trackMedia.onMediaDataReady)

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


        self.rpane.panePtr(1).update()

        if self.pinnedTab is not None and selTab != self.pinnedTab:
            md['update_session'] = False


        # Go around the event loop for snappier feeling UI update
        def _requestSessionUpdate(md):
            self.updateSession.emit(md)

        self.trackswitch_timer.connectTimeout(
            functools.partial(_requestSessionUpdate, md))

        self.trackswitch_timer.start()



    def onSessionUpdateRequested(self, md):

        # pprint.pprint(md)

        selTab = md['tab']

        if self.selUuid in self._mediaList:

            if self.trackMedia.grid.hasStashedRowList(self.selUuid):
                self.trackMedia.grid.loadStashedRowList(self.selUuid)

            else:
                print('WARNING: NO STASHED ROW LIST')
                self._mediaList[self.selUuid].refresh()

        if md.get('update_session') is False: # explicit check as opposed to None
            return

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



        # Update enabled state for session tracks
        #
        session = revlens.CNTRL.session()
        for trackIdx in range(session.numTracks()):
            strack = session.getTrackByIndex(trackIdx)
            if strack.trackType() != revlens.TRACK_TYPE_MEDIA:
                continue
            if strack != selTab.track:
                strack.setEnabled(False, False)

        selTab.track.setEnabled(True, False)

        if session.getProperty('bookends.sticky'):

            # frameRange = self.track.getFrameRange()
            frameList = selTab.track.getFrames()
            if frameList:
                frameRange = [frameList[0], frameList[-1]]
                if frameRange[1] > 0:
                    session.resetNodeEnabled()
                    revlens.CNTRL.setInFrame(frameRange[0], False, True)
                    revlens.CNTRL.setOutFrame(frameRange[1], True, True)


        evtInfo = {'track_uuid': self.selUuid}
        revlens.CNTRL.emitNoticeEvent('playlist.changed', evtInfo)

        if 'sync' not in md or md['sync']:
            revlens.CNTRL.emitSyncAction('playlist.changed', evtInfo)

        # NOTE!! VERY VERY IMPORTANT!! ONLY CALL fullRefresh()
        # if a user actually selected a tab in the UI, otherwise
        # the app could crash here on Linux (e.g., on load)
        #
        if md.get('user_press'):
            revlens.CNTRL.fullRefresh()


    def onTabPinnedChanged(self, md):

        newPinnedTab = md['tab']

        for tab in self._tabs:
            if tab != newPinnedTab:
                tab.setPinned(False)

        if md['value']:

            newPinnedTab.setSelected(True)

            # Trigger onTabSelected to switch viewport
            if self.pinnedTab != newPinnedTab:
                self.__pinnedTab = None

        self.onTabSelected(md)

        # Set new value
        #
        if md['value']:
            self.__pinnedTab = md['tab']

        else:
            self.__pinnedTab = None

        


    def onSessionChanged(self, session):
        print('session changed')
        print(session)


    def onSessionCleared(self):
        self.refresh()


    def onTrackDataChanged(self, eventName, trackUuid, data):

        if eventName == 'media_added' and trackUuid in self._mediaList:
            if not self._mediaList[trackUuid].track.isLoading():
                self._mediaList[trackUuid].refresh()
            return


        if eventName == 'media_input_changed' and \
        trackUuid in self._mediaList:
            if not self._mediaList[trackUuid].track.isLoading():
                self._mediaList[trackUuid].refresh()
            return


        elif eventName == 'media_updated' and trackUuid in self._mediaList:
            evtType = data.get('type')
            if evtType == 'reorder':
                self._mediaList[trackUuid].refresh()

            return


        elif eventName == 'set_name':
            for tab in self._tabs:
                if tab.track.uuid().toString() == trackUuid:
                    tab.name_label.setText(data['value.new'])
                    tab.name_label.update()

                    break

            return


    def onTrackAdded(self, uuidStr, trackName, trackType):
        self.refresh()
        self.trackMedia.stash(uuidStr)
        self.selectPlaylistByName(trackName)
        self.trackMedia.refresh()


    def onTrackDeleted(self, trackUuidStr):

        if trackUuidStr in self._mediaList:
            self._mediaList[trackUuidStr].disconnectPySlots()
            del self._mediaList[trackUuidStr]

        self.refresh()


    def onMediaItemSelected(self, itemInfo):

        trackUuid = itemInfo['track_uuid']

        # print('Incoming Track: {} - my track: {}'.format(trackUuid, self.selectedTrack.uuid().toString()))

        if trackUuid != self.selectedTrack.uuid().toString():
            return

        if trackUuid not in self._mediaList:
            print('Warning: Unknown track UUID: {}'.format(trackUuid))

        mediaUuid = itemInfo['media_uuid']
        if mediaUuid in self._mediaList[trackUuid].rowUuidMap: # handleMediaItemSelected(itemInfo)

            rowIdx = self._mediaList[trackUuid].rowUuidMap[mediaUuid]
            row = self.trackMedia.grid.rowAt(rowIdx)
            if row:
                row.setRowSelected(True, 0)

        nodeFrame = self.selectedTrack.getNodeFrameByUuid(mediaUuid)
        revlens.cmds.goto_frame(nodeFrame)


    def onMediaDataReady(self, evtName, evtInfo):

        if evtName == 'media_list':
            trackUuid = evtInfo['track_uuid'].toString()
            if trackUuid in self._mediaList:
                self._mediaList[trackUuid].refresh()



    def onNoticeEvent(self, evtName, evtInfo):

        if evtName == 'load_complete':

            trackIdx = evtInfo['track_idx']
            track = revlens.CNTRL.session().getTrackByIndex(trackIdx)
            trackUuidStr = track.uuid().toString()

            for tab in self._tabs:
                if tab.track.uuid().toString() == trackUuidStr and tab.isSelected():

                    revlens.CNTRL.emitNoticeEvent(
                        'playlist.changed', {'track_uuid': trackUuidStr})


        elif evtName == 'playlist.on_changed': # from sync

            trackUuidStr = evtInfo['track_uuid']
            for tab in self._tabs:
                if tab.track.uuid().toString() == trackUuidStr:
                    tab.select(doSync=False)
                    break


        elif evtName == 'track.request_load':

            itemCount = evtInfo['item_count']
            self.trackMedia.statusText.setText('Loading {} Items..'.format(itemCount))



    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)
        pass

        self.rpane.setWidth(width)
        self.rpane.setHeight(height)
        self.rpane.updatePanes(0)


def create(parent):
    return SessionPanel(parent)
