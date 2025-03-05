
import os
import json
import uuid

from rlp import QtCore, QtGui
import rlp.gui as RlpGui
import rlp.core as RlpCore
import rlp.util as RlpUtil

import revlens.gui

from rlp.core.net.websocket import RlpClient

from .dialogs import NewProjectDialog


class ProjectItem(RlpGui.GUI_ItemBase):

    _thumbf = RlpGui.GUI_ThumbnailItem()
    _uimage = RlpCore.UTIL_Image()

    def __init__(self, parent, project_info, fallback_image=':misc/3d-modeling.svg'): # :feather/lightgrey/package.svg'
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.http_client = parent.http_client
        self.setItemAcceptHoverEvents(True)

        self.selected = QtCore.PySignal()

        self._hover = False

        self._project_info = project_info

        self.thumb = None
        self.fthumb = None

        self.run_id = str(uuid.uuid4())

        # if 'image' in project_info:
        #     self.thumb = self._uimage.fromBase64(
        #         project_info['image_icon']).scaledToWidth(240)

        # else:
        #     self.fthumb = RlpGui.GUI_SvgIcon(
        #         fallback_image, self, 140, 0, QtGui.QColor(140, 140, 140))
        #     self.fthumb.setPos(58, 50)
        #     self.fthumb.buttonPressed.connect(self.mousePressEventItem)


        self.label = RlpGui.GUI_Label(self, '')
        self.label.setDropShadow(True)

        size = 256

        lf = self.label.font()
        lf.setPixelSize(14)
        lf.setBold(True)
        self.label.setFont(lf)
        self.label.setText(project_info['name'])
        self.label.setPos(10, size - 30)

        self._colHoverOff = QtGui.QColor(0, 0, 0)
        self._colHoverOn = QtGui.QColor(80, 80, 80)

        self.setItemWidth(size)
        self.setItemHeight(size)


    def hoverEnterEventItem(self, event):
        self._hover = True
        self.updateItem()

    def hoverLeaveEventItem(self, event):
        self._hover = False
        self.updateItem()

    def mousePressEventItem(self, event):
        self.selected.emit(self._project_info)


    def boundingRect(self):
        return QtCore.QRectF(0, 0, self.width(), self.height())

    def paintItem(self, painter):

        # painter.setAntiAliasing(True)

        col = self._colHoverOff
        if self._hover:
            col = self._colHoverOn

        painter.setPen(QtGui.QPen(col))
        painter.setBrush(QtGui.QBrush(col))

        painter.drawRoundedRect(self.boundingRect(), 15, 15)

        if self.thumb:
            painter.drawImage(12, 10, self.thumb)


class ProjectChooserPanel(RlpGui.GUI_ItemBase):

    _PANE = None

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.projectSelected = QtCore.PySignal()

        self.http_client = RlpCore.CoreNet_HttpClient()
        self.http_client.requestReady.connect(self.on_request_ready)

        self._projects = {}

        self.layout = RlpGui.GUI_FlowLayout(self)
        self.layout.setSpacing(25)
        self.layout.setOutlined(False)

        self.onParentSizeChangedItem(parent.width() - 10, parent.height())

        self.init()


    @classmethod
    def create_panel(cls):

        if cls._PANE:
            cls._PANE.setVisible(True)
            return

        pane = RlpGui.VIEW.createFloatingPane(900, 600, False)
        pane.setPersistent(True)
        pane.setText("Select Project..")

        pcp = cls(pane.body())

        cls._PANE = pane

        def _project_selected(md=None):
            import revlens.gui
            revlens.gui.init_menu_project_list()

            pane.requestClose()

        pcp.projectSelected.connect(_project_selected)

        return pcp

    def on_request_ready(self, data):

        run_id = data['run_id']

        if run_id not in self._projects:
            print('WARNING: run_id not found: {}'.format(run_id))
            return

        self._projects[run_id].thumb = data['image']
        self._projects[run_id].update()


    def init(self):

        self.layout.clear()
        self._projects = {}

        def _projects_ready(result):

            if not result:
                return

            for entry in result:

                proj_item = ProjectItem(self, entry)
                proj_item.selected.connect(self.on_project_selected)
                self.layout.addItem(proj_item)
                self._projects[proj_item.run_id] = proj_item

                if 'image' in entry and entry['image']:
                    self.http_client.requestImage(entry['image'], proj_item.run_id)

            # new_project = ProjectItem(self, {'fullname':'New Project..', 'name': 'new_project'}, fallback_image=':misc/add.svg')
            # new_project.selected.connect(self.on_new_project_selected)
            # self.layout.addItem(new_project)

            # self._projects.append(new_project)

            self.onParentSizeChangedItem(self.width(), self.height())


        edbc = RlpClient.instance()
        edbc.call(
            _projects_ready, 'sg.find',
            'Project', [['sg_status', 'is', 'Active'], ['sg_type', 'in', ['Feature Film', 'Television']]],
            ['name', 'code', 'image']
        ).run()


    def on_project_selected(self, project_info):

        print(project_info)

        ag = RlpUtil.get_app_globals()
        ag['project.name'] = project_info['code']
        ag['project.info'] = project_info

        RlpUtil.set_app_globals(ag)

        appctx_path = os.path.join(revlens._init_state_dir(), 'appcontext.json')
        with open(appctx_path, 'w') as wfh:
            wfh.write(json.dumps(ag))

        print('Wrote {}'.format(appctx_path))

        self.projectSelected.emit({
            'project.name': project_info['code'],
            'project.info': project_info
        })

        # load project page via sig/slot connection
        # so widgets get cleaned up and app doesnt crash
        # revlens.VIEW.emitSelectionChanged({
        #     'source': 'project_chooser',
        #     'data': project_info['name']
        # })



    def on_new_project_selected(self, md=None):

        def _on_created(md):
            self.init()

        diag = NewProjectDialog.create()
        diag.created.connect(_on_created)


    def onParentSizeChangedItem(self, width, height):

        if not self.isItemVisible():
            return

        self.setItemWidth(width)
        self.setItemHeight(height)

        self.layout.updateItems()

def create(parent):

    
    project_chooser_panel = ProjectChooserPanel(parent)

    # global _PANELS
    # _PANELS.append(project_chooser_panel)

    return project_chooser_panel