
from rlp import QtCore
import rlp.gui as RlpGui

from rlp.gui.dialogs import ConfirmDialog

class RenameDialog(ConfirmDialog):

    def __init__(self, *args, **kwargs):
        ConfirmDialog.__init__(self, *args, **kwargs)

    def _init_layout(self):
        self.new_room_layout = RlpGui.GUI_HLayout(self)

        self.new_room_label = RlpGui.GUI_Label(self, "New Name: ")
        self.new_room_text = RlpGui.GUI_TextEdit(self, 200, 60)

        self.new_room_layout.addItem(self.new_room_label, 0)
        self.new_room_layout.addItem(self.new_room_text, 0)
        
        self.layout.addItem(self.new_room_layout, 0)
        self.layout.addSpacer(20)
        self.layout.addItem(self.btn_layout, 0)
        self.layout.addSpacer(30)


class NewRoomDialog(RlpGui.GUI_ItemBase):

    def __init__(self, floating_pane):
        RlpGui.GUI_ItemBase.__init__(self, floating_pane.body())

        self.sig_new_room = QtCore.PySignal() # QtCore.Signal(dict)
        self.sig_done = QtCore.PySignal() # sig_done = QtCore.Signal()

        self._fpane = floating_pane

        label = 'New Room'
        
        self.title_label = RlpGui.GUI_Label(self, label)

        self.room_name_label = RlpGui.GUI_Label(self, "Room Name: ")

        self.room_name_input = RlpGui.GUI_TextEdit(self, 200, 60)
        
        self.room_name_layout = RlpGui.GUI_HLayout(self)
        self.room_name_layout.setSpacing(10)
        self.room_name_layout.addItem(self.room_name_label, 0)
        self.room_name_layout.addItem(self.room_name_input, 0)

        self.create_button = RlpGui.GUI_IconButton('', self, 20, 4)
        self.create_button.setText("Create")
        self.create_button.setOutlined(True)
        self.create_button.buttonPressed.connect(self.on_create_clicked)

        self.empty_radio = RlpGui.GUI_RadioButton(self, "Empty", 20, 0)
        self.existing_radio = RlpGui.GUI_RadioButton(self, "Current Session", 20, 0)

        self._radio_layout = RlpGui.GUI_HLayout(self)
        self.radio_group = RlpGui.GUI_RadioButtonGroup(self, self._radio_layout, '')
        self._radio_layout.addItem(self.empty_radio, 0)
        self._radio_layout.addItem(self.existing_radio, 0)


        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.title_label, 0)
        self.layout.addSpacer(20)
        self.layout.addItem(self.room_name_layout, 0)
        self.layout.addSpacer(10)
        self.layout.addItem(self._radio_layout, 0)
        self.layout.addSpacer(15)
        self.layout.addItem(self.create_button, 50)

        self.setItemWidth(self._fpane.width())
        self.setItemHeight(self._fpane.height())


    @property
    def room_type(self):

        result = 'unknown'
        if self.empty_radio.isToggled():
            result = 'empty'

        elif self.existing_radio.isToggled():
            result = 'current_session'

        return result


    def on_create_clicked(self, md):

        print('CREATE!')

        room_name = self.room_name_input.text()

        print(room_name)
        self.sig_new_room.emit({
            'room_name': room_name,
            'room_type': self.room_type
        })

        self._fpane.requestClose()

        self.sig_done.emit({})


