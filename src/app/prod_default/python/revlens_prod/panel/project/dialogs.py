
import os
import base64

from rlp.Qt import QStr

from rlp import QtCore
import rlp.gui as RlpGui


from rlp.core.net.websocket import RlpClient


class NewProjectDialog(RlpGui.GUI_ItemBase):

    _UI = []

    LABEL_WIDTH = 140
    def __init__(self, fpane):
        RlpGui.GUI_ItemBase.__init__(self, fpane.body())

        self.created = QtCore.PySignal()

        self._fpane = fpane
        self._fpane.setText('New Project')
        self._fpane.setTextBold(True)


        self.name_label = RlpGui.GUI_Label(self, 'Short Name:')
        self.name_label.setWidth(self.LABEL_WIDTH)
        self.name_text = RlpGui.GUI_TextEdit(self, 170, 60)
        self.name_layout = RlpGui.GUI_HLayout(self)
        self.name_layout.addItem(self.name_label, 0)
        self.name_layout.addItem(self.name_text, 0)

        self.fullname_label = RlpGui.GUI_Label(self, 'Full Name:')
        self.fullname_label.setWidth(self.LABEL_WIDTH)
        self.fullname_text = RlpGui.GUI_TextEdit(self, 170, 60)
        self.fullname_layout = RlpGui.GUI_HLayout(self)
        self.fullname_layout.addItem(self.fullname_label, 0)
        self.fullname_layout.addItem(self.fullname_text, 0)


        self.banner_strip_label = RlpGui.GUI_Label(self, 'Banner Strip Path:')
        self.banner_strip_label.setWidth(self.LABEL_WIDTH)
        self.banner_strip_text = RlpGui.GUI_TextEdit(self, 170, 60)
        self.banner_strip_layout = RlpGui.GUI_HLayout(self)
        self.banner_strip_layout.addItem(self.banner_strip_label, 0)
        self.banner_strip_layout.addItem(self.banner_strip_text, 0)

        self.icon_label = RlpGui.GUI_Label(self, 'Icon Path:')
        self.icon_label.setWidth(self.LABEL_WIDTH)
        self.icon_text = RlpGui.GUI_TextEdit(self, 170, 60)
        self.icon_layout = RlpGui.GUI_HLayout(self)
        self.icon_layout.addItem(self.icon_label, 0)
        self.icon_layout.addItem(self.icon_text, 0)

        self.create_button = RlpGui.GUI_IconButton('', self, 20, 4)
        self.create_button.setText('Create')
        self.create_button.setOutlined(True)
        self.create_button.buttonPressed.connect(self.on_create)

        self.cancel_button = RlpGui.GUI_IconButton('', self, 20, 4)
        self.cancel_button.setText('Cancel')
        self.cancel_button.setOutlined(True)
        self.cancel_button.buttonPressed.connect(self.on_cancel)

        self.button_layout = RlpGui.GUI_HLayout(self)
        self.button_layout.addSpacer(80)
        self.button_layout.addItem(self.create_button, 0)
        self.button_layout.addSpacer(30)
        self.button_layout.addItem(self.cancel_button, 0)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addSpacer(10)
        self.layout.addItem(self.name_layout, 0)
        self.layout.addItem(self.fullname_layout, 0)
        self.layout.addItem(self.banner_strip_layout, 0)
        self.layout.addItem(self.icon_layout, 0)
        self.layout.addItem(self.button_layout, 0)

        self.setItemWidth(fpane.width())
        self.setItemHeight(fpane.height())


    def close(self):
        self._fpane.requestClose()

    def on_cancel(self, md=None):
        self.close()


    def on_create(self, md=None):
        print('CREATE PROJECT')

        name = self.name_text.text()
        fullname = self.fullname_text.text()

        if not name or not fullname:
            print('invalid input')
            self.close()

        project_banner_strip_path = self.banner_strip_text.text()
        project_icon_path = self.icon_text.text()

        if os.path.isfile(project_banner_strip_path):
            print('Not Found: {}'.format(project_banner_strip_path))
    
        project_banner_strip = None
        if os.path.isfile(project_banner_strip_path):
            with open(project_banner_strip_path, 'rb') as fh:
                project_banner_strip = base64.b64encode(fh.read()).decode('utf-8')

        project_icon = None
        if os.path.isfile(project_icon_path):
            with open(project_icon_path, 'rb') as fh:
                project_icon = base64.b64encode(fh.read()).decode('utf-8')


        project_info = {
            'name': self.name_text.text(),
            'fullname': self.fullname_text.text()
        }

        if project_banner_strip:
            print('PROJECT BANNER STRIP OK')
            project_info['image_banner_strip'] = project_banner_strip

        if project_icon:
            print('PROJECT ICON OK')
            project_info['image_icon'] = project_icon


        edbc = RlpClient.instance()
        
        def _on_project_created(result):
            self.created.emit({})
            self.close()

        edbc.call(_on_project_created, 'edbc.create', 'Project', '', project_info, '').run()


    @classmethod
    def create(cls):

        parent = RlpGui.VIEW.createFloatingPane(340, 360, False)
        pane = cls(parent)
        cls._UI.append(pane)

        return pane