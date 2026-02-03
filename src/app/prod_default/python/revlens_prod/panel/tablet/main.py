
from rlp import QtGui
import rlp.gui as RlpGui
from rlp.gui.buttongroup import GUI_ButtonGroup

import revlens
import revlens.gui as RevlensGui

import rlplug_qtbuiltin
from rlplug_sync.panel.rooms import RoomsPanel


class TabletToolbarPanel(RlpGui.GUI_ItemBase):

    ICON_SIZE = 40

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._currPanel = None

        self.toolbar = GUI_ButtonGroup(self, GUI_ButtonGroup.MODE_VERTICAL)
        self.toolbar.buttonPressed.connect(self.onButtonPressed)
        self.toolbar.setWidth(self.ICON_SIZE + 10)

        for toolInfo in [
            {'name': 'Display Controls', 'icon': ':misc/frame.svg', 'size': self.ICON_SIZE - 6, 'padding': 8},
            {'name': 'Annotate', 'icon': ':feather/pen-tool.svg'},
            {'name': 'Rooms', 'icon': ':feather/home.svg'},
            {'name': 'Playlist', 'icon': ':feather/list.svg'},
            {'name': 'Tablet Stream', 'icon': ':misc/stream.svg'},
            {'name': 'Timeline', 'icon': ':misc/video-editor.svg'}
        ]:

            iconSize = toolInfo.get('size', self.ICON_SIZE - 4)

            b = self.toolbar.addButton(
                toolInfo['name'],
                imagePath=toolInfo['icon'],
                iconSize=iconSize,
                iconHeight=iconSize,
                padding=toolInfo.get('padding', 0),
                iconWidthPadding=toolInfo.get('padding', 0),
                offset=5
            )
            b.setWidth(40)
            b.svgIcon().setXOffset(2)
            b.svgIcon().setYOffset(2)
            b.setOutlined(False)


        pane = revlens.GUI.mainView().rootPane().panePtr(1)
        pane.splitterPtr(0).setPosPercent(0.0, True)
        pane.panePtr(0).setVisible(False)
        
        self.onParentSizeChangedItem(parent.width(), parent.height())


    def mousePressEventItem(self, event):
        if self._currPanel:
            self.onButtonPressed({'title':self._currPanel})


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)


    def onButtonPressed(self, md):

        pane = revlens.GUI.mainView().rootPane().panePtr(1)

        toolTitle = md['title']

        posPercent = 0.4
        if toolTitle in ['Projects']:
            posPercent = 0.5

        elif toolTitle == 'Display Controls':
            posPercent = 0.25

        if toolTitle == 'Timeline':

            pos = 0.7
            vpane = pane.panePtr(1)
            tlPane = vpane.panePtr(1)
            cpos = vpane.splitterPtr(0).splitterPos()
            if cpos != 1.0:
                vpane.splitterPtr(0).setPosPercent(1.0, True)
                tlPane.setVisible(False)

            else:
                vpane.splitterPtr(0).setPosPercent(0.7, True)
                tlPane.setVisible(True)


        else:


            if toolTitle == self._currPanel and pane.panePtr(0).isVisible():
                pane.splitterPtr(0).setPosPercent(0.0, True)
                pane.panePtr(0).setVisible(False)

            else:
                pane.panePtr(0).setVisible(True)
                pane.panePtr(0).onTabActivateRequested(toolTitle)
                pane.splitterPtr(0).setPosPercent(posPercent, True)

            if toolTitle == 'Display Controls':

                disp = revlens.CNTRL.getVideoManager().getDisplayByIdx(0)
                cmenu = disp.contextMenu()
                dispControls = cmenu.item('Display Controls')

                visVal = pane.panePtr(0).isVisible()
                dispControls.selectItemWithValue(visVal)

            self._currPanel = toolTitle



class TabletPanel(RlpGui.GUI_ItemBase):

    ICON_SIZE = 50

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.toolbar = GUI_ButtonGroup(self, GUI_ButtonGroup.MODE_VERTICAL)
        self.toolbar.buttonPressed.connect(self.onButtonPressed)
        self.toolbar.setWidth(self.ICON_SIZE)

        self.toggleTimelineButton = RlpGui.GUI_SvgButton(
            ':misc/project-plan.svg',
            self,
            25
        )
        self.toggleTimelineButton.buttonPressed.connect(self.onToggleTimeline)

        self.viewer = RevlensGui.GUI_ViewerPane(self)
        viewer_toolbar = self.viewer.toolbar()
        viewer_toolbar.addDivider(20, 30, 0)
        viewer_toolbar.addItem(self.toggleTimelineButton, 4)

        import rlplug_annotate.gui
        rlplug_annotate.gui.init_toolbar({'tool': self.viewer, 'name': 'Viewer'})

        self.timeline = RevlensGui.GUI_TL2_View(self)
        self.timeline.hideItem()

        self.toolbar.addButton(
            'Viewer',
            imagePath=':feather/image.svg',
            iconSize=self.ICON_SIZE,
            view=self.viewer
        )

        self.toolbar.addButton(
            'Files',
            imagePath=':feather/folder.svg',
            iconSize=self.ICON_SIZE,
            view=RevlensGui.GUI_FileBrowser(self)
        )

        self.toolbar.addButton(
            'Rooms',
            imagePath=':feather/home.svg',
            iconSize=self.ICON_SIZE,
            view=RoomsPanel(self)
        )

        # doesn't work well on ios
        # self.toolbar.addButton(
        #     'Streams',
        #     imagePath=':feather/cast.svg',
        #     iconSize=self.ICON_SIZE,
        #     view=rlplug_qtbuiltin.GUI_StreamingPane(self)
        # )
        try:
            from rlplug_annotate.gui.brushes import BrushPanel

            self.toolbar.addButton(
                'Annotate',
                imagePath=':feather/pen-tool.svg',
                iconSize=self.ICON_SIZE,
                view=BrushPanel(self)
            )
        except:
            pass


        self.onParentSizeChangedItem(parent.width(), parent.height())
        self.toolbar.selectButton('Viewer')


    def onToggleTimeline(self, md):
        toggled = not md['toggled']
        self.toggleTimelineButton.setToggled(toggled)

        if toggled:
            self.timeline.setWidth(self.width() - self.ICON_SIZE)
            self.timeline.setHeight(200)
            self.timeline.setPos(self.ICON_SIZE, self.height() - 190)
            self.timeline.showItem()
            self.timeline.refresh()

            self.toolbar.currentView.onParentSizeChanged(
                self.width() - self.ICON_SIZE,
                self.height() - 190
            )
            self.toolbar.currentView.update()

        else:
            self.timeline.hideItem()
            self.toolbar.currentView.onParentSizeChanged(
                self.width() - self.ICON_SIZE,
                self.height()
            )
            self.toolbar.currentView.update()


    def onButtonPressed(self, md):
        if md['title'] == 'Viewer':
            self.viewer.setPaintBackground(False)

        else:
            self.setPaintBackground(True)

        self.update()


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)

        self.toolbar._w = width - self.ICON_SIZE
        self.toolbar._h = height

        self.timeline.setHeight(200)
        self.timeline.setWidth(width - self.ICON_SIZE)
        self.timeline.setPos(self.ICON_SIZE, height - 190)

        for viewObj in self.toolbar.views.values():
            viewObj.setWidth(width - self.ICON_SIZE)
            viewObj.setHeight(height)
            if viewObj.__class__.__name__ == 'GUI_ViewerPane':
                viewObj.setPaintBackground(False)
                viewObj.onParentSizeChanged(width - self.ICON_SIZE, height)

            else:
                viewObj.setPaintBg(True)
            viewObj.setPos(self.ICON_SIZE, 0)


    def paintItem(self, painter):

        painter.setPen(QtGui.QColor(40, 40, 40))
        painter.setBrush(QtGui.QBrush(QtGui.QColor(40, 40, 40)))
        painter.drawRect(0, 0, self.ICON_SIZE, int(self.height()))


def create(parent):
    return TabletPanel(parent)

