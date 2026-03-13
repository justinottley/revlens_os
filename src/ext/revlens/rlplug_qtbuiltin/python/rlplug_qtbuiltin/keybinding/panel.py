


from rlp.Qt import QStr, QVMap, QV
import rlp.gui as RlpGui
import rlp.core as RlpCore

from revlens.keyboard import KeyBinding


class KeybindingPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.setWidth(parent.width())
        self.setHeight(parent.height())

        self.key_table = RlpGui.GUI_GRID_View(self, None)

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


        

        # coltype = colmodel.colType('callback')
        # coltype.setVisible(False)
        # colmodel.updateCols()

        # colmenu_item = self.key_table.toolbar().colSelector().menu().item('Callback')
        # colmenu_item.setChecked(False)

        # print(coltype)

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


        self.key_table.setModelData(result, '', True, True)


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)

        self.key_table.setWidth(width - 30)
        self.key_table.setHeight(height - self.key_table.y() - 40)



def create(parent):
    return KeybindingPanel(parent)