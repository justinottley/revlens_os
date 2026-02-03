

from rlp import QtGui
from rlp import QtCore
from rlp.Qt import QStr

import rlp.gui as RlpGui
import rlp.util as RlpUtil

from rlp.gui.expandable import GUI_ExpandableWidget

import revlens

from . import brushes as ann_brushes
from rlplug_annotate import RLANN_GUI_BrushSizeMenuItem

import rlplug_annotate
import rlplug_annotate.cmds
import rlplug_qtbuiltin.cmds

ag = RlpUtil.APPGLOBALS.globals()

ICON_SIZE = 20
ICON_OUTLINED = False
ANN_ICON_SIZE = 35
GROUP_BASE_HEIGHT = 50
GROUP_LABEL_YPOS = 17

if ag['app.platform'] == 'ios':
    ICON_SIZE = 25
    ICON_OUTLINED = True
    ANN_ICON_SIZE = 45
    GROUP_BASE_HEIGHT = 60
    GROUP_LABEL_YPOS = 20


class AnnButton(RlpGui.GUI_ItemBase):

    def __init__(self, parent, path, text, category=None, name=None):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.buttonPressed = QtCore.PY_Signal(self)

        self.button = RlpGui.GUI_SvgButton(path, self, ANN_ICON_SIZE)
        self.button.setOutlined(True)
        self.button.setPos(2, 2)
        self.button.setZValue(-1)

        self.selected = False

        self.category = category
        self.name = name

        self.text = text
        self.setToolTipText(text)

        self.setWidth(ANN_ICON_SIZE + 5)
        self.setHeight(ANN_ICON_SIZE + 5)


    def setSelected(self, isSelected):
        self.selected = isSelected
        if isSelected:
            self.button.setOutlinedBgColour(QtGui.QColor(80, 80, 120))

        else:
            self.button.setOutlinedBgColour(QtGui.QColor(80, 80, 80))

        self.button.icon().update()
        self.update()


    def mousePressEventItem(self, event):
        self.buttonPressed.emit(self.button.metadata())


    def paintItem(self, painter):

        penWidth = 2
        p = QtGui.QPen(QtGui.QColor(20, 20, 20))
        p.setWidth(penWidth)
        
        painter.setPen(p)
        painter.setBrush(QtGui.QBrush(QtGui.QColor.transparent()))

        painter.drawRoundedRect(
            QtCore.QRectF(penWidth, penWidth, self.width() - (penWidth * 2), self.height() - (penWidth * 2)),
            penWidth, penWidth
        )

        # f = painter.font()
        # f.setPixelSize(12)
        # f.setBold(True)
        # painter.setFont(f)

        # painter.setPen(QtGui.QColor(10, 10, 10))
        # painter.drawText(9, int(self.height() - 8), QStr(self.text))

        # painter.setPen(QtGui.QColor(100, 100, 140))
        # painter.drawText(7, int(self.height() - 10), QStr(self.text))



class GroupWidget(GUI_ExpandableWidget):

    def __init__(self, parent, label):
        kwargs = {
            'baseHeight': GROUP_BASE_HEIGHT,
            'labelYPos': GROUP_LABEL_YPOS,
            'iconSize': 25
        }
        GUI_ExpandableWidget.__init__(self, parent, label, **kwargs)

    @property
    def drawController(self):
        return rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ


class HistoryGroupWidget(GroupWidget):

    def __init__(self, parent):
        
        self.clear_button = None
        self.undo_button = None
        self.redo_button = None

        GroupWidget.__init__(self, parent, 'Undo:')

        iconSize = 25
        if ag['app.platform'] == 'ios':
            iconSize = 30


        self.clear_button = RlpGui.GUI_SvgButton(':feather/trash-2.svg', self, iconSize)
        self.clear_button.setOutlined(True)
        self.clear_button.buttonPressed.connect(self.onClearRequested)

        self.undo_button = RlpGui.GUI_SvgButton(':left-up.svg', self, iconSize)
        self.undo_button.setOutlined(True)
        self.undo_button.setToolTipText('Undo Stroke')
        self.undo_button.buttonPressed.connect(self.onUndoRequested)

        self.redo_button = RlpGui.GUI_SvgButton(':right-up.svg', self, iconSize)
        self.redo_button.setOutlined(True)
        self.redo_button.setToolTipText('Redo Stroke')
        self.redo_button.buttonPressed.connect(self.onRedoRequested)

        #self.toggle_expand_button.setVisible(False)
        #self.toggle_collapse_button.setVisible(False)

    def onClearRequested(self, md):
        rlplug_annotate.cmds.clearAnnotationOnCurrentFrame()


    def onUndoRequested(self, md):
        cf = self.drawController.currFrame()
        atrack = self.drawController.activeTrack()
        if not atrack:
            return

        ann = atrack.getAnnotationPtr(cf)
        ann.undoStroke()


    def onRedoRequested(self, md):
        cf = self.drawController.currFrame()
        atrack = self.drawController.activeTrack()
        if not atrack:
            return

        ann = atrack.getAnnotationPtr(cf)
        ann.redoStroke()


    def onParentSizeChangedItem(self, width, height):
        super().onParentSizeChangedItem(width, height)

        if self.undo_button and self.redo_button:
            self.clear_button.setPos(width - 187, 15)
            self.undo_button.setPos(width - 137, 15)
            self.redo_button.setPos(width - 97, 15)


class PaperTraceGroupWidget(GroupWidget):

    def __init__(self, parent):

        self.toggle = None
        self.stateLabel = None
        self.opacityLabel = None

        GroupWidget.__init__(self, parent, 'Trace Paper:')

        self.stateLabel = RlpGui.GUI_Label(self, 'OFF')
        self.stateLabel.setPos(140, GROUP_LABEL_YPOS + 1)
        f = self.stateLabel.font()
        f.setPixelSize(16)
        f.setBold(True)
        self.stateLabel.setFont(f)

        self.opacityLabel = RlpGui.GUI_Label(self, '')

        self.toggle = RlpGui.GUI_RadioButton(self, '', 25, 0)
        self.toggle.buttonPressed.connect(self.onPaperTraceToggled)

        # self.toggle_expand_button.setVisible(False)
        # self.toggle_collapse_button.setVisible(False)

        self.icon = RlpGui.GUI_SvgButton(':misc/tracing-paper.svg', self, 30)
        self.icon.setPos(10, 15)
        self.slider = RlpGui.GUI_Slider(self, RlpGui.GUI_Slider.O_SL_HORIZONTAL, 300)
        self.slider.setPos(100, 0)
        self.slider.setSliderValue(0.7)
        self.slider.moving.connect(self.onValueChanged)

        self.layout = RlpGui.GUI_HLayout(self)
        # self.slayout.setPos(50, 0)
        self.layout.addSpacer(3)
        self.layout.addItem(self.icon, 0)
        self.layout.addSpacer(5)
        self.layout.addItem(self.slider, -5)
        # self.layout.setVisible(False)

        self.slayout.addItem(self.layout, 0)


    def onValueChanged(self, val):
        self.drawController.setPaperTraceOpacity(val)

        if self.toggle.isToggled():
            self.opacityLabel.setText(str(round(val, 2)))
            self.opacityLabel.update()


    def onPaperTraceToggled(self, md):

        ptIsActive = not self.drawController.isPaperTraceActive()
        self.drawController.setPaperTraceActive(ptIsActive)
        self.toggle.setToggled(ptIsActive)

        stateStr = ''
        if ptIsActive:
            stateStr += 'ON'
            self.opacityLabel.setText(str(round(self.drawController.paperTraceOpacity(), 2)))

        else:
            stateStr += 'OFF'
            self.opacityLabel.setText('')

        self.stateLabel.setText(stateStr)
        self.stateLabel.update()

        
        self.opacityLabel.update()

        rlplug_qtbuiltin.cmds.display_message('Paper Trace: {}'.format(stateStr))


    def onParentSizeChangedItem(self, width, height):
        super().onParentSizeChangedItem(width, height)

        if self.toggle and self.opacityLabel:
            self.toggle.setPos(width - 95, GROUP_LABEL_YPOS - 5)
            self.opacityLabel.setPos(width - 150, 20)


class GhostingGroupWidget(GroupWidget):

    def __init__(self, parent):

        self.toggle = None
        self.stateLabel = None

        GroupWidget.__init__(self, parent, 'Ghosting:')

        self.stateLabel = RlpGui.GUI_Label(self, 'OFF')
        self.stateLabel.setPos(140, GROUP_LABEL_YPOS + 1)
        f = self.stateLabel.font()
        f.setPixelSize(16)
        f.setBold(True)
        self.stateLabel.setFont(f)

        self.toggle = RlpGui.GUI_RadioButton(self, '', 25, 0)
        self.toggle.buttonPressed.connect(self.onGhostingToggled)


    def onGhostingToggled(self, md):
        print(md)

        toggled = not self.toggle.isToggled()
        print(toggled)

        stateStr = ''
        if toggled:
            stateStr += 'ON'

        else:
            stateStr += 'OFF'

        self.stateLabel.setText(stateStr)
        self.toggle.setToggled(toggled)

        atrack = self.drawController.activeTrack()
        if atrack:
            if toggled:
                atrack.setGhostFrameCount(1)

            else:
                atrack.setGhostFrameCount(0)



    def onParentSizeChangedItem(self, width, height):
        super().onParentSizeChangedItem(width, height)

        if self.toggle and self.stateLabel:
            self.toggle.setPos(width - 95, GROUP_LABEL_YPOS - 5)
            # self.stateLabel.setPos(width - 150, 23)



class BrushSizeColGroupWidget(GroupWidget):

    def __init__(self, parent):

        self.currSizeLabel = None

        GroupWidget.__init__(self, parent, 'Size and Colour:')

        self.colInfo = {'r':255, 'g': 0, 'b': 0}

        self.currSizeLabel = RlpGui.GUI_Label(self, '-')
        self.currSizeLabel.setPos(200, GROUP_LABEL_YPOS)
        f = self.currSizeLabel.font()
        f.setPixelSize(16)
        f.setBold(True)
        self.currSizeLabel.setFont(f)

        self.sizeColButton = RLANN_GUI_BrushSizeMenuItem(self)
        self.sizeColButton.colourSelected.connect(self.onColourSelected)
        self.sizeColButton.valueChanged.connect(self.onSizeChanged)
        self.sizeColButton.setValue(1)

        self.slayout.addItem(self.sizeColButton, 0)
        self.slayout.setVisible(False)


    def onSizeChanged(self, md):
        self.currSizeLabel.setText(str(int(md['val'])))
        self.currSizeLabel.update()

        self.drawController.setBrushSettings({'width': md['val']}, "", True)


    def onColourSelected(self, md):
        self.colInfo = md
        self.update()

        self.drawController.setBrushSettings({'col': self.colInfo}, "", True)


    def onParentSizeChangedItem(self, width, height):
        super().onParentSizeChangedItem(width, height)

        if self.currSizeLabel:
            self.currSizeLabel.setPos(width - 130, GROUP_LABEL_YPOS)
    

    def paintItem(self, painter):
        GroupWidget.paintItem(self, painter)

        if self.colInfo:
            rect = QtCore.QRectF(self.width() - 71, GROUP_LABEL_YPOS - 5, 28, 28)
            painter.setPen(QtGui.QColor(self.colInfo['r'], self.colInfo['g'], self.colInfo['b']))
            painter.drawRect(rect)



class BrushTypeGroupWidget(GroupWidget):
    
    EXPAND_HEIGHT = 400

    ICON_POS = 7
    if ag['app.platform'] == 'ios':
        ICON_POS = 10

    def __init__(self, parent):

        self.currBrushImage = None

        GroupWidget.__init__(self, parent, 'Brush:')

        self.currBrushText = RlpGui.GUI_Label(self, '')
        f = self.currBrushText.font()
        f.setPixelSize(16)
        f.setBold(True)
        self.currBrushText.setFont(f)
        self.currBrushText.setPos(100, GROUP_LABEL_YPOS)

        
        self.currBrushImage = RlpGui.GUI_IconButton('', self, 30, 4)
        self.currBrushImage.setPos(100, self.ICON_POS)
        self.brushPanel = ann_brushes.BrushPanel(self)
        self.brushPanel.pane_area.brushSelected.connect(self.onBrushSelected)
        self.slayout.addItem(self.brushPanel, 0)

        self.onBrushSelected({'category': 'deevad', 'brush_name': 'pen-note'})


    def onBrushSelected(self, md):

        brushPath = ':brushes/{}/{}_prev.png'.format(md['category'], md['brush_name'])
        self.currBrushText.setText(md['brush_name'])
        self.currBrushImage.setIconPath(brushPath)
        self.currBrushImage.update()
        self.update()


    def onParentSizeChangedItem(self, width, height):
        GroupWidget.onParentSizeChangedItem(self, width, height)

        if self.currBrushImage:
            self.currBrushImage.setPos(width - 100, self.ICON_POS)


class TrackGroupWidget(GroupWidget):

    def __init__(self, parent):
        GroupWidget.__init__(self, parent, 'Track:')

        self.trackSelector = RlpGui.GUI_IconButton('', self, 20, 4)
        self.trackSelector.setPos(100, GROUP_LABEL_YPOS - 5)
        self.trackSelector.setOutlined(True)
        self.trackSelector.setMenuXOffset(40)
        menu = self.trackSelector.menu()

        trackCount = revlens.CNTRL.session().numTracks()
        for trackIdx in range(trackCount):
            strack = revlens.CNTRL.session().getTrackByIndex(trackIdx)
            if strack.trackType() == rlplug_annotate.TRACK_TYPE_ANNOTATE:
                menu.addItem(strack.name(), {}, False)
    
        atrack = self.drawController.activeTrack()
        if atrack:
            self.trackSelector.setText(atrack.name())


        self.newTrackButton = RlpGui.GUI_IconButton('', self, 25, 4)
        self.newTrackButton.setOutlined(True)
        self.newTrackButton.setText('New Track')
        self.newTrackButton.buttonPressed.connect(self.onNewTrackRequested)

        self.slayout.addItem(self.newTrackButton, 10)


    def onNewTrackRequested(self, md):
        revlens.CNTRL.session().addTrackByType('Annotation')


class AnnotationPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.drawController.brushInfoChanged.connect(self.onBrushInfoChanged)

        self.annToolLayout = RlpGui.GUI_HLayout(self)
        self.annToolLayout.setSpacing(10)

        self.cursorButton = AnnButton(self, ':misc/cursor.svg', 'Cursor')
        self.cursorButton.buttonPressed.connect(self.onCursorSelected)

        self.paintBrushButton = AnnButton(self, ':misc/brush-stroke.svg', ' Paint', 'deevad', 'pen-note')
        self.paintBrushButton.buttonPressed.connect(self.onPaintBrushSelected)

        self.eraserBrushButton = AnnButton(self, ':misc/eraser.svg', ' Erase', 'classic', 'ink_eraser')
        self.eraserBrushButton.buttonPressed.connect(self.onEraserBrushSelected)

        self.textButton = AnnButton(self, ':text.svg', 'Text')
        self.textButton.buttonPressed.connect(self.onTextSelected)

        self.pickerButton = AnnButton(self, ':misc/color-picker.svg', '  Pick')
        self.pickerButton.buttonPressed.connect(self.onPickerSelected)


        self.tools = {
            'cursor': self.cursorButton,
            'paint': self.paintBrushButton,
            'eraser': self.eraserBrushButton,
            'text': self.textButton,
            'picker': self.pickerButton
        }

        for toolName, toolButton in self.tools.items():
            toolButton.button.setMetadata('name', toolName)
            toolButton.buttonPressed.connect(self.onToolChanged)
            self.annToolLayout.addItem(toolButton, 5)


        self.brushTypeGroup = BrushTypeGroupWidget(self)
        self.brushSizeColGroup = BrushSizeColGroupWidget(self)
        self.undoRedoGroup = HistoryGroupWidget(self)
        self.paperTraceGroup = PaperTraceGroupWidget(self)
        self.ghostingGroup = GhostingGroupWidget(self)
        self.trackGroup = TrackGroupWidget(self)


        self.toolLayout = RlpGui.GUI_VLayout(self)
        # self.toolLayout.addItem(self.brushPanel, 10)

        self.sa = RlpGui.GUI_ScrollArea(self)
        self.sa.setPos(0, ANN_ICON_SIZE + 15)
        self.sa.setHeight(200)
        self.sa.setOutlined(False)

        self.layout = RlpGui.GUI_VLayout(self.sa.content())
        # self.layout.setVSpacing(0)
        # self.layout.addItem(self.annToolLayout, 0)
        self.layout.addItem(self.brushTypeGroup, 10)
        self.layout.addItem(self.brushSizeColGroup, 10)
        self.layout.addItem(self.undoRedoGroup, 10)
        self.layout.addItem(self.paperTraceGroup, 10)
        self.layout.addItem(self.ghostingGroup, 10)
        self.layout.addItem(self.trackGroup, 10)


        # self.layout.addItem(self.tracePaperWidget, 10)
        # self.layout.addSpacer(25)
        # self.layout.addItem(self.toolLayout, 0)

        self.onPaintBrushSelected({})
        self.onParentSizeChangedItem(parent.width(), parent.height())



    @property
    def drawController(self):
        return rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ
    

    def onToolChanged(self, md):
        toolName = md['name']
        for toolButton in self.tools.values():
            toolButton.setSelected(False)

        self.tools[toolName].setSelected(True)


    def onCursorSelected(self, md):
        self.brushTypeGroup.hide()
        self.brushSizeColGroup.hide()
        self.drawController.setBrush('NONE', '')


    def onPaintBrushSelected(self, md):
        self.brushTypeGroup.show()
        self.brushSizeColGroup.show()

        self.drawController.setBrush('MyPaint', '')
        self.drawController.mypaintHandler().requestBrush(
            self.paintBrushButton.category,
            self.paintBrushButton.name
        )
        rlplug_qtbuiltin.cmds.display_message('Draw')


    def onEraserBrushSelected(self, md):
        self.brushTypeGroup.show()
        self.brushSizeColGroup.show()

        self.drawController.setBrush('MyPaintEraser', '')
        rlplug_qtbuiltin.cmds.display_message('Erase')


    def onTextSelected(self, md):
        self.brushTypeGroup.hide()
        self.brushSizeColGroup.show()
        self.drawController.setBrush('Text', '')
        rlplug_qtbuiltin.cmds.display_message('Text')


    def onPickerSelected(self, md):
        self.brushTypeGroup.hide()
        self.drawController.setBrush('Picker', '')
        rlplug_qtbuiltin.cmds.display_message('Picker')


    def onBrushInfoChanged(self, info):
        print(info)
        if info.get('brush.name') == 'Picker':
            col = QtGui.QColor(info['r'], info['g'], info['b'])
            self.brushSizeColGroup.sizeColButton.onColourSelected(col)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)

        self.sa.onParentSizeChanged(width - 15, height)
        self.sa.setMaxChildHeight(self.layout.itemHeight() + 100)
        self.sa.setHeight(height)





def create(parent):
    return AnnotationPanel(parent)