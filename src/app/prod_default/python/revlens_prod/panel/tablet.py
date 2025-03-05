
from rlp import QtGui
import rlp.gui as RlpGui
from rlp.gui.buttongroup import GUI_ButtonGroup

import revlens.gui as RevlensGui

from rlplug_sync.panel.rooms import RoomsPanel


_PANELS = []


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
        viewer_toolbar.addDivider(20, 30)
        viewer_toolbar.addItem(self.toggleTimelineButton, 4)

        import rlplug_annotate.gui
        rlplug_annotate.gui.init_toolbar({'tool': self.viewer})

        self.timeline = RevlensGui.GUI_TL2_View(self)
        self.timeline.hideItem()

        self.toolbar.addButton(
            'Viewer',
            imagePath=':feather/lightgrey/image.svg',
            iconSize=self.ICON_SIZE,
            view=self.viewer
        )

        self.toolbar.addButton(
            'Files',
            imagePath=':feather/lightgrey/folder.svg',
            iconSize=self.ICON_SIZE,
            view=RevlensGui.GUI_FileBrowser(self)
        )

        self.toolbar.addButton(
            'Rooms',
            imagePath=':feather/lightgrey/home.svg',
            iconSize=self.ICON_SIZE,
            view=RoomsPanel(self)
        )

        try:
            from rlplug_annotate.gui.brushes import BrushPanel

            self.toolbar.addButton(
                'Brushes',
                imagePath=':feather/lightgrey/pen-tool.svg',
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
    global _PANELS
    panel = TabletPanel(parent)
    _PANELS.append(panel)

    return panel

