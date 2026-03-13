
import os

from rlp import QtCore, QtGui
import rlp.gui as RlpGui
import rlp.util as RlpUtil

import revlens
import revlens.gui as RevlensGui


class FullSizeView(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._sizeToFit = False # True

        self.__oimage = None
        self.__image = None

        self._initPosX = 0
        self._initPosY = 0
        self._currPosX = 0
        self._currPosY = 0
        self._pressPos = None

    def setSizeToFit(self, val):
        self._sizeToFit = val
        self._currPosX = 0
        self._currPosY = 0
        self._initPosX = 0
        self._initPosY = 0
        self.setImage(self.__oimage)


    def setImage(self, image):

        self.__oimage = image

        if self._sizeToFit:
            pimage = image.scaledToHeight(int(self.height()))

            if (pimage.width() > self.width()):
                pimage = pimage.scaledToWidth(int(self.width()))
        else:
            pimage = self.__oimage.copy()

        self.__image = pimage
        self.update()


    def mousePressEventItem(self, event):
        self._pressPos = event.position()


    def mouseMoveEventItem(self, event):

        newPos = event.position()
        self._currPosX = self._initPosX + int(newPos.x() - self._pressPos.x())
        self._currPosY = self._initPosY + int(newPos.y() - self._pressPos.y())
        self.update()


    def mouseReleaseEventItem(self, event):
        self._initPosX = self._currPosX
        self._initPosY = self._currPosY


    def onParentSizeChangedItem(self, width, height):
        if (self.__oimage is not None):
            self.setImage(self.__oimage)


    def paintItem(self, painter):

        if self.__image is not None:
            # painter.setPen(QtGui.QPen(QtGui.QColor(200, 200, 200)))
            # painter.drawRect(self.boundingRect())
            painter.drawImage(self._currPosX, self._currPosY, self.__image)


class SnapshotThumbnail(RlpGui.GUI_ItemBase):

    def __init__(self, parent, image):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.setItemAcceptsHoverEvents(True)

        self._inHover = False

        self.buttonPressed = QtCore.PY_Signal(self)

        self.fimage = image
        self.image = image.scaledToWidth(230)

        self.setWidth(self.image.width() + 6)
        self.setHeight(self.image.height() + 6)


    def hoverEnterEventItem(self, event):
        self._inHover = True
        self.update()


    def hoverLeaveEventItem(self, event):
        self._inHover = False
        self.update()


    def mousePressEventItem(self, event):
        self.buttonPressed.emit({'image': self.image, 'fimage': self.fimage})


    def paintItem(self, painter):

        col = QtGui.QColor(10, 10, 10)
        if self._inHover:
            col = QtGui.QColor(180, 180, 180)

        painter.setPen(QtGui.QPen(col))
        painter.drawRect(self.boundingRect())
        painter.drawImage(3, 3, self.image)


class StreamingSnapshotPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.thumbSelected = QtCore.PY_Signal(self)

        self.preview = FullSizeView(self)
        self.flayout = RlpGui.GUI_FlowLayout(self)

        self.thumbs = [] # needed for reference counting... ASDLFKJSFJ

        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onNoticeEvent(self, evtName, evtInfo):

        print('{} {}'.format(evtName, evtInfo))

        if evtName != 'stream.snapshot_ready':
            return


        print('GOT SNAPSHOT')
        # imagePath = evtInfo['image_path']

        # image = QtGui.QImage(QtCore.QString(imagePath))
        image = evtInfo['image']
        thumb = SnapshotThumbnail(self, image)
        thumb.buttonPressed.connect(self.onThumbClicked)

        self.thumbs.append(thumb)

        self.flayout.addItem(thumb)

        self.onParentSizeChangedItem(self.width(), self.height())


    def onThumbClicked(self, md):

        print(md)
        self.preview.setImage(md['fimage'])
        self.thumbSelected.emit(md)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width - 20)
        self.setHeight(height)

        halfHeight = height / 2
        halfWidth = width / 2

        self.flayout.setWidth(halfWidth)

        self.preview.setHeight(height)
        self.preview.setWidth(width)
        self.preview.setPos(halfWidth, 0)


def create(parent):
    return StreamingSnapshotPanel(parent)