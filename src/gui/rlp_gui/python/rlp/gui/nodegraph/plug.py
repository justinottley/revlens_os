
from rlp import QtCore, QtGui
import rlp.gui as RlpGui

nodebg = QtGui.QColor(100, 100, 100)

class Plug(RlpGui.GUI_PaintedItemBase):

    TYPE_IN = 0
    TYPE_OUT = 1

    RADIUS = 15 # 20

    def __init__(self, node, name, plug_type):
        RlpGui.GUI_PaintedItemBase.__init__(self, node)

        self.press = QtCore.PySignal()
        self.press_end = QtCore.PySignal()

        self.setItemAcceptHoverEvents(True)

        self.setZValue(6)
        self.setWidth(Plug.RADIUS * 2)
        self.setHeight(Plug.RADIUS * 2)

        self.__node = node
        self.__name = name
        self.__plug_type = plug_type

        self.__connections = []

        self._brush = QtGui.QBrush(nodebg)
        self._in_hover = False
        self._in_node_hover = False

        self.press.connect(self.view.onConnectionStart)
        self.press_end.connect(self.view.onConnectionEnd)



    @property
    def node(self):
        return self.__node

    @property
    def scene(self):
        return self.node.scene

    @property
    def view(self):
        return self.scene.view


    @property
    def plug_type(self):
        return self.__plug_type

    @property
    def name(self):
        return self.__name

    @property
    def connections(self):
        return self.__connections

    def add_connection(self, conn):
        self.__connections.append(conn)
        self.node.plug_connected(self, conn)

    def remove_connection(self, conn):
        if conn in self.__connections:
            self.__connections.remove(conn)
            self.node.plug_disconnected(self, conn)


    def set_node_hover(self, in_node_hover):
        self._in_node_hover = in_node_hover
        self.update()

    def set_hover(self, in_hover):
        self._in_hover = in_hover
        self.update()


    def hoverEnterEventItem(self, event):
        self._in_hover = True
        self.update()

    def hoverLeaveEventItem(self, event):
        self._in_hover = False
        self.update()

    def mousePressEventItem(self, event):
        if self._in_hover:
            self.press.emit({'plug': self, 'evt': event})

    def mouseMoveEventItem(self, event):
        if self._in_hover:
            self.press.emit({'plug': self, 'evt': event})

    def mouseReleaseEventItem(self, event):
        print('MOUSE RELEASE')
        self.press_end.emit({'plug': self, 'evt': event})


    def paintItem(self, painter):

        painter.setAntiAliasing(True)

        col = QtGui.QColor(10, 10, 10)

        if self._in_hover:
            col = QtGui.QColor(200, 100, 100)

        elif self._in_node_hover:
            col = QtGui.QColor(200, 200, 200)
        
        p = QtGui.QPen(col)
        p.setWidth(5)

        painter.setPen(p)
        painter.setBrush(self._brush)

        painter.drawEllipse(5, 5, int(self.width() - 10), int(self.height() - 10))

        # painter.drawCircle(15, 15, 10) # llipse(0, 0, 2, 2) # int(self.width() - 10), int(self.height() - 10))


    def toJson(self):
        return {
            'node_id': self.node.node_id,
            'name': self.name,
            'plug_type': self.plug_type
        }