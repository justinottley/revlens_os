from rlp import QtCore
import rlp.gui as RlpGui

import revlens

from rlplug_qtbuiltin import RLQTGUI_CompositeModeButton


class DisplaySwitchWidget(RlpGui.GUI_ItemBase):

    def __init__(self, parent, displayIdx):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.compositeModeChanged = QtCore.PY_Signal(self)

        self.displayIdx = displayIdx

        self.label = RlpGui.GUI_Label(self, 'Disp: {}'.format(displayIdx))

        self.modeButton = RLQTGUI_CompositeModeButton(self)
        self.modeButton.menu().menuItemSelected.connect(self.onCompositeModeChanged)

        self.layout = RlpGui.GUI_HLayout(self)
        self.layout.addItem(self.label, 0)
        self.layout.addSpacer(15)
        self.layout.addItem(self.modeButton, 0)

        self.setWidth(200)
        self.setHeight(60)


    def onCompositeModeChanged(self, md):
        # print(md)
        md['display_idx'] = self.displayIdx
        self.compositeModeChanged.emit(md)
        self.modeButton.setText(md['text'])



class ClipSwitchPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        # self.compositeModeButton = RLQTGUI_CompositeTools(self)

        revlens.CNTRL.playStateChanged.connect(self.onPlayStateChanged)
        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)

        self.compositeNode = None
        self.displayWidgets = []

        self.clipLabel = RlpGui.GUI_Label(self, '-')
        self.dlayout = RlpGui.GUI_VLayout(self)

        self.layout = RlpGui.GUI_VLayout(self)
        # self.layout.addSpacer(10)
        self.layout.addItem(self.clipLabel, 0)
        self.layout.addItem(self.dlayout, 0)

        self.refreshDisplays()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def refreshDisplays(self):
        
        self.displayWidgets = []
        self.dlayout.clear()
        self.dlayout.addSpacer(10)

        for displayIdx in range(revlens.CNTRL.getVideoManager().getDisplayCount()):
            dw = DisplaySwitchWidget(self, displayIdx)
            dw.compositeModeChanged.connect(self.onCompositeModeChanged)
            self.displayWidgets.append(dw)
            self.dlayout.addItem(dw, 0)

        self.updateCurrentClip()


    def updateCurrentClip(self):

        # print(displayList)
        node = revlens.CNTRL.session().getNodeByFrame(revlens.CNTRL.currentFrameNum())
        if not node:
            return

        props = node.getProperties()
        compositeNode = props.get('graph.node.composite')
        if not compositeNode:
            print('WARNING: no composite node!')
            return

        self.compositeNode = compositeNode

        self.clipLabel.setText(props['media.name'])


    def onCompositeModeChanged(self, md):
        print(md)

        dIdx = md['display_idx']
        display = revlens.CNTRL.getVideoManager().getDisplayByIdx(dIdx)
        display.setDisplayMetadata("composite_mode", md['text'])

        currFrame = revlens.CNTRL.currentFrameNum()
        node = revlens.CNTRL.session().getNodeByFrame(currFrame)
        if not node:
            return

        node.setProperty("display.composite_mode", md['text'])


        vmgr = revlens.CNTRL.getVideoManager()
        cframe = vmgr.currentFrameNum()
        vmgr.clearAll(True)
        vmgr.updateToFrame(cframe, True, False)


    def onNoticeEvent(self, evtName, evtInfo):
        print(f'NOTICE EVENT: {evtName} {evtInfo}')
        if evtName == 'clip_changed':
            self.updateCurrentClip()


    def onPlayStateChanged(self, playState):
        print('PLAY STATE CHANGED: {}'.format(playState))


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)



def create(parent):
    return ClipSwitchPanel(parent)