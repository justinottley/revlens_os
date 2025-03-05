


from rlp.Qt import QStr, QVMap, QV
import rlp.gui as RlpGui
import rlp.core as RlpCore

from revlens.keyboard import KeyBinding

_PANELS = []

class KeybindingPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        

        self.key_table = RlpGui.GUI_GRID_View(self)

        colmodel = self.key_table.colModel()
        colmodel.clear()
        colmodel.addCol(
            {
                'col_name': 'name',
                'display_name': 'Keybinding',
                'col_type': 'str',
                'width': 140
            },
            -1
        )

        colmodel.addCol(
            {
                'col_name': 'description',
                'display_name': 'Description',
                'col_type': 'str',
                'width': 300
            },
            -1
        )

        colmodel.addCol(
            {
                'col_name': 'callback',
                'display_name': 'Callback',
                'col_type': 'str',
                'width': 300
            },
            -1
        )


        colmodel.updateCols()

        coltype = colmodel.colType('callback')
        coltype.setVisible(False)
        colmodel.updateCols()

        colmenu_item = self.key_table.toolbar().colSelector().menu().item('Callback')
        colmenu_item.setChecked(False)

        print(coltype)

        self.key_table.updateHeader()

        self.init()

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.key_table, 10)

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def init(self):

        result = RlpCore.CoreDs_ModelDataItem.makeItem()

        for binding in KeyBinding.KEY_BINDINGS:

            item = RlpCore.CoreDs_ModelDataItem.makeItem()
            item.insert('name', binding.key_entry_name)
            item.insert('description', binding.description)
            item.insert('callback', binding.func_name)

            result.append(item)


        self.key_table.setModelData(result)

        



    def onParentSizeChangedItem(self, width, height):

        self.setItemWidth(width)
        self.setItemHeight(height)

        self.key_table.setItemWidth(width)
        self.key_table.setItemHeight(height - self.key_table.y())



def create(parent):

    keybinding_panel = KeybindingPanel(parent)
    _PANELS.append(keybinding_panel)
    return keybinding_panel