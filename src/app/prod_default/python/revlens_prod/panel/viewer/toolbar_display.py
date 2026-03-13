
import os

from rlp import QtCore, QtGui
from rlp.Qt import QStr

import rlp.gui as RlpGui
from rlp.gui.expandable import GUI_ExpandableWidget
from rlp.gui.circlebutton import GUI_CircleButton

import revlens
import rlplug_qtbuiltin.hud
from rlplug_qtbuiltin import RLQTGUI_ViewerIconButton, RLQTGUI_CompositeModeButton
import revlens_prod
import revlens_prod.cmds


class ViewerToolWidget(GUI_ExpandableWidget):

    FIXED_WIDTH = 210
    EXPAND_HEIGHT = 140

    def __init__(self, parent, label):
        GUI_ExpandableWidget.__init__(self, parent, label,
            iconSize=20, baseHeight=40, labelXPos=30, labelYPos=10, buttonYPos=10)

        self.setOpacity(0.9)

        self.tools = RlpGui.GUI_MenuPane(self)
        self.tools.addWidgetItem(self.slayout)
        # self.slayout.setParentItem(self.tools)
        self.tools.setVisible(False)




    @property
    def widgetHeight(self):
        return self.baseHeight


    def onToggleExpandRequested(self, md=None):
        if self._destructed:
            return

        self._toggled = True

        self.toggle_expand_button.setVisible(False)
        self.toggle_collapse_button.setVisible(True)

        p = self.pos()

        self.slayout.setVisible(True)
        self.tools.setPos(p.x(), p.y() + self.height() + 50)
        self.tools.setVisible(True)

        self.tools.setWidth(self.slayout.width())
        self.tools.setHeight(self.slayout.height())


    def onToggleCollapseRequested(self, md=None):

        self._toggled = False
        self.toggle_expand_button.setVisible(True)
        self.toggle_collapse_button.setVisible(False)
        self.slayout.setVisible(False)
        self.tools.setVisible(False)


    def onParentSizeChangedItem(self, width, height):

        if self._destructed:
            return

        self.setWidth(self.FIXED_WIDTH)


class ControlsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.label = RlpGui.GUI_Label(self, "Controls")


class StreamOverlayWidget(RlpGui.GUI_ItemBase):

    def __init__(self, vidSrcWidget):
        RlpGui.GUI_ItemBase.__init__(self, vidSrcWidget.viewer)

        self._vidSrcWidget = vidSrcWidget
        self._viewerPane = vidSrcWidget.viewer
        self._viewerPane.toolbarVisUpdated.connect(self.updateDimensions)

        self._rawMsg = 'Nothing Loaded'
        self._msg = QStr(self._rawMsg)

        self.updateDimensions()


    def setMsg(self, msg):
        self._rawMsg = msg
        self._msg = QStr(self._rawMsg)
        self.update()


    def mousePressEventItem(self, evt):
        evt.setAccepted(False) # pass event down / on


    def updateDimensions(self):

        yoff = self._viewerPane.toolbarYPosOffset()
        hoff = self._viewerPane.toolbarHeightOffset()

        if os.getenv('RLP_APP_PLATFORM') == 'ios':
            yoff += 20
            hoff -= 20

        self.setPos(0, yoff)
        self.setWidth(self._viewerPane.width())
        self.setHeight(self._viewerPane.height() - hoff)


    def paintItem(self, painter):
        
        col = QtGui.QColor(60, 60, 60)
        p = QtGui.QPen(col)
        painter.setPen(p)
        painter.setBrush(QtGui.QBrush(col))
        painter.drawRect(self.boundingRect())

        pWhite = QtGui.QPen(QtGui.QColor(220, 220, 220))
        f = painter.font()
        f.setBold(True)
        f.setPixelSize(30)

        xoff = 120
        if len(self._rawMsg) > 18:
            xoff = 320

        painter.setFont(f)
        painter.setPen(pWhite)
        painter.drawText(
            int((self.width()) / 2 - xoff),
            int((self.height() / 2) - 30),
            self._msg
        )


class VideoSourceWidget(ViewerToolWidget):

    FIXED_WIDTH = 220

    def __init__(self, parent, viewer):
        ViewerToolWidget.__init__(self, parent, 'Source:')

        self.viewer = viewer

        self.streamOverlay = StreamOverlayWidget(self)
        # self.streamOverlay.setPos(display.position().x(), display.position().y())
        self.streamOverlay.setVisible(False)

        self.videoSourceSelector = RLQTGUI_ViewerIconButton(self, '', 15)
        self.videoSourceSelector.setTextYOffset(0)
        self.videoSourceSelector.setPos(90, self._labelYPos - 2)
        self.videoSourceSelector.setOutlined(True)
        self.videoSourceSelector.setSvgIconPath(
            ":/feather/chevron-down.svg", 20, True)
        vsMenu = self.videoSourceSelector.menu()
        vsMenu.menuItemSelected.connect(self.onVideoSourceSelected)

        ri = self.videoSourceSelector.svgIcon()
        ri.setPos(ri.x(), 3)

        self.playPauseStreamButton = RlpGui.GUI_ToggleIconButton(
            self,
            ':feather/play-circle.svg',
            ':feather/stop-circle.svg',
            20
        )
        self.playPauseStreamButton.setHeight(22)
        self.playPauseStreamButton.setWidth(22)
        self.playPauseStreamButton.setPos(190, self._labelYPos)
        self.playPauseStreamButton.setVisible(False)
        self.playPauseStreamButton.buttonPressed.connect(self.onPlayPauseToggled)

        self.initSources()


    @property
    def streamCntrl(self):
        return revlens_prod.CNTRL_STREAMINGCONTROLLER_OBJ


    def initSources(self):

        vsMenu = self.videoSourceSelector.menu()
        vsMenu.clear()

        vidSources = revlens.CNTRL.getAllVideoSources()

        idx = 1
        for vidSource in vidSources:
            md = {'name': vidSource, 'idx': idx}
            vsMenu.addItem(vidSource, md, False)
            idx += 1

        self.videoSourceSelector.setText(vidSources[0])


    def onPlayPauseToggled(self, md):

        if self.playPauseStreamButton.isToggled():

            streamUrl = self.streamCntrl.streamUrl()
            streamInfo = self.streamCntrl.streamInfo()

            if 'user' in streamInfo and 'host' in streamInfo and 'screen' in streamInfo:
                self.streamCntrl.loadStream(streamUrl, streamInfo)

                self.streamOverlay.setMsg('Loading Stream: {} @ {} : {} ..'.format(
                    streamInfo['user'], streamInfo['host'], streamInfo['screen']
                ))

        else:
            self.streamCntrl.stopStream()
            self.streamOverlay.setMsg('Nothing Loaded')
            self.streamOverlay.setVisible(True)


    def onVideoSourceSelected(self, md):

        annMediaPlug = self.viewer.view().getEventPluginByName("RlpAnnotationDisplayMedia")
        annStreamPlug = self.viewer.view().getEventPluginByName("RlpAnnotationDisplayStreaming")

        self.videoSourceSelector.setText(md['text'])
        self.viewer.view().setVideoSourceIdx(md['data']['idx'])

        if md['text'] == 'Stream':

            annMediaPlug.setEnabled(False)
            annStreamPlug.setEnabled(True)

            self.playPauseStreamButton.setVisible(True)

            pbState = self.streamCntrl.getPlaybackState()
            if pbState != revlens.PLAYSTATE_PLAYING:
                self.streamOverlay.setVisible(True)

        else:

            annMediaPlug.setEnabled(True)
            annStreamPlug.setEnabled(False)

            self.streamOverlay.setVisible(False)
            self.playPauseStreamButton.setVisible(False)


    def handleNoticeEvent(self, evtSubTool, evtInfo):

        if evtSubTool == 'source_created':
            self.initSources()

        elif evtSubTool == 'stream_load_requested':

            if revlens.CNTRL.getPlaybackState() == revlens.PLAYSTATE_PAUSED:
                itemData = self.videoSourceSelector.menu().item('Stream').data()
                self.onVideoSourceSelected({'text': 'Stream', 'data': itemData})

            self.streamOverlay.setMsg('Loading Stream: {} @ {} : {} ..'.format(
                evtInfo['user'], evtInfo['host'], evtInfo['screen']
            ))


        elif evtSubTool == 'stream_started':

            self.streamOverlay.setVisible(False)
            self.playPauseStreamButton.setToggled(True)
            self.playPauseStreamButton.update()

        elif evtSubTool == 'stream_stopped':

            self.streamCntrl.stopStream()
            self.streamOverlay.setMsg('Stream Stopped')

            if self.videoSourceSelector.text() == 'Stream':
                self.streamOverlay.setVisible(True)



class ZoomPanWidget(ViewerToolWidget):

    def __init__(self, parent, display=None):

        self.resetButton = None

        ViewerToolWidget.__init__(self, parent, '')

        self.display = display

        self.zoomButton = RlpGui.GUI_SvgButton(':misc/zoom-in.svg', self, 25)
        self.zoomButton.setText('Zoom:')

        self.panButton = RlpGui.GUI_SvgButton(':feather/move.svg', self, 25)
        self.panButton.setText('Pan:')

        self.zoomLabel = RlpGui.GUI_Label(self, 'Zoom:')
        self.zoomLabel.setPos(self._labelXPos, self._labelYPos)
        
        f = self.zoomLabel.font()
        f.setPixelSize(12)

        self.zoomValue = RlpGui.GUI_Label(self, '0.3')
        self.zoomValue.setPos(self._labelXPos + 40, self._labelYPos)

        lf = self.zoomValue.font()
        lf.setPixelSize(12)
        lf.setBold(True)

        self.panLabel = RlpGui.GUI_Label(self, 'Pan:')
        self.panLabel.setPos(self._labelXPos + 70, self._labelYPos)

        self.panValue = RlpGui.GUI_Label(self, '0.0')
        self.panValue.setPos(self._labelXPos + 100, self._labelYPos)

        self.zoomLabel.setFont(f)
        self.zoomValue.setFont(lf)
        self.panLabel.setFont(f)
        self.panValue.setFont(lf)

        self.resetButton = RlpGui.GUI_SvgButton(':feather/refresh-ccw.svg', self, 18)
        self.resetButton.buttonPressed.connect(self.onResetZoomPanRequested)
        self.resetButton.setPos(self.FIXED_WIDTH - 30, 12)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.setPos(40, 0)
        self.layout.addItem(self.zoomButton, 20)
        self.layout.addItem(self.panButton, 20)

        self.slayout.addItem(self.layout, 0)

        self.initDisplayConnections()


    def initDisplay(self, display):
        display.displayZoomEnd.connect(self.onZoomPosChanged)
        display.displayPanEnd.connect(self.onPanPosChanged)


    def initDisplayConnections(self):

        if self.display:
            self.initDisplay(self.display)
            return

        vmgr = revlens.CNTRL.getVideoManager()
    
        for dIdx in range(vmgr.getDisplayCount()):
            display = revlens.CNTRL.getVideoManager().getDisplayByIdx(dIdx)
            self.initDisplay(display)

        vmgr.displayRegistered.connect(self.initDisplay)


    def onZoomPosChanged(self, zoomPos):
        self.zoomValue.setText(str(round(zoomPos, 2)))
        self.zoomValue.update()


    def onPanPosChanged(self, pan):
        self.panValue.setText('{} {}'.format(
            round(pan.x(), 2),
            round(pan.y(), 2)
        ))
        self.panValue.update()


    def onResetZoomPanRequested(self, md):

        for dIdx in range(revlens.CNTRL.getVideoManager().getDisplayCount()):
            display = revlens.CNTRL.getVideoManager().getDisplayByIdx(dIdx)
            display.resetZoom()
            display.resetPan()
            display.update()


    # def onParentSizeChangedItem(self, width, height):
    #     super().onParentSizeChangedItem(width, height)

    #     if self.resetButton:
    #         self.resetButton.setPos(self.FIXED_WIDTH - 5, 18)


class HUDWidget(ViewerToolWidget):

    def __init__(self, parent, display):

        self.toggle = None

        ViewerToolWidget.__init__(self, parent, 'HUD:')

        self.display = display
        self._currHud = None

        self.hudSelector = RLQTGUI_ViewerIconButton(self, '', 15)
        self.hudSelector.setTextYOffset(0)
        self.hudSelector.setText('No HUD')
        self.hudSelector.setPos(80, self._labelYPos - 2)
        self.hudSelector.setOutlined(True)
        self.hudSelector.setSvgIconPath(
            ":/feather/chevron-down.svg", 20, True)
        
        ri = self.hudSelector.svgIcon()
        ri.setPos(ri.x(), 3)

        hudCntrl = rlplug_qtbuiltin.RLQTDISP_HUD_OBJ
        hudConfigs = hudCntrl.getHudConfig().keys()

        menu = self.hudSelector.menu()
        menu.menuItemSelected.connect(self.onHudSelected)

        for hudConfigName in hudConfigs:
            menu.addItem(hudConfigName.capitalize(), {'hud': hudConfigName}, False)

        # menu.addItem('No HUD', {'hud': None}, False)
        # menu.addItem('Frame', {'hud': 'min'}, False)
        # menu.addItem('Media Info', {'hud': 'metadata'}, False)
        # menu.addItem('ShotGrid', {'hud': 'sginfo'}, False)

        self.toggle = RlpGui.GUI_RadioButton(self, '', 25, 0)
        self.toggle.buttonPressed.connect(self.onHudVisibilityToggled)


    @property
    def hudCntrl(self):
        return rlplug_qtbuiltin.RLQTDISP_HUD_OBJ


    def onHudSelected(self, md):

        # if self._currHud:   
        #    self.hudCntrl.setHudVisible(self.display.uuid(), self._currHud, False)

        hudName = md['data']['hud']

        # if md['text'] == 'Media Info' and not self.hudCntrl.hasHud(self.display.uuid(), hudName):
        #     print('UPDATE METADATA HUD!!!!')
        #     rlplug_qtbuiltin.hud.updateMetadataHud()

        # if self.toggle.isToggled():
        
        isVis = self.hudCntrl.isHudVisible(self.display.uuid(), hudName)
        newVis = not isVis

        # print('{} {} {}'.format(self.display.uuid(), isVis, newVis))

        self.hudCntrl.setHudVisible(self.display.uuid(), hudName, newVis)

        self.hudSelector.setText(md['text'])
        self.hudSelector.update()

        self._currHud = md['data']['hud']


    def onHudVisibilityToggled(self, md):
        self.toggle.setToggled(not self.toggle.isToggled())

        if self._currHud:
            self.hudCntrl.setHudVisible(self.display.uuid(), self._currHud, self.toggle.isToggled())


    def handleNoticeEvent(self, evtName, evtInfo):

        dcount = revlens.CNTRL.getVideoManager().getDisplayCount()
        dfirst = revlens.CNTRL.getVideoManager().getDisplayByIdx(0)

        action = evtInfo.get('action')
        if not action:
            print('Error: no action, got {}'.format(evtInfo))
            return

        if action == 'set':
            hudName = evtInfo['hud_name']
            hudVis = evtInfo['visibility']

            if dcount > 1:
                if (dfirst.uuid() != self.display.uuid()):
                    print('Set HUD visibility: {} {}'.format(hudName, hudVis))
                    self.hudCntrl.setHudVisible(self.display.uuid(), hudName, hudVis)

                else:
                    self.hudCntrl.setHudVisible(self.display.uuid(), hudName, False)
            else:
                print('MADE IT HERE')
                self.hudCntrl.setHudVisible(self.display.uuid(), hudName, hudVis)

        elif evtName == 'toggle_vis':

            hudName = 'threepio'
            hudVis = self.hudCntrl.isHudVisible(self.display.uuid(), hudName)
            self.hudCntrl.setHudVisible(self.display.uuid(), hudName, not hudVis)

            # if dcount > 1:
            #     if (dfirst.uuid() != self.display.uuid()):
            #         self.hudCntrl.setHudVisible(self.display.uuid(), hudName, not hudVis)
            
            # else:
            #     self.hudCntrl.setHudVisible(self.display.uuid(), hudName, not hudVis)


    def onParentSizeChangedItem(self, width, height):
        super().onParentSizeChangedItem(width, height)

        if self.toggle:
            self.toggle.setPos(width - 65, 18)


class MediaSelectorWidget(RlpGui.GUI_ItemBase):

    def __init__(self, parent, mediaInput):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.mediaInput = mediaInput

        self.toggleChanged = QtCore.PY_Signal(self)

        self.toggleButton = GUI_CircleButton(self)
        self.toggleButton.toggled = True
        self.toggleButton.toggleChanged.connect(self.onToggleChanged)

        self.text = RlpGui.GUI_Label(self, mediaInput['name'])
        f = self.text.font()
        f.setPixelSize(12)
        f.setBold(True)
        self.text.setFont(f)
        self.layout = RlpGui.GUI_HLayout(self)
        self.layout.addSpacer(10)
        self.layout.addItem(self.toggleButton, 0)
        self.layout.addItem(self.text, -4)

        self.setWidth(int(self.text.width() * 1.1) + 20)
        self.setHeight(25)


    def onToggleChanged(self, md):
        md.update(self.mediaInput)
        self.toggleChanged.emit(md)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(int(self.text.width() * 1.1) + 20) # Hackity hack hack hack
        self.setHeight(25)


class CompositeModeWidget(ViewerToolWidget):

    FIXED_WIDTH = 220

    def __init__(self, parent, display):
        ViewerToolWidget.__init__(self, parent, 'Composite:')

        self.display = display

        self._currNode = None

        self.modeButton = RLQTGUI_CompositeModeButton(self)
        self.modeButton.menu().menuItemSelected.connect(self.onCompositeModeChanged)
        self.modeButton.setPos(120, self._labelYPos - 2)

        self.compositeMedia = []
        self.slayout.setVSpacing(0)

        # TODO FIXME: temp, move to separate plugin!!
        self.currEyeStr = 'left'

        revlens.CNTRL.session().trackDataChanged.connect(self.onTrackDataChanged)



    def updateCompositeMediaList(self):


        self.slayout.clear()
        self.compositeMedia = []


        currFrame = revlens.CNTRL.currentFrameNum()
        node = revlens.CNTRL.session().getNodeByFrame(currFrame)
        if not node:
            return

        compositeNode = node.getNodeProperty('graph.node.composite')
        if not compositeNode:
            return

        compositeInputs = compositeNode.getInputInfo()

        inIdx = 0
        for mediaInput in compositeInputs:
            mediaEntry = MediaSelectorWidget(self, mediaInput)
            mediaEntry.toggleChanged.connect(self.onMediaToggled)
            self.compositeMedia.append(mediaEntry)
            self.slayout.addItem(mediaEntry, 5)

            self.display.setDisplayMetadata(
                'node.composite.{}.enabled'.format(inIdx),
                True
            )
            inIdx+= 1

        
        self.slayout.setHeight(len(compositeInputs) * 30)
        self.slayout.setWidth(self.slayout.itemWidth() + 40)


    def mousePressEventItem(self, evt):
        print('PRESS')
        pass


    def onCompositeModeChanged(self, md):

        print(md)

        newCompositeMode = md['text']
        self.modeButton.setText(newCompositeMode)

        self.display.setCompositeMode(newCompositeMode)
        self.display.clearDisplayMetadata('node.composite')

        self.updateCompositeMediaList()


        vmgr = revlens.CNTRL.getVideoManager()
        cframe = vmgr.currentFrameNum()
        vmgr.clearAll(True)
        vmgr.updateToFrame(cframe, True, False)


    def toggleEyeEnabled(self):

        print('Toggling eye, current: {}'.format(self.currEyeStr))

        # TODO FIXME: assumes particular setup for composite

        for bIdx in range(3):
            self.display.setDisplayMetadata(
                'node.composite.{}.enabled'.format(bIdx + 1),
                False
            )

        if self.currEyeStr == 'left':
            self.display.setDisplayMetadata(
                'node.composite.2.enabled',
                True)
            self.currEyeStr = 'right'
            
        elif self.currEyeStr == 'right':
            self.display.setDisplayMetadata(
                'node.composite.1.enabled',
                True)
            self.currEyeStr = 'left'


    def onMediaToggled(self, md):
        print('MEDIA TOGGLED')
        print(md)

        self.display.setDisplayMetadata(
            'node.composite.{}.enabled'.format(md['idx']),
            md['val']
        )


    def onTrackDataChanged(self, evtName, trackUuidStr, evtInfo):

        if evtName == 'media_input_changed':
            self._currNode = None
            self.updateCompositeMediaList()


    def handleNoticeEvent(self, evtName, evtInfo):


        if evtName == 'clip_changed':
            self.updateCompositeMediaList()
            return


        elif evtName == 'frame_changed':
            
            currFrame = revlens.CNTRL.currentFrameNum()
            node = revlens.CNTRL.session().getNodeByFrame(currFrame)
            if self._currNode != node:

                self._currNode = node
                self.updateCompositeMediaList()


        action = evtInfo.get('action')
        if not action:
            # print('Error: no "action" in event payload, got {}'.format(evtInfo))
            return


        elif action == 'set_mode':
            self.onCompositeModeChanged({'text': evtInfo['composite_mode']})

        elif action == 'toggle_eye':

            # TODO FIXME: hack to only toggle for first display, fix to use under cursor?
            dcount = revlens.CNTRL.getVideoManager().getDisplayCount()
            if dcount > 1:
                dfirst = revlens.CNTRL.getVideoManager().getDisplayByIdx(0)
                if (dfirst.uuid() == self.display.uuid()):
                    self.toggleEyeEnabled()

                else:
                    self.display.setDisplayMetadata('node.composite.0.enabled', True)
                    self.display.setDisplayMetadata('node.composite.1.enabled', False)
                    self.display.setDisplayMetadata('node.composite.2.enabled', False)

            else:
                self.toggleEyeEnabled()


class ViewerControlsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, viewer):
        RlpGui.GUI_ItemBase.__init__(self, viewer)

        # Doesn't work?
        # self.setItemClipsChildren(False)

        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)

        self._display = viewer.view()

        self.snapshotButton = RlpGui.GUI_SvgButton(':feather/camera.svg', self, 20)
        self.snapshotButton.setToolTipText('Take a Snapshot')
        self.snapshotButton.buttonPressed.connect(self.onSnapshotRequested)

        self.clearButton = RlpGui.GUI_SvgButton(':misc/clean.svg', self, 20)
        self.clearButton.setToolTipText('Clear Annotations')
        self.clearButton.buttonPressed.connect(self.onClearRequested)

        # self.zoomPanGroup = ZoomPanWidget(self, self._display)
        self.sourceGroup = VideoSourceWidget(self, viewer)
        self.hudGroup = HUDWidget(self, self._display)
        self.compositeGroup = CompositeModeWidget(self, self._display)
        self.compositeGroup.toggleChanged.connect(self.onToolToggleChanged)


        self.layout = RlpGui.GUI_HLayout(self)
        self.layout.addSpacer(10)
        self.layout.addItem(self.snapshotButton, 10)
        self.layout.addSpacer(10)
        self.layout.addItem(self.clearButton, 10)
        self.layout.addSpacer(30)
        self.layout.addItem(self.sourceGroup, 0)
        # self.layout.addItem(self.zoomPanGroup, 0)
        self.layout.addItem(self.compositeGroup, 0)
        self.layout.addItem(self.hudGroup, 0)
        

        self.onParentSizeChangedItem(viewer.width(), viewer.height())


    def onToolToggleChanged(self, val):
        print(val)
        self.setHeight(val['height'])

        currFrame = revlens.CNTRL.currentFrameNum()
        node = revlens.CNTRL.session().getNodeByFrame(currFrame)
        if not node:
            return
        
        props = node.getProperties()
        compositeNode = props.get('graph.node.composite')
        compositeInputInfo = compositeNode.getInputInfo()
        print(compositeInputInfo)


    def onSnapshotRequested(self, md):
        revlens.CNTRL.emitNoticeEvent('view.request_snapshot', {'display_idx': self._display.idx()})


    def onClearRequested(self, md):
        revlens_prod.cmds.clearAnnotationsOnCurrentFrame()


    def onNoticeEvent(self, evtName, evtInfo):

        if evtName in ['frame_changed', 'clip_changed']:
            self.compositeGroup.handleNoticeEvent(evtName, evtInfo)


        evtNameParts = evtName.split('.')
        # if len(evtNameParts) != 3:
        #     return

        if len(evtNameParts) < 2:
            return

        evtMainTool = evtNameParts[0]
        evtSubTool = evtNameParts[1]

        if evtMainTool not in ['view', 'video']:
            return

        if evtMainTool == 'view':
            if evtSubTool == 'request_composite':
                self.compositeGroup.handleNoticeEvent(evtSubTool, evtInfo)

            elif evtSubTool == 'request_hud':
                self.hudGroup.handleNoticeEvent(evtSubTool, evtInfo)

        elif evtMainTool == 'video':
            self.sourceGroup.handleNoticeEvent(evtSubTool, evtInfo)


    def paintItem(self, painter):

        b = QtGui.QBrush(QtGui.QColor(60, 60, 60))

        painter.setBrush(b)
        painter.drawRect(self.boundingRect())


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(40) # height)




class ViewerControlsToolbarHandler(RlpGui.GUI_ItemBase):

    def __init__(self, viewer):
        viewerToolbar = viewer.toolbar()

        RlpGui.GUI_ItemBase.__init__(self, viewerToolbar)

        self.controlsButton = RlpGui.GUI_SvgButton(':feather/image.svg', viewerToolbar, 20)
        self.controlsButton.setToolTipText('Viewer Controls')
        # self.controlsButton.icon().setBgColour(QtGui.QColor(120, 120, 120))
        self.controlsButton.buttonPressed.connect(self.toggleTools)

        viewerToolbar.addItem(self.controlsButton, 4)

        self.controls = ViewerControlsPanel(viewer)
        self.controls.setVisible(False)

    def setIconToggled(self, val):

        self.controlsButton.setToggled(val)
        self.controlsButton.icon().setOutlined(val)
        self.controlsButton.icon().update()


    def toggleTools(self, md):
        print(md)

        newVal = not md['toggled']
        self.setIconToggled(newVal)
        self.controls.setVisible(newVal)


class ViewerControlsToolbar(RlpGui.GUI_ItemBase):

    def __init__(self, viewer):
        RlpGui.GUI_ItemBase.__init__(self, viewer)

        self.setHeight(30)
        self.setWidth(viewer.width())


def createToolbar(viewer):
    return ViewerControlsPanel(viewer)


def initToolbar(toolInfo):
    return

    if toolInfo['name'] != 'Viewer':
            return

    viewer = toolInfo['tool']

    cbh = ViewerControlsToolbarHandler(viewer)

    # TODO FIXME: store in viewer instead?
    global VIEWER_CONTROLS
    VIEWER_CONTROLS.append(cbh)


def create(parent):
    return ViewerControlsPanel(parent)