
import os
import json
import pprint
import logging
import tempfile

from rlp.Qt import QStr
from rlp import QtCore, QtGui

import rlp.core as RlpCore
import rlp.gui as RlpGui
import rlp.util as RlpUtil

import revlens
import revlens.gui as RevlensGui

from revlens.gui.dialogs import LoadDialog
from rlp.core.net.websocket import RlpClient

from ..widgets.hierarchy_nav import HierarchyNavWidget

# from .entities.dialogs.delete_entity import DeleteEntityDialog
# from .entities.widgets.grid_view import EntityPanelGridView

_PANELS = []
_DIALOGS = []

LOG = logging.getLogger('rlp.{}'.format(__name__))



class MediaThumbItem(RlpGui.GUI_ItemBase):

    _utext = RlpCore.UTIL_Text()
    _thumbf = RlpGui.GUI_ThumbnailItem()

    def __init__(self, media_entry, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.loadRequested = QtCore.PySignal()

        self._media_entry = media_entry

        if 'thumb_frame_data' in media_entry:
            self.thumb = self._thumbf.fromBase64(
                self,
                media_entry['thumb_frame_data'],
                230
            )
        else:
            self.thumb = self._thumbf.fromPath(
                self,
                ':misc/thumb_image.png',
                230
            )

        self.thumb.setData(media_entry)

        otext = media_entry.get('title', '-')
        ntext = self._utext.clipToWidth(
            otext, 170)

        if ntext != otext:
            ntext += " ..."


        self.thumb.setLabel(ntext)

        self.thumb.hoverEnter.connect(self.on_thumb_hover_enter)
        self.thumb.hoverLeave.connect(self.on_thumb_hover_leave)

        self.load_button = RlpGui.GUI_IconButton(
            '', self.thumb, 20, 4)

        self.load_button.setText("Load..")
        self.load_button.setOutlined(True)  
        self.load_button.setZValue(self.z() + 1)
        self.load_button.buttonPressed.connect(self.on_load_requested)

        thumb_width = self.thumb.width()
        thumb_height = self.thumb.height()

        self.load_button.setPos(
            (thumb_width / 2) - 30,
            (thumb_height / 2) - 10
        )

        self.load_button.hideItem()

        self.setItemHeight(self.thumb.height() + 30)
        self.setItemWidth(self.thumb.width())


    def get_selection(self):
        return self._media_entry

    def on_thumb_hover_enter(self, payload):
        self._selection = payload
        self.load_button.showItem()


    def on_thumb_hover_leave(self, payload):
        pass
        
        if not self.load_button.inHover():
            self.load_button.hideItem()


    def on_load_requested(self, md):

        sel_data = self.get_selection()
        if not sel_data:
            self.LOG.error('nothing selected, cannot load')

            # NOTE: EARLY RETURN
            return

        self.loadRequested.emit(self._media_entry)



class ThumbView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self._mpanel = parent

        self.layout = RlpGui.GUI_FlowLayout(self)
        self.layout.setSpacing(20)

        self._selection = None
        self._active = False

        self._thumbs = {}


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)

        self.layout.updateItems()


    def get_selection(self):
        return self._selection


    def onThumbImageReady(self, runId, image):
        if runId in self._thumbs:
            self._thumbs[runId].thumb.setThumbImage(image)
            self._thumbs[runId].thumb.update()

    def onLoadRequested(self, md):
        print(md)

        import rlplug_dpix.cmds
        rlplug_dpix.cmds.loadSequence(md)


    def refresh(self, media_results):

        self.layout.clear()
        self._thumbs = {}

        for media_entry in media_results:
            # if media_entry.get('create_date'):
            #     media_entry['create_date'] = QtCore.QDateTime.fromSecsSinceEpoch(
            #         media_entry['create_date']).toString()


            # print(media_entry)

            thumbId = str(media_entry['id'])
            thumbw = MediaThumbItem(media_entry, self)
            thumbw.loadRequested.connect(self.onLoadRequested)
            self.layout.addItem(thumbw)
            self._thumbs[thumbId] = thumbw

        self.layout.updateItems()



class _TableView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.grid_view = RlpGui.GUI_GRID_View(self) # EntityPanelGridView(self, 'Media')
        self.grid_view.setPos(10, 10)
        colModel = self.grid_view.colModel()
        colModel.addCol(
            {
                "col_name": "code",
                "display_name": "Name",
                "col_type": "str",
                "metadata": {},
                "width": 110
            },
            -1
        )

        colModel.addCol(
            {
                "col_name": "_image",
                "display_name": "Thumbnail",
                "col_type": "image_b64",
                "metadata": {},
                "width": 230
            },
            -1
        )

        colModel.addCol(
            {
                "col_name": "sg_frame_range",
                "display_name": "Frame Range",
                "col_type": "str",
                "metadata": {},
                "width": 230
            },
            -1
        )

        colModel.addCol(
            {
                "col_name": "sg_assignees",
                "display_name": "Assignees",
                "col_type": "str",
                "metadata": {},
                "width": 300
            },
            -1
        )


        self.grid_view.updateHeader()

        self._rowIdxMap = {}

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def refresh(self, media_results):
        self._rowIdxMap = {}

        idx = 0
        for entry in media_results:
            self._rowIdxMap[str(entry['id'])] = idx
            idx += 1

        self.grid_view.setModelDataList(media_results)
        pass

    def onThumbImageReady(self, runId, image):

        if runId not in self._rowIdxMap:
            return

        rowIdx = self._rowIdxMap[runId]
        row = self.grid_view.rowAt(rowIdx)
        if not row:
            print('ERROR NO ROW AT IDX {}'.format(rowIdx))
            return

        cell = row.cell("_image")

        cell.insertValue("image", image)
        cell.insertValue("image.original", image)
        cell.insertValue("value", "")
        cell.update()


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)
        
        self.grid_view.setWidth(width - 20)


class TableView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

    def refresh(self, media_results):
        pass
    
    def onThumbImageReady(self, runId, image):
        pass

    def onParentSizeChangedItem(self, width, height):
        pass

class MediaFilter(RlpGui.GUI_ItemBase):

    def __init__(self, parent, entity, op, value):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.entity = entity
        self.op = op
        self.value = value

        self.entity_label = RlpGui.GUI_Label(self, self.entity)
        elf = self.entity_label.font()
        elf.setBold(True)
        self.entity_label.setFont(elf)

        self.label = RlpGui.GUI_Label(self, self.value)
        self.label.setPos(5, 4)

        self.layout = RlpGui.GUI_HLayout(self)
        self.layout.setPos(4, 5)
        self.layout.addItem(self.entity_label, 0)
        self.layout.addItem(self.label, 0)

        self.setHeight(30)
        self.setWidth(160)


    @property
    def text(self):
        return '{} {}'.format(self.entity, self.value)

    def paintItem(self, painter):

        p = QtGui.QPen((QtGui.QColor(60, 60, 80)))
        painter.setPen(p)
        painter.drawRoundedRect(self.boundingRect(), 10, 10)


class MediaPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._destructed = False

        self.edbc = RlpClient.instance()

        self.project_entity = None
        app_globals = RlpUtil.APPGLOBALS.globals()
        if 'project.info' in app_globals:
            self.project_entity = {'type': 'Project', 'id': app_globals['project.info']['id']}

        self.http_client = RlpCore.CoreNet_HttpClient()
        self.http_client.requestReady.connect(self.onThumbRequestReady)


        self.setWidth(parent.width())
        self.setHeight(parent.height())

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.view_type = None

        self.searchFilters = []

        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.addSpacer(10)

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":feather/lightgrey/refresh-cw.svg",
            parent,
            20
        )
        self.refresh_button.buttonPressed.connect(self.refresh)
        self.toolbar_layout.addItem(self.refresh_button, 0)
        self.toolbar_layout.addSpacer(10)

        self.thumb_button = RlpGui.GUI_SvgButton(
            ":feather/lightgrey/grid.svg",
            parent,
            20
        )
        self.thumb_button.buttonPressed.connect(self.onThumbViewRequested)

        self.toolbar_layout.addItem(self.thumb_button, 0)
        self.toolbar_layout.addSpacer(10)

        self.table_button = RlpGui.GUI_SvgButton(
            ":feather/lightgrey/align-justify.svg",
            parent,
            20
        )
        self.table_button.buttonPressed.connect(self.onTableViewRequested)
        
        self.toolbar_layout.addItem(self.table_button, 0)

        # self.toolbar_layout.addDivider(20, 20)

        # self.load_button = RlpGui.GUI_IconButton('', self, 20, 4)
        # self.load_button.setTextYOffset(-1)
        # self.load_button.setText('Load')
        # self.load_button.setOutlined(True)
        # self.load_button.buttonPressed.connect(self.onLoadRequested)
        # self.toolbar_layout.addItem(self.load_button, 0)

        self.toolbar_layout.addDivider(20, 20)

        self.status_label = RlpGui.GUI_Label(self, 'Ready')
        self.status_label.setPos(parent.width() - 100, 5)

        # self.search_bar = RlpGui.GUI_TextEdit(self, 200, 25)
        # self.search_bar.setTempHintText("Search")
        # self.search_bar.scrollArea().setHScrollBarVisible(False)

        self.hierarchy_nav = HierarchyNavWidget(self)
        self.hierarchy_nav.status.connect(self.onNavStatus)
        self.hierarchy_nav.navItemsReady.connect(self.onNavItemsReady)
        self.hierarchy_nav.endSelected.connect(self.onNavItemSelected)
        self.hierarchy_nav.initHierarchyMenu()

        # self.toolbar_layout.addItem(self.search_bar, 0)
        self.toolbar_layout.addItem(self.hierarchy_nav, 0)
        self.toolbar_layout.addItem(self.status_label, 0)

        # self.filters = RlpGui.GUI_HLayout(self)
        # self._filters = []

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.toolbar_layout, 0)
        
        # self.layout.addSpacer(5)
        # self.layout.addItem(self.filters, 10)

        self.thumb_view = ThumbView(self)
        self.layout.addItem(self.thumb_view, 0)


        self.thumb_view.hideItem()

        self.table_view = TableView(self)
        self.layout.addItem(self.table_view, 0)
        self.table_view.hideItem()

        self._curr_view = self.thumb_view

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def destruct(self):
        self._destructed = True


    def onFieldDataReady(self, md):

        # self.onParentSizeChangedItem(self.width(), self.height())
        self.onThumbViewRequested()
        self.refresh()
        

    def updateView(self):

        if self._curr_view == self.thumb_view:
            self.onThumbViewRequested()

        elif self._curr_view == self.table_view:
            self.onTableViewRequested()


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)

        # self.search_bar.setWidth(width - 200)
        self.status_label.setPos(width - 100, 0)

        self.updateView()
        

    def refresh(self, sig_info=None):
        
        
        if not self.edbc:
            self.status_label.setText("Error: Login required")
            return

        print('Querying for Media..')

        self.status_label.setText('Loading..')
        self.status_label.update()

        def _on_media_loaded(media_results):

            # pprint.pprint(media_results)
            # print('-------------')
            if not media_results:
                return

            # media_results = list(reversed(sorted(media_results, key=lambda x: x.get('created_at', ''))))

            count = 0
            total = len(media_results)

            for media_entry in media_results:
                count += 1
                media_entry['title'] = media_entry['code']
                if 'create_date' in media_entry:
                    media_entry['create_date'] = QtCore.QDateTime.fromSecsSinceEpoch(
                        media_entry['create_date']).toString().toStdString()

                if media_entry.get('image'):
                    self.http_client.requestImage(media_entry['image'], str(media_entry['id']))

            self.thumb_view.refresh(media_results)
            self.table_view.refresh(media_results)

            self.status_label.setText('Ready')

            self.update()



        # return_field_list = ['1uuid', 'title', 'name', 'comment', 'create_date', 'frame_count', 'thumb_frame_data', 'size', 'rfs_uuid']
        
        print('Media search filters:')
        pprint.pprint(self.searchFilters)


        # def _on_projects_ready(result):
        #     print('PROJECTS READY')
        #     self.status_label.setText('')
        #     print(result)

        # self.edbc.call(_on_projects_ready, 'sg.find', 'Project', [], ['code']).run()

        sequence_entity = {'type': 'Sequence', 'id': 11398}

        self.searchFilters = [
            ['project', 'is', project_entity],
            ['sg_sequence', 'is', sequence_entity],
        ]

        print(self.searchFilters)

        seq_filter = MediaFilter(self, 'Sequence', 'is', 'ID: 11398')
        self.filters.addItem(seq_filter, 0)
        self.filters.addSpacer(5)
        self._filters.append(seq_filter)

        # self.table_view.grid_view.searchReturnFields

        self.edbc.call(_on_media_loaded, 'sg.find', 
            'Shot', self.searchFilters, ['id', 'code', 'image', 'created_at', 'sg_frame_range', 'sg_assignees']
        ).run()



    def get_selection(self):
        return self.thumb_view.get_selection()


    def onThumbRequestReady(self, data):
        self.thumb_view.onThumbImageReady(data['run_id'], data['image'])
        self.table_view.onThumbImageReady(data['run_id'], data['image'])


    def onThumbViewRequested(self, md=None):

        self._curr_view = self.thumb_view

        self.table_view.hideItem()
        self.table_view.setItemWidth(0)
        self.table_view.setItemHeight(0)

        self.thumb_view.showItem()
        self.thumb_view.onParentSizeChangedItem(self.width(), self.height())


    def onTableViewRequested(self, md=None):

        self._curr_view = self.table_view

        self.thumb_view.hideItem()
        self.thumb_view.setItemWidth(0)
        self.thumb_view.setItemHeight(0)


        self.table_view.showItem()
        self.table_view.onParentSizeChangedItem(self.width(), self.height())


    def onNavStatus(self, md):
        self.status_label.setText('Loading {}..'.format(md['name']))


    def onNavItemSelected(self, md):
        print('nav item selected')
        print(md)
        self.view_entity = md[1]

        print(self.view_entity)


    def onNavItemsReady(self, md):

        self.status_label.setText('Loading..')
        self.status_label.update()

        entityResultList = md['data']
        for media_entry in entityResultList:
            # count += 1
            media_entry['title'] = media_entry['code']
            if 'create_date' in media_entry:
                media_entry['create_date'] = QtCore.QDateTime.fromSecsSinceEpoch(
                    media_entry['create_date']).toString().toStdString()

            if media_entry.get('image'):
                self.http_client.requestImage(media_entry['image'], str(media_entry['id']))

        self.thumb_view.refresh(entityResultList)
        self.table_view.refresh(entityResultList)

        self.status_label.setText('')

        # self.view_entity = {'type': md['type'], 'id': md.get('id')}

        self.update()


    def onLoadRequested(self, md):
        print('LOAD')
        print(self.view_entity)
        
        # with open('/tmp/seq_info.json') as fh:
        #     result = json.load(fh)
        #     media_list = []
        #     # pprint.pprint(result)
        #     self.status_label.setText('Done')

        #     for entry in result[1:20]:
        #         media_list.append({
        #             'media.name': entry['code'],
        #             'media.frame_count': entry.get('frame_count', 24),
        #             'media.video': entry['sg_path_to_frames'],
        #             'media.audio': entry['sg_path_to_frames']
        #         })

        #     # revlens.cmds.load_media_list(media_list, 0, 1)
        #     revlens.CNTRL.getMediaManager().loadMediaList(media_list)


        if self.view_entity['type'] == 'Sequence':
            print('LOADING SEQUENCE')

            

def show_delete_dialog(sel_data):

    floating_pane = revlens.VIEW.createFloatingPane(400, 400, False)
    diag = DeleteEntityDialog('Media', sel_data, floating_pane)
    
    _DIALOGS.append(diag)

    return diag


def create(parent):
    media_panel = MediaPanel(parent)

    global _PANELS
    _PANELS.append(media_panel)

    return media_panel