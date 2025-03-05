
from rlp import QtCore, QtGui
import rlp.gui as RlpGui

from .plug import Plug
from .connection import Connection
from .node import SelectedNodeBackground

class MousePlug:

    class _MouseNode:

        def __init__(self, mouse_plug):
            self.mouse_plug = mouse_plug
            self.moved = QtCore.PySignal()

        def x(self):
            return -Plug.RADIUS
    
        def y(self):
            return -Plug.RADIUS

        @property
        def name(self):
            return '_MouseNode'


    def __init__(self):
        self.node = MousePlug._MouseNode(self)
        self.pos_x = 0
        self.pos_y = 0


    def x(self):
        return self.pos_x
    
    def y(self):
        return self.pos_y

    def width(self):
        return (Plug.RADIUS * 2)

    def height(self):
        return (Plug.RADIUS * 2)

    def add_connection(self, conn):
        pass


    def remove_connection(self, conn):
        pass


class NodeGraphView(RlpGui.GUI_ItemBase):

    _VIEWS = {}

    def __init__(self, parent, viewId=None):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        # self.setPaintMode(0)
        self.connectionCreated = QtCore.PySignal()

        self._ip_connection = None # in progress connection - click and drag
        self._ip_plug = None
        self.mouse_plug = MousePlug()
        
        if viewId is None:
            if self.__class__._VIEWS:
                viewId = max(self.__class__._VIEWS.keys()) + 1
            else:
                viewId = 1
                
        else:
            if viewId in self.__class__._VIEWS:
                raise Exception('View ID {} already issued: {}'.format(viewId))

        self.__class__._VIEWS[viewId] = self
        self.__viewId = viewId

        self._pos_x = 0
        self._pos_y = 0
        self._press_x = 0
        self._press_y = 0
        self._press_scale = 0.5

        self.selNodeBg = SelectedNodeBackground(self)
        self.selNodeBg.setPos(30, 30)
        self.selNodeBg.hideItem()


    @classmethod
    def getViewById(cls, viewId):
        return cls._VIEWS.get(viewId)

    @classmethod
    def views(cls):
        return cls._VIEWS

    @property
    def viewId(self):
        return self.__viewId

    def onConnectionStart(self, md):
        '''
        A new in-progress connection
        '''

        plug_source = md['plug']
        scene = plug_source.node.scene
        if self._ip_connection is None:
            self._ip_connection = Connection(scene, plug_source, self.mouse_plug)

        else:
            self._ip_connection.plug_source = plug_source
            self._ip_connection.showItem()
    
        evt = md['evt']
        evt_pos = evt.position()
        mapped_pos = plug_source.mapToItem(self, evt_pos)

        self.mouse_plug.pos_x = mapped_pos.x()
        self.mouse_plug.pos_y = mapped_pos.y()
        self._ip_connection.update()

        children = self.childItems()

        if self._ip_plug:
            self._ip_plug.set_hover(False)

        self._ip_plug = None

        dest_plug_type = Plug.TYPE_IN
        if plug_source.plug_type == Plug.TYPE_IN:
            dest_plug_type = Plug.TYPE_OUT
    
        for child in children:
            if not child.__class__.__name__.endswith('Node'):
                continue

            if child == plug_source.node:
                continue

            child_pos = self.mapToItem(child, mapped_pos)
            # print(child_pos)
            child_contains = child.containsItem(child_pos)
            # print('{} {}'.format(child_contains))

            if child_contains:
                for dest_plug in child.get_plugs(dest_plug_type):
                    plug_pos = child.mapToItem(dest_plug, child_pos)
                    if dest_plug.containsItem(plug_pos):
                        dest_plug.set_hover(True)
                        self._ip_plug = dest_plug

                    else:
                        dest_plug.set_hover(False)

    def onConnectionEnd(self, md):

        released_plug = md['plug']
        scene = released_plug.node.scene

        if self._ip_plug:
            if self._ip_plug.node != released_plug.node:
                print('Baking connection')
                source_plug = self._ip_plug
                dest_plug = released_plug

                if source_plug.plug_type == Plug.TYPE_OUT:
                    source_plug = released_plug
                    dest_plug = self._ip_plug

                conn = Connection(scene, source_plug, dest_plug)
                self.connectionCreated.emit({'conn': conn})

        if self._ip_connection:
            self._ip_connection.hideItem()

        # print('children: {}'.format(self.childAt(mapped_pos.x() + 2, mapped_pos.y() + 2)))


    def mousePressEventItem(self, event):

        pos = event.globalPosition()
        print('PRESS {}'.format(pos))
        self._press_x = pos.x()
        self._press_y = pos.y()
        self._pos_x = self.pos().x()
        self._pos_y = self.pos().y()
        self._press_scale = self.scale()

    def mouseMoveEventItem(self, event):

        mods = event.modifiers()
        if (mods not in [QtCore.KEY_CNTRL, QtCore.KEY_CNTRL_SHIFT]):
            return

        pos = event.globalPosition()

        xdiff = pos.x() - self._press_x
        ydiff = pos.y() - self._press_y
        mscale = 0.75
        if (mods == QtCore.KEY_CNTRL):
            self.setPos(self._pos_x + (xdiff * mscale), self._pos_y + (ydiff * mscale))

        if (mods == QtCore.KEY_CNTRL_SHIFT):
            nscale = self._press_scale + (xdiff * 0.005)
            if nscale > 0:
                self.setScale(nscale)
            # print('SCALE!!! {} -> {}'.format(scale, nscale))
