
import logging

from rlp.Qt import QStr
from rlp import QtCore, QtGui

import rlp.core as RlpCore
from rlp.core.net.websocket import RlpClient

import rlp.util as RlpUtil

import rlp.gui as RlpGui
from rlp.gui.buttongroup import GUI_ButtonGroup


from revlens.gui.dialogs import LoadDialog

# from ..widgets.review.reviewlist import ReviewlistView
from .review.main import ReviewMainWidget
from .sequence import SequenceMediaView
from .stream import StreamView

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

        self.loadRequested = QtCore.PY_Signal(self)

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

        self.setWidth(width - 40)
        self.setHeight(height - 40)

        self.layout.updateItems()


    def get_selection(self):
        return self._selection


    def onThumbImageReady(self, runId, image):
        if runId in self._thumbs:
            self._thumbs[runId].thumb.setThumbImage(image, 230)
            self._thumbs[runId].thumb.update()


    def onLoadRequested(self, md):
        print(md)


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



class TableView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.entity_type = None

        self.grid_view = RlpGui.GUI_GRID_View(self, None) # EntityPanelGridView(self, 'Media')
        self.grid_view.setPos(10, 0)
        colModel = self.grid_view.colModel()

        colModel.addCol(
            {
                "col_name": "_image",
                "display_name": "Thumbnail",
                "col_type": "image_b64",
                "metadata": {},
                "width": 80
            },
            -1
        )

        colModel.addCol(
            {
                "col_name": "code",
                "display_name": "Name",
                "col_type": "str",
                "metadata": {},
                "width": 80
            },
            -1
        )

        '''
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
        '''

        colModel.addCol(
            {
                "col_name": "description",
                "display_name": "Description",
                "col_type": "str",
                "metadata": {},
                "width": 300
            },
            -1
        )

        colModel.addCol(
            {
                "col_name": "sg_path_to_frames",
                "display_name": "Path to Media",
                "col_type": "str",
                "metadata": {},
                "width": 300
            },
            -1
        )

        colModel.addCol(
            {
                "col_name": "frame_count",
                "display_name": "Length",
                "col_type": "int",
                "metadata": {},
                "width": 120
            },
            -1
        )

        self.grid_view.updateHeader()
        grid_toolbar = self.grid_view.toolbar()

        self.load_button = RlpGui.GUI_IconButton("", grid_toolbar, 20, 4)
        self.load_button.setText('Load..')
        self.load_button.setOutlined(True)
        self.load_button.buttonPressed.connect(self.onLoadRequested)

        grid_toolbar.layout().addItem(self.load_button, 0)

        self._rowIdxMap = {}

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def refresh(self, media_results=[]):

        self.grid_view.clearData()

        self._rowIdxMap = {}

        idx = 0

        modelData = RlpCore.CoreDs_ModelDataItem.makeItem()
        for entry in media_results:

            
            self._rowIdxMap[str(entry['id'])] = idx
            idx += 1

            row = RlpCore.CoreDs_ModelDataItem.makeItem()
            for modelKey, modelVal in entry.items():
                row.insert(modelKey, modelVal)

            if 'sg_path_to_frames' in entry:
                row.setMimeTypeKey('sg_path_to_frames')
                row.setMimeType('sg_path_to_frames', 'text/filepath')
                row.setMimeData(entry)


            modelData.append(row)

        
        # self.grid_view.setModelDataList(media_results)
        self.grid_view.setModelData(modelData)
        pass


    def onLoadRequested(self, md=None):
        
        fullSel = self.grid_view.fullSelectionValues()

        print('LOAD REQUESTED: {}'.format(fullSel))

        if not fullSel:
            print('Error: nothing selected!')
            return

        if fullSel[0]['type'] == 'Playlist':
            import rlplug_shotgrid.cmds
            rlplug_shotgrid.cmds.request_load(
                fullSel[0]['type'],
                fullSel[0],
                reviewList_name=fullSel[0]['code']
            )

        else:
            # TODO FIXME: Late import for iOS
            from rlplug_shotgrid.dialogs import ShotgridSequenceLoadDialog
            ShotgridSequenceLoadDialog.create(fullSel[0])

        # rlplug_shotgrid.cmds.request_load(
        #     self.entity_type,
        #     fullSel[0]
        # )


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

        print('TableView: {}'.format(width))

        self.grid_view.setWidth(width - 20)


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



class SearchView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)



class LibraryPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.edbc = RlpClient.instance()

        self.projectEntity = None
        appGlobals = RlpUtil.APPGLOBALS.globals()
        if 'project.info' in appGlobals:
            self.projectEntity = {'type': 'Project', 'id': appGlobals['project.info']['id']}

        self.thumbnailClient = RlpCore.CoreNet_HttpClient()
        self.thumbnailClient.requestReady.connect(self.onThumbRequestReady)

        # self.sourceSelector = RlpGui.GUI_IconButton('', self)

        self.setWidth(parent.width())
        self.setHeight(parent.height())

        self.reviewlistView = ReviewMainWidget(self) # DataSource must be set by a plugin
        self.reviewlistView.setPos(0, 50)
        self.reviewlistView.hideItem()

        self.sequenceView = SequenceMediaView(self)
        self.sequenceView.setPos(0, 50)
        self.sequenceView.hideItem()

        self.streamView = StreamView(self)
        self.streamView.setPos(0, 50)
        self.streamView.hideItem()

        self.searchView = SearchView(self)
        self.searchView.setPos(0, 50)
        self.searchView.hideItem()


        self.navGroup = GUI_ButtonGroup(self)
        self.navGroup.setPos(10, 10)
        self.navGroup.buttonPressed.connect(self.onNavItemRequested)
        # self.navGroup.setHeight(40)
        # self.navGroup.layout.setHeight(40)
        rbtn = self.navGroup.addButton('Reviewlist', ':feather/list.svg', iconSize=20, view=self.reviewlistView)
        ri = rbtn.svgIcon()
        ri.setYOffset(5)
        ri.setXOffset(4)
        ri.setHeight(30)

        seqbtn = self.navGroup.addButton('Sequence', ':feather/film.svg', iconSize=16, view=self.sequenceView)
        seqi = seqbtn.svgIcon()
        seqi.setYOffset(7)
        seqi.setXOffset(4)
        seqi.setHeight(30)
        seqi.setWidth(20)

        streambtn = self.navGroup.addButton('Stream', ':misc/stream.svg', iconSize=22, view=self.streamView)
        streami = streambtn.svgIcon()
        streami.setYOffset(4)
        streami.setXOffset(2)
        streami.setWidth(22)
        streami.setHeight(30)


        sbtn = self.navGroup.addButton('Search', ':feather/search.svg', iconSize=16, view=self.searchView)
        si = sbtn.svgIcon()
        si.setYOffset(7)
        si.setXOffset(4)
        si.setHeight(30)
        si.setWidth(20)

        self.navGroup.selectButton('Reviewlist')


    def onNavItemRequested(self, md):
        pass


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


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)




def create(parent):
    media_panel = LibraryPanel(parent)

    global _PANELS
    _PANELS.append(media_panel)

    return media_panel