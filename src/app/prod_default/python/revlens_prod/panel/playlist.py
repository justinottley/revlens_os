

from rlp import QtCore, QtGui
import rlp.gui as RlpGui

import revlens
import revlens.cmds
from revlens.keyboard import KeyBinding

_PANELS = []

KeyBinding.register(
    'b',
    'revlens_prod.cmds.bookendSelectedShots',
    'Bookend shots selected in Playlist'
)


class TrackMedia(RlpGui.GUI_ItemBase):

    def __init__(self, parent, track):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.track = track

        self.grid = RlpGui.GUI_GRID_View(self)
        colModel = self.grid.colModel()
        colModel.addCol({
            "col_name": "name",
            "display_name": "Name",
            "col_type": "str",
            "metadata": {},
            "width": 200
        }, -1)

        colModel.addCol({
            "col_name": "frame",
            "display_name": "Frame",
            "col_type": "str",
            "metadata": {},
            "width": 100
        }, -1)

        colModel.addCol({
            "col_name": "image",
            "display_name": "Thumbnail",
            "col_type": "image_b64",
            "metadata": {},
            "width": 80
        }, -1)

        self.grid.updateHeader()

        self.grid.selectionChanged.connect(self.onGridSelectionChanged)
        self.refresh()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onGridSelectionChanged(self, selInfo):
        frameNum = selInfo['frame']
        revlens.cmds.goto_frame(frameNum)


    def refresh(self):

        itemList = []
        nodeFrameList = self.track.getNodeFrameList()

        for trackItem in nodeFrameList:
            frameNum = trackItem['frame']
            node = self.track.getNodeByFrame(frameNum)
            nodeProps = node.getProperties()
            # print(trackItem)
            # print(nodeProps)
            gridData = {
                'name': nodeProps['media.name'],
                'frame': frameNum,
                'length': nodeProps['media.frame_count']
            }
            if 'media.thumbnail.data' in nodeProps:
                gridData['image'] = nodeProps['media.thumbnail.data']

            itemList.append(gridData)

        self.grid.setModelDataList(itemList)


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)

        self.grid.setWidth(width - 10)
        self.grid.setHeight(height)



class TrackTab(RlpGui.GUI_ItemBase):

    BASE_HEIGHT = 60

    def __init__(self, parent, track):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.selected = QtCore.PySignal()

        self.track = track

        self.name_label = RlpGui.GUI_Label(self, '')
        nlf = self.name_label.font()
        nlf.setPixelSize(nlf.pixelSize() + 2)
        nlf.setBold(True)
        self.name_label.setFont(nlf)
        self.name_label.setText(self.track.name())
        self.name_label.setPos(40, 10)

        self._selected = False

        self.setHeight(self.BASE_HEIGHT)
        self.onParentSizeChangedItem(parent.width(), parent.height())


    def setSelected(self, isSelected):
        self._selected = isSelected

        session = self.track.session()
        for trackIdx in range(session.numTracks()):
            strack = session.getTrackByIndex(trackIdx)
            if strack != self.track:
                strack.setEnabled(not isSelected, False)

        self.track.setEnabled(isSelected, False)

        revlens.CNTRL.onTrackDataChanged("set_enabled", "", {})
        self.update()


    def mousePressEventItem(self, evt):
        self._selected = True
        self.selected.emit({'tab': self})

        self.update()


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width - 15)


    def paintItem(self, painter):

        p = QtGui.QPen()
        
        selCol = QtGui.QColor(0, 0, 0)
        if self._selected:
            selCol = QtGui.QColor(200, 200, 200)
            p.setWidth(4)

        else:
            p.setWidth(1)

        p.setColor(selCol)

        b = QtGui.QBrush(QtGui.QColor(60, 60, 60))

        painter.setPen(p)
        painter.setBrush(b)

        painter.drawRect(self.boundingRect())



class PlaylistPanel(RlpGui.GUI_ItemBase):

    _INSTANCE = None

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":feather/lightgrey/refresh-cw.svg",
            parent,
            20
        )

        self.rpane = RlpGui.GUI_Pane(self, "playlist_root", 2)
        self.rpane.split(False)
        self.rpane.setHeaderVisible(False, True)
        self.rpane.splitterPtr(0).setPosPercent(0.3, True)

        # self.label = RlpGui.GUI_Label(self.rpane.panePtr(0), "Test")
        self.refresh_button.buttonPressed.connect(self.refresh)

        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.addSpacer(10)

        self.toolbar_layout.addItem(self.refresh_button, 0)
        self.toolbar_layout.addSpacer(10)

        self.tab_layout = RlpGui.GUI_VLayout(self) # .rpane.panePtr(0))

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.toolbar_layout, 0)
        self.layout.addItem(self.tab_layout, 0)

        self._tabs = []
        self._mediaList = {}

        revlens.CNTRL.sessionChanged.connect(self.onSessionChanged)

        self.session = revlens.CNTRL.session()
        self.session.trackAdded.connect(self.onTrackAdded)
        self.session.trackDeleted.connect(self.onTrackDeleted)
        self.session.trackDataChanged.connect(self.onTrackDataChanged)

        self.refresh()

        self.onParentSizeChangedItem(parent.width(), parent.height())

        PlaylistPanel._INSTANCE = self


    @classmethod
    def instance(cls):
        return cls._INSTANCE

    def selectedMedia(self):
        if self.selUuid and self.selUuid in self._mediaList:
            return self._mediaList[self.selUuid].grid.fullSelectionValues()

        return []


    def onTabSelected(self, md):
        print(md)

        selTab = md['tab']
        for tab in self._tabs:
            if tab != selTab:
                tab.setSelected(False)

        self.selUuid = selTab.track.uuid().toString()

        for trackUuid, trackMedia in self._mediaList.items():
            vis = trackUuid == self.selUuid
            trackMedia.setVisible(vis)

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




    def onSessionChanged(self, session):
        print('session changed')
        print(session)


    def onTrackDataChanged(self, eventName, trackUuid, data):

        print('PlaylistPanel ontrackdatachanged: {}'.format(eventName))

        if eventName == 'media_added' and trackUuid in self._mediaList:
            self._mediaList[trackUuid].refresh()

        elif eventName == 'set_name':
            for tab in self._tabs:
                if tab.track.uuid().toString() == trackUuid:
                    tab.name_label.setText(data['value.new'])
                    tab.name_label.update()

                    break




    def onTrackAdded(self, uuidStr, trackName, trackType):
        self.refresh()


    def onTrackDeleted(self, trackUuidStr):
        self.refresh()

    def refresh(self, md=None):
        
        self.tab_layout.clear()

        print(self.session.numTracks())
        # NOTE: ensure clear() is called on the layout first, or else the app will crash
        self._tabs = []

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
            if trackUuid not in self._mediaList:
                medialist = TrackMedia(self.rpane.panePtr(1), track)
                medialist.setVisible(False)

                self._mediaList[trackUuid] = medialist

            self._mediaList[trackUuid].refresh()



    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)
        pass

        self.rpane.setWidth(width)
        self.rpane.setHeight(height)
        self.rpane.updatePanes(0)


def create(parent):
    panel = PlaylistPanel(parent)
    _PANELS.append(panel)
    return panel