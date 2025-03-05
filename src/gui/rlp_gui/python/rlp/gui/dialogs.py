
from rlp.Qt import QStr
from rlp import QtCore, QtGui
import rlp.gui as RlpGui


class InfoDialog(RlpGui.GUI_ItemBase):

    _UI = []

    def __init__(self, floating_pane, title, text, icon='alert-circle'):
        RlpGui.GUI_ItemBase.__init__(self, floating_pane.body())

        self._fpane = floating_pane

        self._fpane.setText(title)
        self._fpane.setTextBold(True)

        self.icon = RlpGui.GUI_SvgIcon(
            ':feather/lightgrey/{}.svg'.format(icon),
            self,
            60,
            0,
            QtGui.QColor(220, 220, 220)
        )

        # self.icon.setSize(60, 60)
        self.icon.setPos(20, 30)

        self.text = RlpGui.GUI_Label(self, text)
        self.text.setPos(100, 30)

        self.text2 = RlpGui.GUI_Label(self, '')
        self.text2.setPos(100, 60)

        self._fpane.setItemWidth(self.text.width() + 140)
        cbtn = self._fpane.closeButton()
        cbtn.setPos(self._fpane.width() - cbtn.width() - 10, 6)


    @classmethod
    def create(cls, title, text, width=300, height=180, icon='alert-circle'):

        pane = RlpGui.VIEW.createFloatingPane(width, height, False)
        panel = cls(pane, title, text, icon)

        pane.setItemWidth(width)
        pane.setItemHeight(height)

        cbtn = pane.closeButton()
        cbtn.setPos(pane.width() - cbtn.width() - 10, 6)

        panel.setItemWidth(width)
        panel.setItemHeight(height)

        cls._UI.append(panel)

        return panel

    def setPersistent(self, persistent):
        self._fpane.setPersistent(persistent)

    def setCloseButtonVisible(self, isvis):
        self._fpane.setCloseButtonVisible(isvis)

    def setText(self, text):
        self.text.setText(text)

    def setText2(self, text):
        self.text2.setText(text)

    def setTitle(self, title):
        self._fpane.setText(title)


    def hideDialog(self):
        self._fpane.hideItem()

    def showDialog(self):
        self._fpane.showItem()


    def close(self, md=None):
        self._fpane.requestClose()

    def refresh(self):

        self.text.update()
        self.text2.update()
        self._fpane.update()
        self._fpane.showItem()


class ConfirmDialog(InfoDialog):

    def __init__(self, *args, accept_label='Okay', **kwargs):
        InfoDialog.__init__(self, *args, **kwargs)

        self.accept = QtCore.PySignal()

        self.layout = RlpGui.GUI_VLayout(self)

        self.accept_button = RlpGui.GUI_IconButton("", self, 20, 4)
        self.accept_button.setText(accept_label)
        self.accept_button.setOutlined(True)

        self.accept_button.buttonPressed.connect(self.on_accept_requested)

        self.cancel_button = RlpGui.GUI_IconButton("", self, 20, 4)
        self.cancel_button.setText("Cancel")
        self.cancel_button.setOutlined(True)

        self.cancel_button.buttonPressed.connect(self.close)

        
        self.button_layout = RlpGui.GUI_HLayout(self)
        self.button_layout.addSpacer(90)
        self.button_layout.addItem(self.accept_button, 0)
        self.button_layout.addSpacer(50)
        self.button_layout.addItem(self.cancel_button, 0)

        self._init_layout()

        self.layout.addItem(self.button_layout, 0)

        self.setWidth(self._fpane.width())
        self.setHeight(self._fpane.height())


    def _init_layout(self):
        self.layout.addSpacer(110)
        pass


    @classmethod
    def create(cls, title, text, accept_label='Okay', width=340, height=210, icon='alert-circle'):

        pane = RlpGui.VIEW.createFloatingPane(width, height, False)
        panel = cls(pane, title, text, accept_label=accept_label, icon=icon)
        cls._UI.append(panel)

        return panel

    @property
    def value(self):
        return None

    def on_accept_requested(self, md):
        self.accept.emit({'value': self.value})
        self.close()


class InputDialog(ConfirmDialog):

    def __init__(self, *args, **kwargs):
        ConfirmDialog.__init__(self, *args, **kwargs)

        self.input_text = RlpGui.GUI_TextEdit(self, 170, 60)
        self.input_text.setPos(100, 60)


    @classmethod
    def create(cls, title, text, accept_label='Okay', icon='alert-circle', width=320):

        pane = RlpGui.VIEW.createFloatingPane(width, 210, False)
        panel = cls(pane, title, text, accept_label=accept_label, icon=icon)

        panel._fpane.setWidth(width)
        panel.setWidth(width)
        cbtn = panel._fpane.closeButton()
        cbtn.setPos(panel._fpane.width() - cbtn.width() - 10, 6)

        cls._UI.append(panel)

        return panel


    @property
    def value(self):
        return self.input_text.text()

