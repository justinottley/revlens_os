
import os

from rlp import QtGui
import rlp.gui as RlpGui

G_HANDLERS = []

RLOCIO_PLUGIN_OBJ = None

class OCIOToolbarHandler(RlpGui.GUI_ItemBase):

    def __init__(self, viewer):
        viewer_toolbar = viewer.toolbar()
        RlpGui.GUI_ItemBase.__init__(self, viewer_toolbar)

        self.ocio_button = RlpGui.GUI_SvgButton(':misc/opencolorio-icon-white.svg', self, 20)
        self.ocio_button.setToolTipText("Color Tools")
        self.ocio_button.icon().setBgColour(QtGui.QColor(120, 120, 120))
        self.ocio_button.buttonPressed.connect(self.toggle_tools)

        viewer_toolbar.addDivider(20, 25)
        viewer_toolbar.addItem(self.ocio_button, 4)

        self._view = viewer.view()
        self.na_label = RlpGui.GUI_Label(self._view, "OCIO Controls Not Available")
        self.na_label.setBgColor(QtGui.QColor(180, 40, 40))
        self.na_label.setPos(500, 10)
        self.na_label.hideItem()


    @property
    def paintEngine(self):
        return self._view.getCurrentPaintEngine()


    def _set_icon_toggled(self, val):
        self.ocio_button.setToggled(val)
        self.ocio_button.icon().setOutlined(val)
        self.ocio_button.icon().update()
        
    def toggle_tools(self, md=None):
        global RLOCIO_PLUGIN_OBJ

        if not self.paintEngine:
            return

        if 'OCIO' not in self.paintEngine.__class__.__name__:
            print('INVALID PAINT ENGINE: {}'.format(self.paintEngine))
            return

        import rlplug_qtbuiltin

        if not self.paintEngine.isEnabled():
            if not self.na_label.isItemVisible():
                self._set_icon_toggled(True)
                self.na_label.showItem()

            else:
                self._set_icon_toggled(False)
                self.na_label.hideItem()

            # return

        # self.na_label.hideItem()

        otools = RLOCIO_PLUGIN_OBJ.overlayTools()

        if otools.isItemVisible():
            self._set_icon_toggled(False)
            otools.hideItem()
            rlplug_qtbuiltin.RLQTDISP_FADEMESSAGEPLUGIN_OBJ.setYPos(20)


        else:
            self._set_icon_toggled(True)
            otools.showItem()
            rlplug_qtbuiltin.RLQTDISP_FADEMESSAGEPLUGIN_OBJ.setYPos(50)



def init_toolbar(tool_info):

    viewer = tool_info['tool']
    ocio_handler = OCIOToolbarHandler(viewer)

    global G_HANDLERS
    G_HANDLERS.append(ocio_handler)
