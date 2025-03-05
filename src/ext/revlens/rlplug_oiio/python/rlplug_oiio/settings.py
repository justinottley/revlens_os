
import subprocess

from rlp.Qt import QStr, QVMap, QV
import rlp.core as RlpCore
import rlp.gui as RlpGui


import revlens


_PANELS = []

class OIIOSettingsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent.body())

        self.version_label = RlpGui.GUI_Label(self, 'Version: 2.5.5.0')
        self.formats_table = RlpGui.GUI_GRID_View(self)
        colmodel = self.formats_table.colModel()
        colmodel.clear()
        colmodel.addCol(
            {
                'col_name': 'text',
                'display_name': 'Format Info',
                'col_type': 'str',
                'width': 350
            },
            -1
        )

        self.formats_table.updateHeader()

        self.init()

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(40)

        self.layout.addItem(self.version_label, 0)
        self.layout.addSpacer(10)
        self.info_layout = RlpGui.GUI_HLayout(self)
        self.info_layout.addItem(self.formats_table, 0)

        self.layout.addItem(self.info_layout, 20)

        parent.setItemHeight(400)
        parent.setItemWidth(500)
        cbtn = parent.closeButton()
        cbtn.setPos(parent.width() - cbtn.width() - 10, 6)


        self.setItemWidth(parent.width())
        self.setItemHeight(parent.height())
        
        self.formats_table.setItemWidth(550)
        self.formats_table.setItemHeight(parent.height() - self.formats_table.y() - 80)

    def init(self):

        format_result = subprocess.run(['oiiotool', '--list-formats'], capture_output=True)

        result = RlpCore.CoreDs_ModelDataItem.makeItem()
        for line in format_result.stdout.decode('utf-8').split('\n'):

            item = RlpCore.CoreDs_ModelDataItem.makeItem()
            item.insert('text', line)

            result.append(item)

        self.formats_table.setModelData(result)




def create(parent):
    settings_panel = OIIOSettingsPanel(parent)
    _PANELS.append(settings_panel)
    return settings_panel