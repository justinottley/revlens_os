
import pprint
import logging

from rlp import QtCore, QtGui
import rlp.gui as RlpGui


_PANELS = []
_LOGS = []


class TransfersPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        # self.setFlag(QtGui.QGraphicsItem.ItemClipsChildrenToShape)

        self.transfers = {}

        self.layout = RlpGui.GUI_VLayout(self)
        self.top_label = RlpGui.GUI_Label(self, '')

        self.layout.addItem(self.top_label, 0)
        self.layout.addSpacer(10)

        self.init_logs()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def onParentSizeChangedItem(self, width, height):

        # print('onSizeChanged {} {}'.format(width, height))

        self.setItemWidth(width)
        self.setItemHeight(height)

        for transfer_widget in self.transfers.values():
            transfer_widget.setItemWidth(width - 10)
            transfer_widget.setText(transfer_widget.text())


    def init_logs(self):

        global _LOGS
        for log_entry in _LOGS:
            self.on_logrecord_received(log_entry)


        _LOGS = []


    def on_logrecord_received(self, logrecord_info):

        # print('LOGRECORD: {}'.format(logrecord_info))

        if logrecord_info['levelname'] != 'PROGRESS':
            self.top_label.setText('{} [ {} ] : {}'.format(
                logrecord_info['logger'].split('.')[-1],
                logrecord_info['levelname'],
                logrecord_info['message']
            ))
            print(logrecord_info['message'])


        else:
            # progress

            tr_info = logrecord_info['extra']
            run_id = tr_info['metadata']['run_id']

            if run_id not in self.transfers:
                
                # transfer_widget = RlpGui.GUI_TextEdit(self, self.width() - 20, 30)
                # TODO FIXME: TextEdit not updating?
                #

                transfer_widget = RlpGui.GUI_Label(self, "HELLO")
                transfer_widget.setItemWidth(200)
                transfer_widget.setItemHeight(30)
                transfer_widget.setText("-")
                # transfer_widget.setWordWrap(True)
                # transfer_widget.setHScrollBarVisible(False)

                self.transfers[run_id] = transfer_widget # RlpGui.GUI_Label(self, "")
                self.layout.addItem(transfer_widget, 0)

                self.layout.onItemSizeChanged()

            self.transfers[run_id].setText(
                '{} : {}% - {}'.format(
                    tr_info['action'], tr_info['value'], tr_info.get('display_title', '')
            ))

            self.top_label.setText(self.top_label.text())

            self.updateItem()
            self.onParentSizeChangedItem(self.width(), self.height())

            # pprint.pprint(logrecord_info)
            # pass


def on_logrecord_received(logrecord_info):

    global _PANELS
    global _LOGS

    # print('IN PY: {}'.format(logrecord_info))

    if not _PANELS:
        _LOGS.append(logrecord_info)

    else:
        for panel in _PANELS:
            panel.on_logrecord_received(logrecord_info)


def create(parent):
    transfers_panel = TransfersPanel(parent)

    global _PANELS
    _PANELS.append(transfers_panel)

    return transfers_panel