

import rlp.gui as RlpGui

_PANELS = []


class AnnotationPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.brushEngineLabel = RlpGui.GUI_Label(self, 'Brush Engine:')
        self.brushEngineList = RlpGui.GUI_IconButton('', self, 20, 4)
        self.brushEngineList.setOutlined(True)
        self.brushEngineList.setSvgIconPath(':feather/lightgrey/chevron-down.svg', 20, True)
        si = self.brushEngineList.svgIcon()
        si.setPos(si.x(), 4)
        
        blMenu = self.brushEngineList.menu()
        blMenu.setExclusiveSelect(True)
        blMenu.addItem('Basic', {}, True)
        blMenu.addItem('MyPaint', {}, True)
        blMenu.menuItemSelected.connect(self.onBrushEngineChanged)

        self.brushEngineList.setText('Basic')

        self.beLayout = RlpGui.GUI_HLayout(self)
        self.beLayout.addItem(self.brushEngineLabel, 0)
        self.beLayout.addItem(self.brushEngineList, 0)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.beLayout, 0)

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onBrushEngineChanged(self, md):
        print(md)

    def onParentSizeChangedItem(self, width, height):
        self.setItemWidth(width)
        self.setItemHeight(height)



def create(parent):
    ann_panel = AnnotationPanel(parent)
    global _PANELS
    _PANELS.append(ann_panel)

    return ann_panel