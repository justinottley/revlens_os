
import pprint

from rlp import QtGui
import rlp.gui as RlpGui

import revlens
import rlplug_sync

PERM_CATEGORY_MAP = {
    'V': 'viewport',
    'P': 'playback',
    'S': 'session',
    'A': 'annotate'
}

class PermissionRadioButton(RlpGui.GUI_ItemBase):

    SIZE = 16

    def __init__(self, parent, category, userident):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.category = category
        self.userident = userident

        self.toggled = False

        self.setToolTipText(category)

        self.setWidth(self.SIZE)
        self.setHeight(self.SIZE)


    @property
    def cntrl(self):
        return rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ


    def to_map(self):
        return {
            'user_ident': self.userident,
            'category': self.category,
            'value': self.toggled
        }


    def mousePressEventItem(self, evt):
        self.toggled = not self.toggled

        self.cntrl.cmdFilter().setCategoryPermission(
            self.userident, self.category, self.toggled)

        state = self.to_map()
        self.cntrl.onSyncAction("updatePermissions", state)
        revlens.CNTRL.emitNoticeEvent("permissions_changed", state)

        self.update()


    def paintItem(self, painter):

        # p1 = QtGui.QPen(QtGui.QColor(180, 0, 0))
        # painter.setPen(p1)
        # painter.drawRect(self.boundingRect())

        p = QtGui.QPen(QtGui.QColor(10, 10, 10))
        p.setWidth(2)

        b = QtGui.QBrush(QtGui.QColor(200, 200, 200))

        painter.setPen(p)
        painter.setBrush(b)
        hs = self.SIZE  / 2
        painter.drawCircle(hs, hs, hs - 1)

        if self.toggled:
            b = QtGui.QBrush(QtGui.QColor(10, 10, 10))
            painter.setBrush(b)
            painter.drawCircle(hs, hs, hs - 4)


class RoomClientName(RlpGui.GUI_ItemBase):

    HEIGHT = 30

    def __init__(self, parent, name):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.name = name

        self.name_label = RlpGui.GUI_Label(self, '')
        nlf = self.name_label.font()
        # nlf.setPixelSize(nlf.pixelSize() + 1)
        nlf.setBold(True)
        self.name_label.setFont(nlf)
        self.name_label.setText(self.name.split(':')[0])
        self.name_label.setPos(10, 5)

        self._selected = False

        self.setWidth(parent.width() - 10)
        self.setHeight(self.HEIGHT)

    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width - 10)

    def paintItem(self, painter):

        p = QtGui.QPen()
        
        selCol = QtGui.QColor(0, 0, 0)
        if self._selected:
            selCol = QtGui.QColor(200, 200, 200)
            p.setWidth(4)

        else:
            p.setWidth(1)

        p.setColor(selCol)

        b = QtGui.QBrush(QtGui.QColor(60, 60, 60))

        painter.setPen(p)
        painter.setBrush(b)

        painter.drawRect(self.boundingRect())


class RoomClientPermissions(RlpGui.GUI_ItemBase):

    HEIGHT = 30

    def __init__(self, parent, userident):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.userident = userident

        self.layout = RlpGui.GUI_HLayout(self)
        self.layout.setPos(5, 8)
        self.catButtons = {}
        for category in PERM_CATEGORY_MAP.values():
            pb = PermissionRadioButton(self, category, userident)
            self.layout.addItem(pb, 0)
            self.layout.addSpacer(10)
            self.catButtons[category] = pb

        self.setWidth(parent.width() - 10)
        self.setHeight(self.HEIGHT)

    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width - 10)



class RoomControlsWidget(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.room_uuid = None
        self.room_name = None

        self.room_cntrl = rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ

        self.rpane = RlpGui.GUI_Pane(self, "sync_controls", 2)
        self.rpane.split(False)
        self.rpane.setHeaderVisible(False, True)
        self.rpane.splitterPtr(0).setPosPercent(0.4, True)
        self.rpane.splitterPtr(0).setDrawLine(False)

        self.room_cntrl.sendCallToClient.connect(self.on_data_received)

        self.client_layout = RlpGui.GUI_VLayout(self.rpane.panePtr(0))
        self.perms_layout = RlpGui.GUI_VLayout(self.rpane.panePtr(1))

        self._clients = []
        self._perms = []


    def load_room(self, room_uuid, room_name):

        self.room_uuid = room_uuid
        self.room_name = room_name

        self.client_layout.clear()
        self._clients = []

        self.perms_layout.clear()
        self._perms = {}

        loading = RlpGui.GUI_Label(self, 'Loading clients..')
        self.client_layout.addItem(loading, 0)
        self._clients.append(loading)

        # self.room_cntrl.cmdFilter().setDriver('jottley', True)
        # self.room_cntrl.cmdFilter().setDriver('mobile', True)


    def add_user_client(self, client_info, show_msg=False):

        client_user_ident = '{}{}'.format(
            client_info['client.user'],
            client_info['client.ident']
        )

        if client_user_ident in self._perms:
            print('WARNING: client {} already present'.format(client_user_ident))
            return

        client = RoomClientName(self.rpane.panePtr(0), client_user_ident)
        self.client_layout.addItem(client, 5)
        self._clients.append(client)

        perms = RoomClientPermissions(self.rpane.panePtr(1), client_user_ident)
        self.perms_layout.addItem(perms, 5)
        self._perms[client_user_ident] = perms

        perm_info = client_info.get('permissions', {})

        self.update_client_permissions(client_user_ident, perm_info)


        if show_msg:
            msg = 'User {} joined room {}'.format(client_info['client.user'], self.room_name)
            self.room_cntrl.emitEventMessage(msg, 800)


    def remove_user_client(self, client_info):

        client_user_ident = '{}{}'.format(
            client_info['client.user'],
            client_info['client.ident']
        )

        remaining_clients = []
        for client in self._clients:
            if client.name == client_user_ident:
                
                if not hasattr(client, 'name'): # Loading text....
                    continue

                print('Removing {} {}'.format(client.name, client))
    
                msg = 'User {} exited room {}'.format(client_info['client.user'], self.room_name)
                self.room_cntrl.emitEventMessage(msg, 800)

                self.client_layout.removeItem(client)

            else:
                remaining_clients.append(client)


        self._clients = remaining_clients

        if client_user_ident in self._perms:
            perm_widget = self._perms[client_user_ident]
            self.perms_layout.removeItem(perm_widget)
            del self._perms[client_user_ident]


    def update_client_permissions(self, client_user_ident, perm_info):

        print('upddate_client_permissions {} {}'.format(client_user_ident, perm_info))

        client_perms = self._perms[client_user_ident]
        for perm_entry, perm_value in perm_info.items():

            self.room_cntrl.cmdFilter().setCategoryPermission(
                self.room_cntrl.currentUserIdent(), perm_entry, perm_value
            )

            client_perms.catButtons[perm_entry].toggled = perm_value
            client_perms.catButtons[perm_entry].update()


    def load_all_clients(self, client_info_list):

        self.client_layout.clear()
        self.client_layout.addSpacer(20)

        self._clients = []

        self.perms_layout.clear()
        ph = RlpGui.GUI_Label(self, 'V     P     S    A')
        ph.setToolTipText(
            'V: {} P: {} S: {} A: {}'.format(
                PERM_CATEGORY_MAP['V'],
                PERM_CATEGORY_MAP['P'],
                PERM_CATEGORY_MAP['S'],
                PERM_CATEGORY_MAP['A']
            )
        )

        self.perms_layout.addItem(ph, 8)
        self._perms = {
            '_header': ph
        }

        for client_info in client_info_list:
            self.add_user_client(client_info)


    def on_data_received(self, msg_data):
        # print('msg received:')
        # pprint.pprint(msg_data)

        method_name = msg_data.get('method')

        if method_name == 'on_room_loaded':
            
            self.room_cntrl.callFromClient(
                'rooms.get_clients',
                [],
                {'room_uuid': msg_data['kwargs']['room_uuid']}
            )


        elif method_name == 'join_room':

            client_info = msg_data['kwargs']
            client_user_ident = '{}{}'.format(
                client_info['client.user'],
                client_info['client.ident']
            )

            if client_user_ident not in self._perms:
                self.add_user_client(client_info, show_msg=True)


        elif method_name == 'client_leave':

            client_info = msg_data['kwargs']
            room_uuid = client_info['broadcast.group_name']
            if room_uuid == self.room_uuid:
                self.remove_user_client(client_info)


        elif  method_name == 'on_get_clients':
            self.load_all_clients(msg_data['kwargs']['result'])


        elif method_name == 'updatePermissions':

            kwargs = msg_data['kwargs']
            userident = kwargs['user_ident']
            category = kwargs['category']
            self.room_cntrl.cmdFilter().setCategoryPermission(
                userident, category, kwargs['value'])

            if userident in self._perms:
                perms = self._perms[userident]
                perms.catButtons[category].toggled = kwargs['value']
                perms.catButtons[category].update()


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)