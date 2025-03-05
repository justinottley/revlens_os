
from rlp import QtCore, QtGui
from rlp.Qt import QStr

import rlp.core as RlpCore
import rlp.gui as RlpGui

class UserLabel(RlpGui.GUI_ItemBase):

    _CHAR_OFFSET_MAP = {
        'G': [-5, 0],
        'J': [1, 0]
    }

    _ILENGTH_MAP = {
        22: 7
    }

    _utext = RlpCore.UTIL_Text()

    NAME_FONT_SIZE = 12

    def __init__(self, parent, username, first_name, last_name, font_size=2):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._parent = parent
        self.username = username
        self.first_name = first_name or 'Nobody'
        self.first_name = self.first_name.capitalize()
        self.last_name = last_name or 'User'

        self.font_size = font_size
        self._name_xpos = None

        self.name_str = '{} {}'.format(self.first_name, self.last_name)
        self._do_fullname = True

        self.initials = '{}{}'.format(
            self.first_name[0].upper(),
            self.last_name[0].upper())

        self._ilength = -1



    @property
    def name_xpos(self):

        if self._name_xpos is None:
            print('WARNING: _name_xpos not calculated')
            return 120

        return self._name_xpos


    def setFullname(self, do_fullname):
        self._do_fullname = do_fullname


    def onParentSizeChangedItem(self, width, height):
        self.setItemWidth(width)
        self.setItemHeight(50)


    def boundingRect(self):
        return QtCore.QRectF(0, 0, self.width(), self.height())

    def paintItem(self, painter):

        # painter.setAntiAliasing(True)

        col = QtGui.QColor(110, 110, 110)

        p = QtGui.QPen(col)
        b = QtGui.QBrush(col)
        painter.setPen(p)
        painter.setBrush(b)

        painter.drawEllipse(22, 22, 20, 20) # 35, 35) # self.boundingRect())


        painter.save()

        f = painter.font()
        curr_pixel_size = f.pixelSize()
        f.setPixelSize(curr_pixel_size + self.font_size)
        f.setBold(True)

        initials = '{}{}'.format(
            self.first_name[0].upper(),
            self.last_name[0].upper())

        ilength = self._utext.getTextWidthF(initials, f)

        painter.setFont(f)

        if self._name_xpos is None:
            self._name_xpos = self._utext.getTextWidth(self.name_str, f.toString().toStdString())


        p = QtGui.QPen(QtGui.QColor(250, 250, 250)) # QtCore.Qt.white)
        painter.setPen(p)

        # char = self.username[0].upper()
        # char_offset = self._CHAR_OFFSET_MAP.get(char, [-3, 0])
        # char_xpos = 12 + char_offset[0]
        # char_ypos = 25 + char_offset[1]
        char_xpos = 34 - ilength
        if ilength in self._ILENGTH_MAP:
            char_xpos = self._ILENGTH_MAP[ilength]

        char_ypos = 28
        painter.drawText(char_xpos, char_ypos, QStr(initials))

        if self._do_fullname:
            f.setPixelSize(curr_pixel_size)
            painter.setFont(f)
            painter.drawText(45, 20, QStr(self.name_str))

        painter.restore()

