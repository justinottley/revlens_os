
import logging

import revlens

from rlp.Qt import QStr, QV
from rlp import QtCore, QtGui
import rlp.gui as RlpGui
import rlp.util as RlpUtil

import rlplug_qtbuiltin.cmds

import rlplug_annotate

ag = RlpUtil.APPGLOBALS.globals()

ICON_SIZE = 20
if ag['app.platform'] == 'ios':
    ICON_SIZE = 25

rlogger = logging.getLogger('rlp.rlplug_annotate.gui')

G_ANN_TOOLBAR_HANDLERS = []


class PaintBrushButton(RlpGui.GUI_IconButton):
    
    def __init__(self, parent):
        RlpGui.GUI_IconButton.__init__(self, ':/Freepik/White/pencil.png', parent, ICON_SIZE - 5, 4)

        self.setToolTipText('Paint Brush')

        self.category = 'ramon'
        self.name = 'Thin_Pen'

class EraserBrushButton(RlpGui.GUI_IconButton):
    def __init__(self, parent):
        RlpGui.GUI_IconButton.__init__(self, ':/Freepik/White/rubber.png', parent, ICON_SIZE - 5, 4)
        self.setToolTipText('Eraser')

        self.category = 'classic'
        self.name = 'ink_eraser'


class AnnotationToolbarHandler(RlpGui.GUI_ItemBase):

    def __init__(self, viewer_toolbar):
        RlpGui.GUI_ItemBase.__init__(self, viewer_toolbar)

        self._destructed = False # object lifecycle issues if layout is changed..

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.brush_paint_button = PaintBrushButton(self)
        self.brush_paint_button.setToggled(True)
        
        self.brush_eraser_button = EraserBrushButton(self)
        self.brush_eraser_button.setToggled(False)

        self.text_button = RlpGui.GUI_SvgButton(':text.svg', self, ICON_SIZE)
        self.text_button.setToolTipText('Text Tool')
        self.text_button.icon().setBgColour(QtGui.QColor(180, 180, 180))
        # self.text_button.icon().setPos(0, 3)
        # self.new_drawing_button = RlpGui.GUI_SvgButton(':create.svg', viewer_toolbar, 20)

        self.undo_button = RlpGui.GUI_SvgButton(':left-up.svg', self, ICON_SIZE)
        self.undo_button.setToolTipText('Undo Stroke')

        self.redo_button = RlpGui.GUI_SvgButton(':right-up.svg', self, ICON_SIZE)
        self.redo_button.setToolTipText('Redo Stroke')

        self.brush_sizecol_button = rlplug_annotate.RLANN_GUI_BrushSizeButton(self)
        self.brush_sizecol_button.setValue(10)

        self.colour_picker_button = RlpGui.GUI_SvgButton(':misc/color-picker.svg', self, ICON_SIZE)
        self.colour_picker_button.setToolTipText('Colour Picker')
        self.colour_picker_button.icon().setBgColour(QtGui.QColor(140, 140, 140))

        self.paper_trace_button = RlpGui.GUI_SvgButton(':misc/tracing-paper.svg', self, ICON_SIZE)
        self.paper_trace_button.setToolTipText('Toggle Paper Trace')
        self.paper_trace_button.setToggled(False)
        self.paper_trace_button.icon().setBgColour(QtGui.QColor(140, 140, 140))


        # self.brush_mypaint_button = MyPaintBrushButton(viewer_toolbar)

        viewer_toolbar.addDivider(20, ICON_SIZE + 5)
        viewer_toolbar.addItem(self.brush_paint_button, 2)
        viewer_toolbar.addItem(self.brush_eraser_button, 2)
        viewer_toolbar.addSpacer(5)
        viewer_toolbar.addItem(self.text_button, 4)
        viewer_toolbar.addSpacer(5)
        viewer_toolbar.addItem(self.colour_picker_button, 4)
        viewer_toolbar.addSpacer(5)
        viewer_toolbar.addItem(self.brush_sizecol_button, 3)
        viewer_toolbar.addSpacer(5)
        viewer_toolbar.addItem(self.paper_trace_button, 4)
        viewer_toolbar.addSpacer(5)
        viewer_toolbar.addItem(self.undo_button, 4)
        viewer_toolbar.addSpacer(5)
        viewer_toolbar.addItem(self.redo_button, 4)
        viewer_toolbar.addSpacer(5)


        self.brush_paint_button.buttonPressed.connect(self.on_brush_paint_selected)
        self.brush_eraser_button.buttonPressed.connect(self.on_brush_eraser_selected)
        self.text_button.buttonPressed.connect(self.on_text_selected)

        self.brush_sizecol_button.sizeItem().valueChanged.connect(self.on_brush_size_changed)
        self.brush_sizecol_button.sizeItem().colourSelected.connect(self.on_brush_colour_changed)

        self.colour_picker_button.buttonPressed.connect(self.on_pick_colour_requested)

        self.undo_button.buttonPressed.connect(self.on_undo_requested)
        self.redo_button.buttonPressed.connect(self.on_redo_requested)

        self.draw_controller.brushInfo.connect(self.on_brush_info_changed)
        self.draw_controller.setBrush('MyPaint')

        self.mp_handler.brushRequested.connect(self.on_brush_changed)
        self.set_colour({'r': 255, 'g': 0, 'b': 0})

        self.paper_trace_button.buttonPressed.connect(self.on_paper_trace_toggled)


    @property
    def draw_controller(self):
        return rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ

    @property
    def mp_handler(self):
        return self.draw_controller.mypaintHandler()


    def destruct(self):
        self._destructed = True


    def on_brush_changed(self, brush_info):
        if self._destructed:
            return

        brush_cat = brush_info['category']
        brush_name = brush_info['name']

        btn = self.brush_paint_button

        if 'text' in brush_name.lower():
            btn = self.text_button

            self.text_button.icon().setOutlined(True)

            self.brush_eraser_button.setToggled(True)
            self.brush_paint_button.setToggled(False)

            rlplug_qtbuiltin.cmds.display_message('Text')

        else:

            self.text_button.icon().setOutlined(False)

            if 'eraser' in brush_name.lower():
                btn = self.brush_eraser_button

                self.brush_eraser_button.setToggled(True)
                self.brush_paint_button.setToggled(False)

            elif 'text' in brush_name.lower():

                self.brush_eraser_button.setToggled(False)
                self.brush_paint_button.setToggled(True)


            btn.category = brush_cat
            btn.name = brush_name

            rlplug_qtbuiltin.cmds.display_message(
                'Brush: {}'.format(btn.name))



    def on_brush_paint_selected(self, md):
        if self._destructed:
            return

        self.LOG.info('paint brush')
        self.brush_eraser_button.setToggled(False)
        self.brush_paint_button.setToggled(True)

        self.text_button.icon().setOutlined(False)
        self.text_button.icon().update()

        self.colour_picker_button.setToggled(False)
        self.colour_picker_button.icon().setOutlined(False)
        self.colour_picker_button.icon().update()

        be = self.draw_controller.brushEngine()
        if be == 'Basic':
            self.draw_controller.setBrush('BasicPaint')

        else:
            self.draw_controller.setBrush('MyPaint')

            self.mp_handler.requestBrush(
                self.brush_paint_button.category,
                self.brush_paint_button.name
            )


    def on_brush_eraser_selected(self, md):
        if self._destructed:
            return

        self.LOG.info('eraser brush')

        self.brush_paint_button.setToggled(False)
        self.brush_eraser_button.setToggled(True)

        self.text_button.icon().setOutlined(False)
        self.text_button.icon().update()

        self.colour_picker_button.setToggled(False)
        self.colour_picker_button.icon().setOutlined(False)
        self.colour_picker_button.icon().update()

        be = self.draw_controller.brushEngine()
        if be == 'Basic':
            self.draw_controller.setBrush('BasicEraser')

        else:
            self.draw_controller.setBrush('MyPaint')

            self.mp_handler.requestBrush(
                self.brush_eraser_button.category,
                self.brush_eraser_button.name
            )

    def on_text_selected(self, md):
        if self._destructed:
            return

        print(md)
        self.brush_eraser_button.setToggled(False)
        self.brush_paint_button.setToggled(False)
        self.colour_picker_button.setToggled(False)
        self.colour_picker_button.icon().setOutlined(False)
        self.colour_picker_button.icon().update()


        self.text_button.icon().setOutlined(True)
        self.text_button.icon().update()

        self.draw_controller.setBrush('Text')

        rlplug_qtbuiltin.cmds.display_message('Text')


    def on_brush_size_changed(self, valInfo):
        if self._destructed:
            return

        self.LOG.info('brush_size_changed: {}'.format(valInfo))

        value = valInfo['val']

        self.draw_controller.setBrushSettings({'width': value})


    def on_brush_colour_changed(self, colInfo):
        if self._destructed:
            return

        # print('brush_colour_changed: {}'.format(colInfo))

        self.draw_controller.setBrushSettings({'col': colInfo})


    def on_undo_requested(self, md):
        cf = self.draw_controller.currFrame()
        atrack = self.draw_controller.activeTrack()
        if not atrack:
            return

        ann = atrack.getAnnotationPtr(cf)
        ann.undoStroke()


    def on_redo_requested(self, md):
        cf = self.draw_controller.currFrame()
        atrack = self.draw_controller.activeTrack()
        if not atrack:
            return

        ann = atrack.getAnnotationPtr(cf)
        ann.redoStroke()


    def set_colour(self, colour):

        # print('set colour: {}'.format(colour))

        self.brush_sizecol_button.onColourSelected(colour)
        self.on_brush_colour_changed(colour)


    def on_pick_colour_requested(self, md=None):

        if self._destructed:
            return

        self.brush_eraser_button.setToggled(False)
        self.brush_paint_button.setToggled(False)
        
        self.text_button.icon().setOutlined(False)
        self.text_button.icon().update()

        self.colour_picker_button.setToggled(True)
        self.colour_picker_button.icon().setOutlined(True)
        self.colour_picker_button.icon().update()

        self.draw_controller.setBrush('Picker')

        rlplug_qtbuiltin.cmds.display_message('Colour Picker')

    def on_brush_info_changed(self, info):
        print(info)
        if (info.get('brush.name') == 'Picker'):
            self.set_colour({
                'r': info['r'],
                'g': info['g'],
                'b': info['b']
            })  


    def on_paper_trace_toggled(self, md):
        # print('paper trace toggle')

        ptIsActive = not self.draw_controller.isPaperTraceActive()
        self.draw_controller.setPaperTraceActive(ptIsActive)
        self.paper_trace_button.setToggled(ptIsActive)
        self.paper_trace_button.icon().setOutlined(ptIsActive)
        self.paper_trace_button.icon().update()

        msg = 'Paper Trace: '
        if ptIsActive:
            msg += 'ON'

        else:
            msg += 'OFF'

        rlplug_qtbuiltin.cmds.display_message(msg)


def init_toolbar(tool_info):

    tool = tool_info['tool']
    viewer_toolbar = tool.toolbar()
    rlogger.info('initializing viewer toolbar: {}'.format(viewer_toolbar))

    global G_ANN_TOOLBAR_HANDLERS
    t_ann_handler = AnnotationToolbarHandler(viewer_toolbar)
    # G_ANN_TOOLBAR_HANDLERS.append(t_ann_handler) 
