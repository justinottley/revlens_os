

import time
import pprint
import functools

from rlp.Qt import QStr
from rlp import QtCore, QtGui
import rlp.gui as RlpGui
from rlp.gui.userlabel import UserLabel

import revlens
import rlplug_sync

from revlens_prod.client_ws import SiteWsClient



_PANELS = []


class Message(RlpGui.GUI_ItemBase):

    MSG_HEIGHT = 80

    def __init__(self, parent, text, username, user_info, create_time=None):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.text = text
        self.username = username
        self.user_info = user_info

        self.create_time = create_time or time.time()
        self.create_time_str = time.strftime(
            '%Y-%m-%d %H:%M:%S', time.localtime(self.create_time))
        

        self.username_label = UserLabel(self,
            self.username,
            self.user_info.get('first_name', '-'),
            self.user_info.get('last_name', '-'),
            3
        )

        self.username_label.setPos(10, 10)


        self.msg_text = RlpGui.GUI_TextEdit(self, self.width(), 80)
        self.msg_text.textArea().setReadOnly(True)
        self.msg_text.setPos(50, 35)
        # self.msg_text.setWordWrap(True) # TODO FIXME NOT WORKING?
        self.msg_text.setOutlined(False)
        self.msg_text.setHScrollBarVisible(False)
        self.msg_text.setText(self.text)

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.msg_text.setWidth(width) #  - 110)

        self.msg_text.setText(self.msg_text.text())

        self.setHeight(self.MSG_HEIGHT) #elf.msg_text.height() + 35 + 10)


    def paintItem(self, painter):

        # p = QtGui.QPen()
        # p.setColor(QtCore.Qt.red)
        # b = QtGui.QBrush(QtGui.QColor(60, 60, 60))
        # painter.setPen(p)
        # painter.setBrush(b)
        # painter.drawRect(self.boundingRect())

        p = painter.pen()
        p.setColor(QtGui.QColor(250, 250, 250))
        
        # painter.save()

        f = painter.font()
        f.setPixelSize(f.pixelSize() - 3)
        painter.setFont(f)
        painter.setPen(p)

        # 55, 43
        xpos = self.username_label.name_xpos
        if xpos:
            painter.drawText(
                65 + xpos,
                25,
                QStr(self.create_time_str)
            )

        # painter.restore()


class ChatPanel(RlpGui.GUI_ItemBase):

    USER_CACHE = {}

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.sendCallToClient.connect(self.on_data_received)

        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.addSpacer(10)

        self.room_label = RlpGui.GUI_Label(self, "-")
        self.room_label.setLeftSvgIconPath(
            ':feather/lightgrey/home.svg', 20, 0)
        self.room_label.hideItem()

        rlFont = self.room_label.font()
        rlFont.setBold(True)
        self.room_label.setFont(rlFont)
        # self.room_label.setVisible(False)

        self.toolbar_layout.addItem(self.room_label, 5)

        self.new_message_textarea = RlpGui.GUI_TextEdit(self, 200, 60)
        self.new_message_textarea.setTempHintText("Write a message..")

        self.new_message_textarea.enterPressed.connect(self.new_message_requested)

        self.layout = RlpGui.GUI_VLayout(self)

        self.message_list_sa = RlpGui.GUI_ScrollArea(self)
        self.message_list_sa.setOutlined(False)
        self.message_list_sa.setMaxChildHeight(1200)
        self.message_list_sa.setPos(0, 30)
        
        self.message_list_layout = RlpGui.GUI_VLayout(self.message_list_sa.content())

        self.messages = []

        self.onParentSizeChangedItem(parent.width(), parent.height())
        
        if not self.edbc:
            self._err_label = RlpGui.GUI_Label(self, "Error: Login required")
            self.toolbar_layout.addItem(self._err_label, 0)

    
    @property
    def edbc(self):
        return SiteWsClient.instance()


    @property
    def curr_room_info(self):
        return {'room_name': '-', 'room_uuid': '-'}

        return {
            'room_name': rlplug_sync.RLSYNC_CONTROLLER.currentRoom(),
            'room_uuid': rlplug_sync.RLSYNC_CONTROLLER.currentRoomUuid()
        }


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width)
        self.setHeight(height)

        self.new_message_textarea.setWidth(width)
        self.new_message_textarea.setPos(10, height - self.new_message_textarea.height() - 20)

        for msg in self.messages:
            msg.onParentSizeChangedItem(width, height)

        self.message_list_layout.onItemSizeChanged()
        self.message_list_layout.setHeight(self.message_list_layout.itemHeight())

        self.message_list_sa.setWidth(width)
        self.message_list_sa.setHeight(height - 60 - self.new_message_textarea.height())

        self.message_list_sa.setMaxChildHeight(self.message_list_layout.itemHeight() + 20)
        self.message_list_sa.setYPosPercent(1.0)


    def add_message(self, room_name, room_uuid, text, username, user_info, create_time, emitSyncAction=True):

        message = Message(self, text, username, user_info, create_time=create_time)

        print(message)

        self.messages.append(message)
        self.message_list_layout.addItem(message, 0)
        self.message_list_layout.setHeight(self.message_list_layout.itemHeight())

        self.message_list_sa.setMaxChildHeight(self.message_list_layout.itemHeight() + 20)
        self.message_list_sa.setYPosPercent(1.0)

        self.new_message_textarea.clear()

        if emitSyncAction:

            sync_info = {
                'text': text,
                'username': username,
                'user_info': user_info,
                'create_time': message.create_time,
                'room_name': room_name,
                'room_uuid': room_uuid
            }

            revlens.CNTRL.emitSyncAction(
                'message', sync_info)




    def new_message_requested(self, emitSyncAction=True):
        
        if not self.edbc:
            return

        text = self.new_message_textarea.text()
        curr_room_info = self.curr_room_info

        room_name = curr_room_info['room_name']
        room_uuid = curr_room_info['room_uuid']
        username = self.edbc.username
        user_info = self.edbc.user_info

        self.add_message(
            room_name,
            room_uuid,
            text,
            username,
            user_info,
            None,
            emitSyncAction=True
        )

        print(text)


        
    def on_data_received(self, payload):

        print('GOT EVENT')
        pprint.pprint(payload)


        action = payload.get('action')
        method = payload.get('method')

        if action == 'message':
            
            if payload['room_name'] != self.curr_room_info['room_name']:
                return


            self.add_message(
                payload['room_name'],
                payload['room_uuid'],
                payload['text'],
                payload['username'],
                payload['user_info'],
                payload['create_time'],
                emitSyncAction=False
            )

        elif method == 'on_room_loaded':

            print('ROOM LOADED')
            
            room_name = payload['kwargs']['room_name']
            room_uuid = payload['kwargs']['room_uuid']

            self.room_label.setText(room_name)
            self.room_label.setItemWidth(300)
            self.room_label.showItem()

            def _load_messages(qresult):

                # sort by create_date
                #
                sqresult = sorted(qresult, key=lambda x: x['create_date'])

                for qentry in sqresult:
                    
                    if 'user_link' not in qentry:
                        print('WARNING: INCOMPLETE MESSAGE DATA, SKIPPING')
                        print(qentry)
                        continue

                    user_info = qentry['user_link']
                    user_uuid = '{' + user_info['uuid'] + '}'

                    if user_uuid not in self.USER_CACHE:

                        def _add_user_to_cache(qentry, uresult):
                            user_info = {}
                            print('User result: {}'.format(uresult))
                            if uresult and len(uresult) == 1:
                                user_info = uresult[0]

                            self.USER_CACHE[user_uuid] = user_info

                            self.add_message(
                                room_name,
                                room_uuid,
                                qentry['text'],
                                qentry['username'],
                                user_info,
                                qentry['create_date'],
                                emitSyncAction=False
                            )


                        self.edbc.call(functools.partial(_add_user_to_cache, qentry), 'edbc.find',
                            'HumanUser',
                            [['uuid', 'is', user_uuid]],
                            ['first_name', 'last_name']
                        ).run()

                    else:
                        user_info = self.USER_CACHE[user_uuid]
                        self.add_message(
                            room_name,
                            room_uuid,
                            qentry['text'],
                            qentry['username'],
                            user_info,
                            qentry['create_date'],
                            emitSyncAction=False
                        )


            self.edbc.call(_load_messages, 'edbc.find',
                'Message', [['room_link', 'is', {'uuid': room_uuid}]],
                ['username', 'user_link', 'text', 'create_date']
            ).run()

            
        

def create(parent):
    chat_panel = ChatPanel(parent)

    # global _PANELS
    # _PANELS.append(chat_panel)

    return chat_panel