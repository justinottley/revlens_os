
import uuid

from rlp.Qt import QStr
from rlp import QtCore, QtGui
import rlp.core as RlpCore
import rlp.gui as RlpGui

from .plug import Plug

nodebg = QtGui.QColor(100, 100, 100)

DEFAULT_SCALE = 2


class SelectedNodeBackground(RlpGui.GUI_PaintedItemBase):

    def __init__(self, parent):
        RlpGui.GUI_PaintedItemBase.__init__(self, parent)
        
        self.setWidth(216)
        self.setHeight(116)

    def paintItem(self, painter):

        # painter.setAntiAliasing(True)

        p  = QtGui.QPen(QtGui.QColor(60, 60, 150))
        p.setWidth(20)
        painter.setPen(p)
        painter.setBrush(QtGui.QBrush(QtGui.QColor(60, 60, 150)))
        painter.drawRoundedRect(
            QtCore.QRectF(12, 12, self.width() - 24, self.height() - 24), 12, 12)


class NodeButton(RlpGui.GUI_PaintedItemBase):

    WIDTH = 40

    def __init__(self, node):
        RlpGui.GUI_PaintedItemBase.__init__(self, node)

        self.setItemAcceptHoverEvents(True)

        self.pressed = QtCore.PySignal()

        self.__node = node
        self._inHover = False
        self._inNodeHover = False
        self._display = False

        self._penBlack = QtGui.QPen(QtGui.QColor(10, 10, 10))
        self._penBlack.setWidth(4)

        self._penWhite = QtGui.QPen(QtGui.QColor(200, 200, 200))
        self._penWhite.setWidth(4)

        self._brush = QtGui.QBrush(QtGui.QColor(54, 70, 108))

        self.setZValue(6)
        self.setWidth(self.WIDTH)
        self.setHeight(self.node.height() - 15)

    @property
    def node(self):
        return self.__node

    def setDisplay(self, isDisplay):
        self._display = isDisplay
        self.updateItem()

    def setInNodeHover(self, inNodeHover):
        self._inNodeHover = inNodeHover
        self.updateItem()

    def hoverEnterEventItem(self, evt):
        self._inHover = True
        self.updateItem()

    def hoverLeaveEventItem(self, evt):
        self._inHover = False
        self.updateItem()

    def mousePressEventItem(self, evt):
        self._display = True
        self.pressed.emit({})
        self.updateItem()

    def rrect(self):
        return QtCore.QRectF(5, 5, self.width() - 10, self.height() - 35)


    def paintItem(self, painter):

        if not self._display and not self._inNodeHover:
            return

        if self._inHover:
            painter.setPen(self._penWhite)

        else:
            painter.setPen(self._penBlack)

        if self._display:
            painter.setBrush(self._brush)

        painter.drawRoundedRect(self.rrect(), 5, 5)



class Node(RlpGui.GUI_PaintedItemBase):

    _utext = RlpCore.UTIL_Text()

    NODEWIDTH = 200

    def __init__(self, scene, name='', node_id=None, payload=None):
        RlpGui.GUI_PaintedItemBase.__init__(self, scene.view)

        # self.setPaintMode(0)

        self.scene = scene

        self.moved = QtCore.PySignal()
        self.selected = QtCore.PySignal()
        self.display = QtCore.PySignal()

        self.setItemAcceptHoverEvents(True)

        self.setZValue(5)
        self.setWidth(350)
        self.setHeight(100)

        self.__name = name
        self.node_id = node_id
        if node_id is None:
            self.node_id = str(uuid.uuid4())

        # self.name_label = RlpGui.GUI_Label(self, self.__name)
        # self.name_label.setPaintMode(0)
        # self.name_label.setPos(200, 25)
        # nf = self.name_label.font()
        # nf.setPixelSize(nf.pixelSize() * DEFAULT_SCALE)
        # self.name_label.setFont(nf)
        # self.name_label.setYOffset(20)
        # self.name_label.setHeight(self.name_label.height() * 2)

        self._font = self._utext.currFont()
        self._font.setPixelSize(30)
        self._brush = QtGui.QBrush(nodebg)

        self._pressPos = None
        self._selected = False
        self._display = False
        self._in_hover = False

        self.in_plugs = []
        self.out_plugs = []

        self.displayButton = NodeButton(self)
        self.displayButton.setPos(140, 22)
        self.displayButton.pressed.connect(self.onDisplayPressed)


        self.scene.register_node(self)

        payload = payload or {}
        self.node_init(payload)


    @property
    def node_type(self):
        return self.__class__.__name__.replace('Node', '')

    def destruct(self):
        print('NODE DESTRUCTED????')


    def cook(self, stage):
        return stage


    @property
    def name(self):
        return self.__name

    def node_init(self, payload):
        if 'x' in payload and 'y' in payload:
            self.setPos(payload['x'], payload['y'])



    def set_name(self, name):
        self.__name = name

        # self.name_label.setText(name)
        # self.name_label.setHeight(self.name_label.height() * 2)


    def add_plug(self, name, plug_type):
        plug = Plug(self, name, plug_type)

        if plug_type == Plug.TYPE_IN:
            plug_list = self.in_plugs
            plug_y = -5
        
        elif plug_type == Plug.TYPE_OUT:
            plug_list = self.out_plugs
            plug_y = self.height() - 25

        plug_list.append(plug)
        plug_xoffset = (self.NODEWIDTH / (len(plug_list) + 1))

        plug_idx = 0
        for iplug in plug_list:
            plug_xpos = plug_xoffset * (plug_idx + 1)
            iplug.setPos(plug_xpos - 15, plug_y)
            plug_idx += 1


    def get_plugs(self, plug_type):
        if plug_type == Plug.TYPE_IN:
            return self.in_plugs

        elif plug_type == Plug.TYPE_OUT:
            return self.out_plugs

    def getPlug(self, plugType, name):
        for plug in self.get_plugs(plugType):
            if plug.name == name:
                return plug

    def plug_connected(self, plug, conn):
        pass

    def plug_disconnected(self, plug, conn):
        pass


    def onDisplayButtonHover(self, inHover):
        self.hoverEnterEventItem(None)


    def onDisplayPressed(self, md):
        self.scene.setDisplayNode(self)

    def setDisplay(self, isDisplay):
        self._display = isDisplay
        self.displayButton.setDisplay(isDisplay)

    def setSelected(self, isSelected):
        self._selected = isSelected


    def hoverEnterEventItem(self, event):
        self._in_hover = True
        for plug_list in [self.in_plugs, self.out_plugs]:
            for plug in plug_list:
                plug.set_node_hover(True)

        self.displayButton.setInNodeHover(True)
        self.updateItem()


    def hoverLeaveEventItem(self, event):
        self._in_hover = False
        for plug_list in [self.in_plugs, self.out_plugs]:
            for plug in plug_list:
                plug.set_node_hover(False)

        self.displayButton.setInNodeHover(False)
        self.updateItem()


    def mousePressEventItem(self, event):
        
        print('selected')

        self._pressPosG = event.globalPosition()
        self._currX = self.x()
        self._currY = self.y()
        self._selected = True

        self.selected.emit({'node_id': self.node_id, 'node': self})


    def mouseMoveEventItem(self, event):

        if not self._selected:
            return

        egpos = event.globalPosition()
        diff_x = (egpos.x() - self._pressPosG.x()) / self.parentItem().scale()
        diff_y = (egpos.y() - self._pressPosG.y()) / self.parentItem().scale()

        self.setPos(self._currX + diff_x, self._currY + diff_y)

        if self._selected:
            mp = self.pos()
            self.scene.view.selNodeBg.setPos(mp.x() - 8, mp.y() - 8)

        self.moved.emit({})


    def nodeRect(self):
        return QtCore.QRectF(
            10, 10, self.NODEWIDTH - 20, self.height() - 20)


    def paintItem(self, painter):

        # print('NODE PAINT ITEM')
        painter.setAntiAliasing(True)
        # white = QtGui.QColor(200, 200, 200)

        col = QtGui.QColor(10, 10, 10)

        if self._selected:
            col = QtGui.QColor(80, 150, 80)
            self.scene.view.selNodeBg.showItem()
            mp = self.pos()
            self.scene.view.selNodeBg.setPos(mp.x() - 8, mp.y() - 8)


        if self._in_hover:
            col = QtGui.QColor(200, 200, 200)
            # self.displayToggleButton.showItem()

        else:
            pass
            # self.displayToggleButton.hideItem()

        pm = QtGui.QPen(col)

        pm.setWidth(5)
        painter.setPen(pm)


        painter.setBrush(self._brush)
        painter.drawRoundedRect(self.nodeRect(), 20, 20)

        # painter.setPen(QtGui.QColor(200, 200, 200))

        rp = QtGui.QPen(QtGui.QColor(200, 200, 200))
        painter.setPen(rp)
        painter.setFont(self._font)
        painter.drawText(210, 60, QStr(self.name))

    def to_json(self):
        return {
            'name': self.__name,
            'type': self.__class__.__name__.replace('Node', ''),
            'x': self.x(),
            'y': self.y()
        }