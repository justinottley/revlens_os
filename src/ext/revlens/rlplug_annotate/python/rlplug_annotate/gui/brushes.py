
import functools

import rlp.gui as RlpGui

import rlplug_annotate

_PANELS = []


class BrushGroupPane(RlpGui.GUI_ItemBase):

    def __init__(self, parent, name):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.mp_handler = parent.mp_handler
        self.name = name

        self.layout = RlpGui.GUI_FlowLayout(self)
        self.layout.setOutlined(False)
        self.layout.setSpacing(10)

        self._thumbs = []
        self._init()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def _init(self):

        brushes = self.mp_handler.getBrushes(self.name)
        for brush_name in brushes:
            brush_path = ':brushes/{}/{}_prev.png'.format(self.name, brush_name)

            ic = RlpGui.GUI_IconButton(
                brush_path, self, 64, 4)

            ic.setMetadata('brush_name', brush_name)
            ic.buttonPressed.connect(self.on_brush_selected)

            self.layout.addItem(ic)
            self._thumbs.append(ic)

        self.layout.updateItems()



    def onParentSizeChangedItem(self, width, height):

        if not self.isItemVisible():
            return

        self.setItemWidth(width - 20)
        self.setItemHeight(height - 50)


    def on_brush_selected(self, md):
        self.mp_handler.requestBrush(
            self.name, md['brush_name']
        )


class BrushPaneArea(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.mp_handler = parent.mp_handler
        

        self.panes = {}

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onParentSizeChangedItem(self, width, height):
        self.setItemWidth(width)
        self.setItemHeight(height)


    def updateBrushes(self, bcat):

        bcat = bcat.lower()

        if bcat not in self.panes:
            self.panes[bcat] = BrushGroupPane(self, bcat)

        for pane in self.panes.values():
            pane.hideItem()

        self.panes[bcat].showItem()




class BrushPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.brush_categories = self.mp_handler.getBrushCategories()

        self.cat_widgets = {}
        self.toolbar_layout = RlpGui.GUI_HLayout(self)

        self.pane_area = BrushPaneArea(self)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.toolbar_layout, 0)
        self.layout.addSpacer(10)
        self.layout.addItem(self.pane_area, 10)

        for bcat in self.brush_categories:

            if bcat == 'favorites':
                continue

            button = RlpGui.GUI_IconButton(
                '',
                self,
                20,
                4
            )

            button.setMetadata('category', bcat)

            button.setText(bcat.capitalize())
            button.setOutlined(True)
            button.buttonPressed.connect(self.on_category_requested)

            self.cat_widgets[bcat] = {
                'button': button
            }

            self.toolbar_layout.addItem(button, 15)

        self.onParentSizeChangedItem(parent.width(), parent.height())
    
    @property
    def draw_controller(self):
        return rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ


    @property
    def mp_handler(self):
        return self.draw_controller.mypaintHandler()

    def onParentSizeChangedItem(self, width, height):
        self.setItemWidth(width)
        self.setItemHeight(height)


    def on_category_requested(self, md):
        self.pane_area.updateBrushes(md['category'])


def create(parent):
    brush_panel = BrushPanel(parent)
    global _PANELS
    _PANELS.append(brush_panel)

    return brush_panel

