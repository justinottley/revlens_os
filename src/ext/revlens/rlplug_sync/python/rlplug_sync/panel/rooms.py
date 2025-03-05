
import os
import time
import pprint
import threading
import functools
import logging
import urllib.parse

from rlp import QtCore, QtGui
import rlp.gui as RlpGui
import rlp.util

import revlens

import rlplug_sync
import rlplug_qtbuiltin.cmds

from rlp.core.net.websocket import RlpClient

from .room_controls import RoomControlsWidget
from .dialogs import ConfirmDialog, RenameDialog, NewRoomDialog

WIDGETS = []
DIALOGS = []

LOG = logging.getLogger('rlp.{}'.format(__name__))


class RoomWidget(RlpGui.GUI_ItemBase):

    _thumbf = RlpGui.GUI_ThumbnailItem()

    BASE_HEIGHT = 60
    EXPAND_HEIGHT = 150

    def __init__(self, parent, room_name, room_uuid, room_media_list, created_at_str):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._parent = parent

        self.setItemAcceptHoverEvents(True)

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.room_name = room_name
        self.room_uuid = room_uuid
        self.room_media = room_media_list

        self.created_at_str = created_at_str

        self._expanded = False

        self.name_label = RlpGui.GUI_Label(self, '')
        nlf = self.name_label.font()
        nlf.setPixelSize(nlf.pixelSize() + 2)
        nlf.setBold(True)
        self.name_label.setFont(nlf)
        self.name_label.setText(self.room_name)
        self.name_label.setPos(40, 10)


        num_items = len(self.room_media)
        self.item_count_label = RlpGui.GUI_SvgButton(
            ':feather/lightgrey/film.svg', self, 20)

        self.item_count_label.setOutlined(False)
        self.item_count_label.setOverlayText(
            str(num_items))

        self.item_count_label.setIconPos(0, 7)
        self.item_count_label.setOverlayTextPos(10, 7)
        self.item_count_label.setToolTipText(
            "{} items".format(num_items))

        self.item_count_label.hideItem()
        self.item_count_label.icon().setBgColour(QtGui.QColor(120, 120, 120))
        self.item_count_label.icon().setFgColour(QtGui.QColor(120, 120, 120))
        # self.item_count_label.icon().setFgColour(QtGui.QColor(180, 180, 180))

        self.item_count_label.setPos(200, 13)

        self.action_button = RlpGui.GUI_SvgButton(
            ':feather/lightgrey/more-vertical.svg', self, 20)

        self.action_button.setMenuXPosOffset(25)
        self.action_button.setMenuYPosOffset(-20)

        # self.action_button.setZValue(5)
        self.action_button.icon().setBgColour(QtGui.QColor(140, 140, 140))
        # self.action_button.icon().setFgColour(QtGui.QColor(120, 120, 120))
        self.action_button.icon().setFgColour(QtGui.QColor(250, 250, 250))

        action_menu = self.action_button.menu()
        rename_action = action_menu.addItem(
            'Rename',
            {},
            False
        )
        
        delete_action = action_menu.addItem(
            'Delete',
            {},
            False
        )

        for action in [rename_action, delete_action]:

            action.insertDataValue('room_name', self.room_name)
            action.insertDataValue('room_uuid', self.room_uuid)
            action.menuItemSelected.connect(self.on_action_requested)


        self.action_button.setPos(230, 20)
        self.action_button.hideItem()

        self.created_at_label = RlpGui.GUI_Label(self, '')
        caf = self.created_at_label.font()
        caf.setPixelSize(caf.pixelSize() - 4)
        self.created_at_label.setFont(caf)
        self.created_at_label.setText(self.created_at_str)
        self.created_at_label.setPos(40, 30)


        self.toggle_expand_button = RlpGui.GUI_SvgButton(
            ':feather/lightgrey/plus-circle.svg', self, 20
        )
        self.toggle_expand_button.setPos(10, 20)
        self.toggle_expand_button.icon().setBgColour(QtGui.QColor(100, 100, 100))
        self.toggle_expand_button.setToolTipText("Show Room Details")

        self.toggle_collapse_button = RlpGui.GUI_SvgButton(
            ':feather/lightgrey/minus-circle.svg', self, 20
        )
        self.toggle_collapse_button.hideItem()
        self.toggle_collapse_button.setPos(10, 20)
        self.toggle_collapse_button.icon().setBgColour(QtGui.QColor(100, 100, 100))
        self.toggle_collapse_button.setToolTipText("Hide Room Details")

        render_as_pixmap = True
        self.enter_button = RlpGui.GUI_SvgButton(
            ':misc/enter.svg', self, 30
        )

        # self.enter_button.icon().setColour(QtGui.QColor(110, 110, 110))
        self.enter_button.icon().setFgColour(QtGui.QColor(220, 220, 255))
        self.enter_button.icon().setBgColour(QtGui.QColor(110, 110, 110))
        

        self.enter_button.setToolTipText("Join Room")

        self.enter_button.setPos(self.width() - 45, 15)

        self.thumb_sa = RlpGui.GUI_ScrollArea(self)
        self.thumb_sa.setBgColour(QtGui.QColor(50, 50, 50))
        self.thumb_sa.setOutlined(True)
        self.thumb_sa.setPos(5, 60)
        self.thumb_sa.hhandle().setAutoScroll(False)
        self.thumb_sa.setMaxChildWidth(600)
        self.thumb_sa.hideItem()

        self.thumb_layout = RlpGui.GUI_HLayout(self.thumb_sa.content())
        self.thumb_list = []


        self._init_connections()

        self.setWidth(parent.width())
        self.setHeight(self.BASE_HEIGHT)

        self.onParentSizeChangedItem(self.width(), self.height())
        # self.updateItem()

    @property
    def EDBC(self):
        return RlpClient.instance()


    def _init_connections(self):

        self.action_button.buttonPressed.connect(self._on_action_button_clicked)

        self.enter_button.buttonPressed.connect(self.on_enter_requested)
        self.enter_button.hoverChanged.connect(self._on_enter_hover)

        self.toggle_expand_button.buttonPressed.connect(self.on_toggle_expand_requested)
        self.toggle_expand_button.hoverChanged.connect(self._on_toggle_expand_hover_changed)
        self.toggle_collapse_button.hoverChanged.connect(self._on_toggle_collapse_hover_changed)
        self.toggle_collapse_button.buttonPressed.connect(self.on_toggle_expand_requested)


    def _on_action_button_clicked(self, md):
        self.action_button.toggleMenu({})

    def _on_toggle_expand_hover_changed(self, value):
        
        self.show_all()

        self.toggle_collapse_button.setHover(value)

    def _on_toggle_collapse_hover_changed(self, value):
        self.show_all()
        self.toggle_expand_button.setHover(value)


    def show_all(self):
        self.item_count_label.showItem()
        self.action_button.showItem()

        self.updateItem()

    def hide_all(self):
        self.item_count_label.hideItem()
        self.action_button.hideItem()

        self.updateItem()

    def hoverEnterEventItem(self, event):
        self.show_all()

    def hoverLeaveEventItem(self, event):
        self.hide_all()
        self.toggle_expand_button.setHover(False)
        self.toggle_collapse_button.setHover(False)


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width - 25)

        self.enter_button.setPos(width - 85, 15)
        self.thumb_sa.onParentSizeChanged(width, height - 50) # 80 # height)


    def _sanitize_uuid_list(self, uuid_list):
        result = []
        for entry in uuid_list:
            if not entry.startswith('{'):
                entry = '{' + entry + '}'
            
            result.append(entry)

        return result


    def _on_enter_hover(self, val):
        self.show_all()


    def on_action_requested(self, sig_info):

        name = sig_info['text']
        info = sig_info['data']

        if name == 'Delete':

            floating_pane = revlens.VIEW.createFloatingPane(200, 200, False)
            diag = ConfirmDialog(
                floating_pane,
                'Delete Room',
                'Delete Room "{}" - are you sure?'.format(info['room_name']),
                accept_label='Delete'
            )

            def _handle_delete_room(sig_info):
                self._parent.on_room_delete_requested(info['room_name'])

            diag.accept.connect(_handle_delete_room)


        elif name == 'Rename':

            floating_pane = revlens.VIEW.createFloatingPane(360, 220, False)
            diag = RenameDialog(
                'Rename Room',
                'Current name: "{}"'.format(info['room_name']),
                floating_pane,
                okay_text='Rename'
            )

            def _handle_dialog_done(sig_info):
                do_rename = sig_info['result']
                if do_rename:
                    new_room_name = diag.new_room_text.text()

                    self._parent.on_room_rename_requested(
                        info['room_name'],
                        info['room_uuid'],
                        new_room_name
                    )

            diag.sig_done.connect(_handle_dialog_done)


    def on_enter_requested(self, md):

        self._parent.curr_room_label.hideItem()
        self._parent.curr_room_label.setText(self.room_name)
        self._parent.curr_room_label.showItem()

        room_msg_data = {
            'room_name': self.room_name,
            'room_uuid': self.room_uuid,
            'sync': True
        }

        print(room_msg_data)
        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.callFromClient(
            'rooms.load_room',
            [],
            room_msg_data
        )

        self._parent.enterRoomRequested(self.room_name)

        # self._parent.onParentSizeChangedItem(self._parent.width(), self._parent.height())


    def refresh(self):

        self.LOG.debug('refresh()')
        
        self._parent.room_list_layout.onItemSizeChanged()


    def on_toggle_expand_requested(self, md):

        if self._expanded:

            self._expanded = False
            self.setItemHeight(self.BASE_HEIGHT)

            for thumb_entry in self.thumb_list:
                thumb_entry.hideItem()

            self.toggle_expand_button.showItem()
            self.toggle_collapse_button.hideItem()

            self.thumb_sa.hideItem()

            self.refresh()


        else:
            self._expanded = True
            self.setItemHeight(self.EXPAND_HEIGHT)

            self.toggle_expand_button.hideItem()
            self.toggle_collapse_button.showItem()
            
            self.thumb_sa.onParentSizeChanged(self.width() - 10,  self.height() - 50) # 80 # height)
            self.thumb_sa.showItem()

            if self.thumb_list:

                for thumb_entry in self.thumb_list:
                    thumb_entry.showItem()

            else:

                media_uuid_list = self._sanitize_uuid_list(self.room_media)
                # media_uuid_list = [ entry + '}' for entry in media_uuid_list ]
                print(media_uuid_list)

        
                def _load_thumb_data(thumb_data_result):

                    xpos = 5
                    for thumb_entry in thumb_data_result:
                        if not thumb_entry['thumb_frame_data']:
                            continue
                        
                        thumb = self._thumbf.fromBase64(
                            self.thumb_sa.content(),
                            thumb_entry['thumb_frame_data'],
                            110
                        )
                        thumb.setOutlined(False)
                        thumb.setToolTipText(
                            thumb_entry.get('title', '-')
                        )

                        self.thumb_layout.addItem(thumb, 0)

                        self.thumb_list.append(thumb)

                        xpos += thumb.width()


                    for thumb_entry in self.thumb_list:
                        thumb_entry.showItem()

                    self.refresh()


                self.EDBC.call(_load_thumb_data, 'edbc.find', 'Media',
                    [['uuid', 'in', media_uuid_list]],
                    ['title', 'thumb_frame_data']
                ).run()


            self.refresh()


    def boundingRect(self):
        return QtCore.QRectF(0, 0, self.width(), self.height())


    def paintItem(self, painter):
        
        p = QtGui.QPen()
        p.setColor(QtGui.QColor(0, 0, 0))

        b = QtGui.QBrush(QtGui.QColor(60, 60, 60))

        painter.setPen(p)
        painter.setBrush(b)

        painter.drawRect(self.boundingRect())



class RoomsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._destructed = False

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":feather/lightgrey/refresh-cw.svg",
            parent,
            20
        )
        self.refresh_button.setToolTipText("Refresh Room List")

        self.new_room_button = RlpGui.GUI_SvgButton(
            ":misc/home_new.svg",
            parent,
            23
        )
        self.new_room_button.icon().setPos(0, -2)
        self.new_room_button.setToolTipText("Create New Room")

        self.leave_button = RlpGui.GUI_SvgButton(
            ":feather/lightgrey/x-square.svg",
            parent,
            20
        )
        self.leave_button.setToolTipText("Leave Room")
        self.leave_button.icon().setFgColour(QtGui.QColor(250, 0, 0))

        self.show_remote_cursors_button = RlpGui.GUI_SvgButton(
            ':misc/cloud_cursor.svg', parent, 20
        )
        self.show_remote_cursors_button.buttonPressed.connect(
            self.on_show_remote_cursors_requested
        )

        self.curr_room_label = RlpGui.GUI_Label(self, "-")
        self.curr_room_label.setLeftSvgIconPath(
            ':feather/lightgrey/home.svg',
            20,
            0
        )

        f = self.curr_room_label.font()
        f.setBold(True)
        self.curr_room_label.setFont(f)

        self.curr_room_label.hideItem()

        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.addSpacer(10)
        self.toolbar_layout.addItem(self.refresh_button, 0)
        self.toolbar_layout.addSpacer(10)
        self.toolbar_layout.addItem(self.new_room_button, 0)
        self.toolbar_layout.addSpacer(10)
        self.toolbar_layout.addItem(self.show_remote_cursors_button, 0)
        self.toolbar_layout.addSpacer(10)

        self.toolbar_layout.addItem(self.leave_button, 0)

        self.status_text = RlpGui.GUI_Label(self, "Connecting..")

        self.toolbar_layout.addItem(self.status_text, 0)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.toolbar_layout, 0)

    
        self.roomlist_sa = RlpGui.GUI_ScrollArea(self)
        self.roomlist_sa.setPos(0, self.toolbar_layout.height() + 10)
        self.roomlist_sa.setBgColour(QtGui.QColor(40, 40, 40))
        self.roomlist_sa.setMaxChildHeight(600)
    
        self.room_list_layout = RlpGui.GUI_VLayout(self.roomlist_sa.content())

        self.room_controls = RoomControlsWidget(self)
        self.room_controls.setVisible(False)
        self.room_controls.setPos(0, self.toolbar_layout.height() + 10)

        # self.layout.addSpacer(10)
        # self.layout.addItem(self.roomlist_sa, 0)

        self.rooms = []

        # connections
        self.refresh_button.buttonPressed.connect(self.on_refresh_clicked)
        self.new_room_button.buttonPressed.connect(self.on_new_room_clicked)
        self.leave_button.buttonPressed.connect(self.on_leave_clicked)

        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.sendCallToClient.connect(self.on_data_received)

        self._init_rooms()

        self.onParentSizeChangedItem(parent.width(), parent.height())

        self.active = True

    @property
    def edbc(self):
        return RlpClient.instance()

    def destruct(self):
        self._destructed = True

    def onParentSizeChangedItem(self, width, height):

        self.setItemWidth(width)
        self.setItemHeight(height)

        sa_ypos = self.roomlist_sa.y()
        nheight = height - sa_ypos - 5

        self.roomlist_sa.onParentSizeChanged(width - 10, nheight)
        self.room_controls.setWidth(width)
        self.room_controls.setHeight(nheight)

        for room in self.rooms:
            room.onParentSizeChangedItem(width - 10, height)

        self.room_list_layout.onItemSizeChanged()

        self.curr_room_label.setPos(
            width - self.curr_room_label.width() - 5,
            6
        )


    def _init_rooms(self):

        if not self.edbc:
            self.status_text.setText("Error: Login required")
            return

        print('init rooms')
        self.status_text.setText("Getting rooms..")
        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.callFromClient(
            'rooms.get_rooms',
            [],
            {}
        )


    def on_refresh_clicked(self, md=None):
        print('refresh')
        self._init_rooms()



    def on_room_delete_requested(self, room_name):

        print('DELETE ROOM: {}'.format(room_name))

        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.callFromClient(
            'rooms.delete_room',
            [],
            {
                'room_name': room_name,
                'sync': True
            }
        )

        # floating_pane = rlp_gui.VIEW.createFloatingPane(200, 100, False)
        # diag = ConfirmDialog('Room "{}" Deleted'.format(room_name), floating_pane)
        # diag.sig_done.connect(self.on_refresh_clicked)

        # DIALOGS.append(diag)


    def on_room_rename_requested(self, room_name, room_uuid, new_room_name):

        print('RENAME ROOM {} -> {}'.format(room_name, new_room_name))

        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.callFromClient(
            'rooms.rename_room',
            [],
            {
                'room_name': room_name,
                'room_uuid': room_uuid,
                'new_room_name': new_room_name,
                'sync': True
            }
        )


    def on_new_room_clicked(self, md):

        print('NEW ROOM FROM SESSION')

        floating_pane = revlens.VIEW.createFloatingPane(400, 240, False)
        diag = NewRoomDialog(floating_pane)
        diag.sig_new_room.connect(self.on_create_room_requested)
    
        global DIALOGS
        DIALOGS.append(diag)


    def on_create_room_requested(self, room_info):

        print('ON CREATE ROOM REQUESTED!!')
        print(room_info)

        room_name = room_info['room_name']
        contents = room_info['room_type']

        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.callFromClient(
            'rooms.create_room',
            [],
            {
                'room_name': room_name,
                'contents': contents
            }
        )

        self.on_refresh_clicked()


    def on_leave_clicked(self, md):

        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.callFromClient(
            'rooms.leave_room',
            [],
            {}
        )


    def on_leave_room(self, data):

        # if this leave came from this client, run leave
        #
        from_session = data['kwargs']['kwargs']
        fromIdent = from_session['client.ident']
        myIdent = rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.clientIdent()

        if (fromIdent == myIdent):
            rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.leaveRoom()
            self.curr_room_label.setText('')
            self.curr_room_label.hideItem()

        else:
            print('Got leave_room from: {}'.format(from_session))
            from_user = from_session['client.user']
            rlplug_qtbuiltin.cmds.display_message(
                '{} left the room'.format(from_user))
            
        self.room_controls.setVisible(False)
        self.roomlist_sa.setVisible(True)

    def on_show_remote_cursors_requested(self, md):

        toggledVal = not md['toggled']

        self.show_remote_cursors_button.setToggled(toggledVal)
        self.show_remote_cursors_button.icon().setOutlined(toggledVal)

        import rlplug_annotate
        rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ.setPref("pref.show_remote_cursors", toggledVal)

        self.show_remote_cursors_button.icon().update()

    def on_data_received(self, data):

        if self._destructed:
            return

        if not self.active:
            return

        method_name = data.get('method')

        if method_name == 'on_get_rooms':
            
            self.status_text.setText('')
            self.room_list_layout.clear()
            self.rooms = []

            for room_info in data['kwargs']['result']:

                self.add_room(
                    room_info['room_name'],
                    room_info['room_uuid'],
                    room_info.get('room_media', []),
                    room_info['created_at_str']
                )

        elif method_name in ['on_delete_room', 'on_rename_room']:
            self._init_rooms()

        elif method_name == 'on_leave_room':
            self.on_leave_room(data)


        self.room_list_layout.onItemSizeChanged()


    def add_room(self, room_name, room_uuid, room_media_list, created_at_str):

        room_widget = RoomWidget(self,
            room_name, room_uuid, room_media_list, created_at_str)

        self.rooms.append(room_widget)
        self.room_list_layout.addItem(room_widget, 5)


    def enterRoomRequested(self, roomName):

        self.roomlist_sa.setVisible(False)
        self.room_controls.setVisible(True)
        self.room_controls.loadRoom(roomName)

        self.onParentSizeChangedItem(self.width(), self.height())

'''
class RoomsWebKitPanel(QtGui.QGraphicsObject):

    sizeChanged = QtCore.Signal(float, float)

    def __init__(self, parent):
        QtGui.QGraphicsObject.__init__(self, parent)

        self.layout = RlpGui.GUI_VLayout(self)

        self.inspector = RlpGui.GUI_IconButton("", self)
        self.inspector.setText("Open Inspector")
        self.inspector.setOutlined(True)
        self.inspector.buttonPressed.connect(self.on_inspector_clicked)

        from PythonQt.rlplug_sync import RLSYNC_WebKitView
        self.rooms = RLSYNC_WebKitView(
            rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ,
            self
        )

        self.layout.addItem(self.inspector)
        self.layout.addItem(self.rooms)

        site_url = rlp.util.APPGLOBALS.globals()['site_url']
        url = '{}/rlplug_sync/default/index'.format(site_url)
        print('Rooms URL: {}'.format(url))

        self.rooms.load(url)

        parent.sizeChanged.connect(self.onSizeChanged)



    def onSizeChanged(self, width, height):
        self.sizeChanged.emit(width, height)


    def boundingRect(self):
        return self.parentItem().boundingRect()

    def on_inspector_clicked(self):
        self.rooms.openInspector()

'''



def create(parent):
    
    global WIDGETS

    '''
    site_url = rlp_util.APPGLOBALS.globals()['site_url']
    site_url_info = urllib.parse.urlparse(site_url)
    
    ws_port = site_url_info.port - 1
    ws_host = site_url_info.hostname
    ws_host = ws_host.replace('localhost', '127.0.0.1')

    ws_url = 'ws://{}:{}/ws'.format(ws_host, ws_port)

    print('WebSocket URL: {}'.format(ws_url))

    revlens.CNTRL.getServerManager().setWebSocketUrl(ws_url)
    '''
    

    room_widget = RoomsPanel(parent)

    for w_entry in WIDGETS:
        w_entry.active = False


    WIDGETS.append(room_widget)

    return room_widget

    '''
    
    WIDGET = RLSYNC_WebKitView(
        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ,
        parent
    )

    WIDGET.load(url)
    '''
    