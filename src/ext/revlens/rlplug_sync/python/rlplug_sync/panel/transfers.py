
import os
import pprint
from collections import OrderedDict

from rlp import QtCore
import rlp.gui as RlpGui

import revlens

import rlplug_qtbuiltin.cmds

from rlp_prod.site.client_ws import SiteWsClient

G_PANEL = None

class TransferContainer(object):

    _INSTANCE = None

    def __init__(self):

        self.transfers = OrderedDict()

        self.panels = [] # hack..

    @classmethod
    def instance(cls):
        if cls._INSTANCE is None:
            cls._INSTANCE = cls()

        return cls._INSTANCE

    @property
    def edbc(self):
        return SiteWsSession.instance().edbc


    def on_data_io_event(self, data_event):

        store_path = data_event['store_path']
        if store_path not in self.transfers:

            # print('adding transfer: {}'.format(store_path))
            # print(data_event)

            media_title = os.path.basename(data_event['full_path'])
            eresult = self.edbc.create('Media', store_path, {
                'title': media_title,
                'size': data_event['size_total'],
                'store_uuid': data_event['uuid']
            })

            # print(eresult)

            self.transfers[store_path] = {}

            rlplug_qtbuiltin.cmds.display_message(
                'Transfer: {} - {}'.format(data_event['status'], media_title)
            )


        curr_event = self.transfers[store_path]

        if data_event['status'] != curr_event.get('status') or \
        data_event['percent'] != curr_event.get('percent'):


            self.transfers[store_path].update(data_event)

            # print('PY data_io_event: {}'.format(data_event))
            for panel in self.panels:
                panel.cb_data_io_event(data_event)


class TransferPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.tr_labels = OrderedDict()

        self.status_label = RlpGui.GUI_Label(self, '')

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.addItem(self.status_label, 0)
        self.layout.addSpacer(10)

        '''
        # DEPRECATED / DISABLED
        # TODO FIXME: resurrect to see transfers before
        # UI was opened
        #
        self.tc = TransferContainer.instance()
        self.tc.panels.append(self)

        for store_path, tr_info in self.tc.transfers.items():
            self._init_transfer(tr_info)
        '''


        revlens.CNTRL.remoteCall.connect(self.on_remote_call_event)

    @property
    def edbc(self):
        return SiteWsClient.instance().edbc


    def _get_transfer_info(self, log_data):

        log_extra = log_data['extra']
        if 'store_path' in log_extra:

            store_path = log_extra['store_path']
            local_path = log_extra['local_path']
            progress_message = log_data['message']

        elif 'args' in log_extra:
            store_path = log_extra['args'][0]
            local_path = log_extra['args'][1]
            progress_message = 'Pending'

        else:
            raise Exception('unknown log data')


        return {
            'store_path': store_path,
            'local_path': local_path,
            'message': progress_message
        }


    def _init_transfer(self, tr_info):

        transfer_info = self._get_transfer_info(tr_info)
        store_path = transfer_info['store_path']

        if store_path in self.tr_labels: # sanity
            return


        tr_label = RlpGui.GUI_Label(self, '')
        self.tr_labels[store_path] = tr_label

        self.layout.addItem(tr_label)


    def cb_data_io_event(self, data_event):
        '''
        DEPRECATED
        '''
        # print('PANEL: {}'.format(data_event))

        store_path = data_event['store_path']
        if store_path not in self.tr_labels:
            self._init_transfer(data_event)

        tr_label = self.tr_labels[store_path]['label']
        tr_label.setText('{} : {} - {}% - {} / {}'.format(
            os.path.basename(data_event['full_path']),
            data_event['status'],
            int(data_event['percent'] * 100),
            data_event['data_count'],
            data_event['size_total'])
        )


    def _log_transfer_message(self, transfer_type, log_data):
        
        transfer_info = self._get_transfer_info(log_data)
        store_path = transfer_info['store_path']
        local_path = transfer_info['local_path']
        progress_message = transfer_info['message']


        if store_path not in self.tr_labels:
            self._init_transfer(log_data)

        message = '{}: {} - {}'.format(
            transfer_type, os.path.basename(local_path), progress_message
        )

        tr_label = self.tr_labels[store_path]
        tr_label.setText(message)


    def filter_log(self, log_data):
        return False


    def on_remote_call_event(self, event):

        if event.get('method') == '__logrecord__':
            
            log_data = event['result']

            # pprint.pprint(log_data)
            if self.filter_log(log_data):
                return

            extra = log_data.get('extra', {})
            action_name = extra.get('action')

            if action_name in ['upload_file', 'download_file']:
                self._log_transfer_message(action_name, log_data)

            else:
                log_msg = '{} [ {} ] {}'.format(
                    log_data['logger'],
                    log_data['levelname'],
                    log_data['message']
                )
                self.status_label.setText(log_msg)



class OneTransferDialog(TransferPanel):

    def __init__(self, action, store_path, fpane):
        TransferPanel.__init__(self, fpane.body())

        self.store_path = store_path
        self._fpane = fpane

        self.status_label.setText('{}: {}'.format(
            action, self.store_path
        ))

    def filter_log(self, log_data):

        print('filter log!!! {}'.format(log_data))
        return False

        log_extra = log_data.get('extra', {})
        log_store_path = log_extra.get('store_path')
        if log_store_path == self.store_path:
            return False

        return True


    def close(self):
        self._fpane.requestClose()


def create(parent):

    global G_PANEL
    G_PANEL = TransferPanel(parent)
    return G_PANEL