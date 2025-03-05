
from rlp import QtGui
import rlp.gui as RlpGui

from .plug import Plug

class Connection(RlpGui.GUI_PaintedItemBase):

    def __init__(self, ngscene, plug_source, plug_dest):
        RlpGui.GUI_PaintedItemBase.__init__(self, ngscene)

        self.setItemAcceptHoverEvents(True)

        self.ngscene = ngscene

        self.plug_source = plug_source
        self.plug_dest = plug_dest

        self.plug_source.add_connection(self)
        self.plug_dest.add_connection(self)

        self.setWidth(ngscene.width())
        self.setHeight(ngscene.height())

        self.plug_source.node.moved.connect(self.on_node_moved)
        self.plug_dest.node.moved.connect(self.on_node_moved)


    def on_node_moved(self, md):
        self.update()


    def mousePressEventItem(self, event):
        self.ngscene.mousePressEventItem(event)

    def mouseMoveEventItem(self, event):
        self.ngscene.mouseMoveEventItem(event)


    def onParentSizeChangedItem(self, width, height):
        # print('connection parent changed: {} {}'.format(width, height))
        self.setWidth(width)
        self.setHeight(height)
        pass

    def shape(self, painter=None):

        src_pos_x = self.plug_source.x() + (self.plug_source.width() / 2) + self.plug_source.node.x() # self.mapToItem(self.ngscene, src_pos)
        src_pos_y = self.plug_source.y() + (self.plug_source.height() / 2) + self.plug_source.node.y() # self.mapToItem(self.ngscene, src_pos)

        dest_pos_x = self.plug_dest.x() + (self.plug_dest.width() / 2) + self.plug_dest.node.x()
        dest_pos_y = self.plug_dest.y() + (self.plug_dest.height() / 2) + self.plug_dest.node.y()

        # src_pos = self.src.plug_pos # self.src.scenePos()
        # dest_pos = self.dest.plug_pos


        '''
        # For plugs to the side of the node
        
        fx = (((dest_pos.x() - src_pos.x()) / 3) * 2) + src_pos.x()
        fy = src_pos.y()
        sx = dest_pos.x() - (((dest_pos.x() - src_pos.x()) / 3) * 2)
        sy = dest_pos.y()
        '''

        # For plugs at top and bottom of the node
        fx = src_pos_x
        fy = (((dest_pos_y - src_pos_y) / 3) * 2) + src_pos_y
        sx = dest_pos_x
        sy = dest_pos_y - (((dest_pos_y - src_pos_y) / 3) * 2)


        if painter:

            painter.beginPath()
            painter.moveTo(src_pos_x, src_pos_y)
            painter.cubicTo(fx, fy, sx, sy, dest_pos_x, dest_pos_y)
            painter.stroke()
            painter.closePath()
        
        else:
            path = QtGui.QPainterPath()
            path.moveTo(src_pos_x, src_pos_y)
            path.cubicTo(fx, fy, sx, sy, dest_pos_x, dest_pos_y)
            return path



    def paintItem(self, painter):

        # painter.setAntiAliasing(True)

        pen = QtGui.QPen(QtGui.QColor(255, 255, 255))
        pen.setWidth(3)
        painter.setPen(pen)

        # painter.drawRect(self.boundingRect())

        # src_spos_x = self.plug_source.x() + self.plug_source.node.x() # self.mapToItem(self.ngscene, src_pos)
        # src_spos_y = self.plug_source.y() + self.plug_source.node.y() # self.mapToItem(self.ngscene, src_pos)

        # dest_spos_x = self.plug_dest.x() + self.plug_dest.node.x()
        # dest_spos_y = self.plug_dest.y() + self.plug_dest.node.y()

        # painter.drawLine(
        #     int(src_spos_x + Plug.RADIUS),
        #     int(src_spos_y + Plug.RADIUS),
        #     int(dest_spos_x + Plug.RADIUS),
        #     int(dest_spos_y + Plug.RADIUS)
        # )

        painter.strokePath(self.shape(), pen)
        # self.shape(painter)



    def toJson(self):
        return {
            'source': self.plug_source.toJson(),
            'dest': self.plug_dest.toJson()
        }