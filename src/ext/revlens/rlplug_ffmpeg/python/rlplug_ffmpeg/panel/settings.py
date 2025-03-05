
import subprocess

from rlp.Qt import QStr, QVMap, QV
import rlp.core as RlpCore
import rlp.gui as RlpGui


import revlens


_PANELS = []

class FFMpegSettingsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent.body())

        plugin = revlens.CNTRL.getPluginManager().getMediaPlugin('FFMpeg')

        self.version_label = RlpGui.GUI_Label(self, 'Version: 4.2.2')
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

        self.codecs_table = RlpGui.GUI_GRID_View(self)
        ccolmodel = self.codecs_table.colModel()
        ccolmodel.clear()
        ccolmodel.addCol(
            {
                'col_name': 'text',
                'display_name': 'Codec Info',
                'col_type': 'str',
                'width': 400
            },
            -1
        )

        self.codecs_table.updateHeader()

        self.init()

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(40)

        self.layout.addItem(self.version_label, 0)
        self.layout.addSpacer(10)
        self.info_layout = RlpGui.GUI_HLayout(self)
        self.info_layout.addItem(self.formats_table, 0)
        self.info_layout.addSpacer(20)
        self.info_layout.addItem(self.codecs_table, 0)

        self.layout.addItem(self.info_layout, 20)

        parent.setItemHeight(400)
        parent.setItemWidth(800)
        cbtn = parent.closeButton()
        cbtn.setPos(parent.width() - cbtn.width() - 10, 6)


        self.setItemWidth(parent.width())
        self.setItemHeight(parent.height())
        
        self.formats_table.setItemWidth(350)
        self.codecs_table.setItemWidth(400)

        self.formats_table.setItemHeight(parent.height() - self.formats_table.y() - 30)
        self.codecs_table.setItemHeight(parent.height() - self.formats_table.y() - 30)
    
    def init(self):

        format_result = subprocess.run(['ffmpeg', '-formats'], capture_output=True)

        result = RlpCore.CoreDs_ModelDataItem.makeItem()
        for line in format_result.stdout.decode('utf-8').split('\n'):

            item = RlpCore.CoreDs_ModelDataItem.makeItem()
            item.insert('text', line)

            result.append(item)

        self.formats_table.setModelData(result)


        codecs_result = subprocess.run(['ffmpeg', '-codecs'], capture_output=True)

        result = RlpCore.CoreDs_ModelDataItem.makeItem()
        for line in codecs_result.stdout.decode('utf-8').split('\n'):

            item = RlpCore.CoreDs_ModelDataItem.makeItem()
            item.insert('text', line)

            result.append(item)

        self.codecs_table.setModelData(result)



def create(parent):
    settings_panel = FFMpegSettingsPanel(parent)
    _PANELS.append(settings_panel)
    return settings_panel