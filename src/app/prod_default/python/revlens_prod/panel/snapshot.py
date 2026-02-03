
import os
import shutil
import tempfile

from rlp import QtCore, QtGui
from rlp.Qt import QStr

import rlp.gui as RlpGui
import rlp.util as RlpUtil
import rlp.core.util as rlp_core_util

from rlp.gui.dialogs import InputDialog

import revlens
import revlens.cmds
import revlens.gui as RevlensGui

import rlplug_annotate


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

    def __init__(self, parent, image, frame, name):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.setItemAcceptHoverEvents(True)

        self.requestRemove = QtCore.PY_Signal(self)

        self._inHover = False

        self.buttonPressed = QtCore.PY_Signal(self)

        self._name = name
        self.name = QStr(name)

        self.fimage = image
        self.image = image.scaledToWidth(230)

        self._iw = self.image.width()
        self._ih = self.image.height()

        self.edit_button = RlpGui.GUI_SvgButton(
            ':feather/edit-2.svg',
            self, 17
        )
        self.edit_button.setPos(8, 3)
        self.edit_button.buttonPressed.connect(self.onSetLabelRequested)

        self.close_button = RlpGui.GUI_SvgButton(
            ":/feather/x-circle.svg",
            self, 17
        )
        self.close_button.setPos(self._iw - 15, 3)
        self.close_button.buttonPressed.connect(self.onRemoveRequested)

        self._frame = frame
        self.frameStr = QStr(str(frame))
        self.frame_button = RlpGui.GUI_SvgButton(
            ':misc/frame.svg', self, 15)
        self.frame_button.setPos(8, self._ih + 7 + 20)
        self.frame_button.buttonPressed.connect(self.onGotoFrameRequested)

        self.setWidth(self.image.width() + 6)
        self.setHeight(self._ih + 6 + 20 + 20)

        self._dragStarted = False

        fileFd, filename = tempfile.mkstemp('.jpg', prefix='{}_'.format(
            rlp_core_util.slugify(name)))
        os.close(fileFd)

        self.filename = filename
        self._label = ''
        self.label = QStr(self._label)

        self.image.save(self.filename)

        print('Saved {}'.format(filename))


    def onRemoveRequested(self, md):
        self.requestRemove.emit({'thumb': self})


    def onGotoFrameRequested(self, md):
        revlens.cmds.goto_frame(self._frame)


    def onSetLabelRequested(self, md):

        dlg = InputDialog.create('Set Label', self._label, icon='edit-2')

        def _onAccept(md):
            self._label = md['value']
            self.label = QStr(self._label)
            self.update()

        dlg.accept.connect(_onAccept)


    def hoverEnterEventItem(self, event):
        self._inHover = True
        self.update()


    def hoverLeaveEventItem(self, event):
        self._inHover = False
        self.update()


    def mousePressEventItem(self, event):
        self.buttonPressed.emit({'image': self.image, 'fimage': self.fimage})


    def mouseMoveEventItem(self, event):

        if self.inDragAndDrop():
            return
        
        pos = event.position()

        ddLabel = self._name
        destPath = self.filename
        if self._label:
            base, filename = os.path.split(self.filename)
            newFilename = '{}__{}'.format(self._label, filename)
            destPath = os.path.join(base, newFilename)

            print('{} -> {}'.format(self.filename, destPath))
            shutil.copyfile(self.filename, destPath)
            ddLabel = self._label


        mimeText = 'file://{}'.format(destPath)
        self.setupDragAndDrop(ddLabel, pos.x(), pos.y(), 'text/uri-list', mimeText)


    def paintItem(self, painter):

        col = QtGui.QColor(70, 70, 70)
        if self._inHover:
            col = QtGui.QColor(90, 90, 90)

        painter.setPen(QtGui.QPen(col))
        painter.drawRect(self.boundingRect())
        painter.drawImage(3, 3 + 20, self.image)

        tp = QtGui.QPen(QtGui.QColor(200, 200, 200))

        f = painter.font()
        f.setBold(True)
        f.setPixelSize(11)

        painter.setPen(tp)
        painter.setFont(f)
        
        if self._label:
            painter.drawText(30, 15, self.label)

        else:
            painter.drawText(30, 15, self.name)


        painter.drawText(30, self._ih + 20 + 18, self.frameStr)


class SnapshotPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.thumbSelected = QtCore.PY_Signal(self)

        # self.preview = FullSizeView(self)
        self.toolbar = RlpGui.GUI_HLayout(self)
        # self.toolbar.setHeight(30)
        self.toolbar.setSpacing(10)

        self.snapshotButton = RlpGui.GUI_SvgButton(':feather/camera.svg', self, 20)
        self.snapshotButton.buttonPressed.connect(self.onSnapshotRequested)

        self.flayout = RlpGui.GUI_FlowLayout(self)

        self.toolbar.addItem(self.snapshotButton, 0)
        self.thumbs = [] # needed for reference counting... ASDLFKJSFJ

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.toolbar, 0)
        self.layout.addItem(self.flayout, 0)

        revlens.CNTRL.noticeEvent.connect(self.onNoticeEvent)

        self.onParentSizeChangedItem(parent.width(), parent.height())

    
    def onSnapshotRequested(self, md):
        print(md)

        dIdx = 0
        if 'display_idx' in md:
            dIdx = md['display_idx']

        display = revlens.CNTRL.getVideoManager().getDisplayByIdx(0)
        if not display:
            print('No display, aborting')
            return

        currFrameNum = revlens.CNTRL.currentFrameNum()
        mediaName = ''
        node = revlens.CNTRL.session().getNodeByFrame(currFrameNum)
        if node:
            mediaName = node.getProperties().get('media.name', '')

        imageList = display.getCurrentFramebufferList()

        flattenedImage = imageList[0]
        if len(imageList) > 1:
            flattenedImage = rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ.flattenAnnotation(
                imageList[0], imageList[1]
            )

        thumb = SnapshotThumbnail(self, flattenedImage, currFrameNum, mediaName)
        thumb.requestRemove.connect(self.onThumbRemoveRequested)

        # thumb.buttonPressed.connect(self.onThumbClicked)
        self.thumbs.append(thumb)

        self.flayout.addItem(thumb)
        self.flayout.updateItems()

        # self.onParentSizeChangedItem(self.width(), self.height())


    def onThumbRemoveRequested(self, md):

        thumb = md['thumb']
        if thumb in self.thumbs:

            thumb.setVisible(False)
            self.thumbs.remove(thumb)
            self.flayout.removeItem(thumb)

            self.flayout.updateItems()


    def onThumbClicked(self, md):

        print(md)
        # self.preview.setImage(md['fimage'])
        self.thumbSelected.emit(md)


    def onNoticeEvent(self, evtName, evtInfo):

        # print('{} {}'.format(evtName, evtInfo))

        if evtName == 'view.request_snapshot':
            self.onSnapshotRequested(evtInfo)
            return


        if evtName != 'view.snapshot_ready':
            return


        print('GOT SNAPSHOT')
        # imagePath = evtInfo['image_path']

        # image = QtGui.QImage(QtCore.QString(imagePath))
        image = evtInfo['image']
        thumb = SnapshotThumbnail(self, image)
        thumb.buttonPressed.connect(self.onThumbClicked)

        self.thumbs.append(thumb)

        # self.flayout.addItem(thumb)

        self.onParentSizeChangedItem(self.width(), self.height())


    def onParentSizeChangedItem(self, width, height):
        # 
        self.setWidth(width)
        self.setHeight(height)

        halfHeight = height / 2
        halfWidth = width / 2

        self.flayout.setWidth(width - 15)
        # self.flayout.setWidth(halfWidth)

        # self.preview.setHeight(height)
        # self.preview.setWidth(width)
        # self.preview.setPos(halfWidth, 0)


def create(parent):
    return SnapshotPanel(parent)