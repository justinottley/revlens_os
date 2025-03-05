
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

    def __init__(self, parent, category, username):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.category = category
        self.username = username

        self.toggled = False

        self.setWidth(self.SIZE)
        self.setHeight(self.SIZE)


    @property
    def cntrl(self):
        return rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ


    def mousePressEventItem(self, evt):
        self.toggled = not self.toggled

        self.cntrl.cmdFilter().setCategoryPermission(
            self.username, self.category, self.toggled)

        state = self.toMap()
        self.cntrl.onSyncAction("updatePermissions", state)
        revlens.CNTRL.emitNoticeEvent("permissions_changed", state)

        self.update()

    def toMap(self):
        return {
            'username': self.username,
            'category': self.category,
            'value': self.toggled
        }

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
        self.name_label.setText(self.name)
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

    def __init__(self, parent, username):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.username = username

        self.layout = RlpGui.GUI_HLayout(self)
        self.layout.setPos(5, 8)
        self.catButtons = {}
        for category in PERM_CATEGORY_MAP.values():
            pb = PermissionRadioButton(self, category, username)
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

        self.roomName = None

        self.roomCntrl = rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ

        self.rpane = RlpGui.GUI_Pane(self, "sync_controls", 2)
        self.rpane.split(False)
        self.rpane.setHeaderVisible(False, True)
        self.rpane.splitterPtr(0).setPosPercent(0.4, True)
        self.rpane.splitterPtr(0).setDrawLine(False)

        self.roomCntrl.sendCallToClient.connect(self.onMsgReceived)

        self.client_layout = RlpGui.GUI_VLayout(self.rpane.panePtr(0))
        self.perms_layout = RlpGui.GUI_VLayout(self.rpane.panePtr(1))

        self._clients = []
        self._perms = []


    def loadRoom(self, roomName):
        self.roomName = roomName

        self.client_layout.clear()
        self._clients = []

        self.perms_layout.clear()
        self._perms = {}

        loading = RlpGui.GUI_Label(self, 'Loading clients..')
        self.client_layout.addItem(loading, 0)
        self._clients.append(loading)

        self.roomCntrl.callFromClient(
            'rooms.get_clients',
            [],
            {'room_name': roomName}
        )


    def onMsgReceived(self, msgData):
        print('msg received:')
        pprint.pprint(msgData)
        if msgData['method'] == 'on_get_clients':

            self.client_layout.clear()
            self.client_layout.addSpacer(20)

            self._clients = []

            self.perms_layout.clear()
            ph = RlpGui.GUI_Label(self, 'V     P     S    A')
            self.perms_layout.addItem(ph, 8)
            self._perms = {
                '_header': ph
            }

            for clientName in msgData['kwargs']['result']:
                client = RoomClientName(self.rpane.panePtr(0), clientName)
                self.client_layout.addItem(client, 5)
                self._clients.append(client)

                perms = RoomClientPermissions(self.rpane.panePtr(1), clientName)
                self.perms_layout.addItem(perms, 5)
                self._perms[clientName] = perms

        elif msgData['method'] == 'updatePermissions':

            kwargs = msgData['kwargs']
            username = kwargs['username']
            category = kwargs['category']
            self.roomCntrl.cmdFilter().setCategoryPermission(
                username, category, kwargs['value'])
            
            if username in self._perms:
                perms = self._perms[username]
                perms.catButtons[category].toggled = kwargs['value']
                perms.catButtons[category].update()

    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)