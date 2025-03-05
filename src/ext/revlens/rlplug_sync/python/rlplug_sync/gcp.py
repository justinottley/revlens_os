
import time

from PythonQt import QtCore

import revlens
import rlp_util
from revlens.service import iosched

from rlp_site.session import RlpSiteSession

from rlplug_sync.panel.transfers import OneTransferDialog, TransferPanel

_DIALOGS = []

def init(creds_in_str, bucket_name):

    app_globals = rlp_util.APPGLOBALS.globals()
    if 'startup.mode.service' in app_globals:
        print('Service Mode, skipping GCP IOSCheduler Queue init')
        return

    print('GCP IOScheduler Queue init')

    qnames = iosched.get_queue_names()

    if 'gcp' not in qnames:

        print('registering GCP IOQueue') #  - bucket name: {}'.format(bucket_name))

        site_session = RlpSiteSession.instance()
        edb_info = site_session.edbc._get_info()

        worker_kwargs = {
            'creds': creds_in_str,
            'bucket_name': bucket_name,
            'edb_info': edb_info
        }

        import pprint
        pprint.pprint(worker_kwargs)

        iosched.register_queue(
            'gcp',
            'rlp_fs.stor_google.bucket.GoogleBucketClientWorkerThread',
            worker_kwargs
        )

    return {'status': 'OK'}



def run_action_dialog(action_name, arg_list):

    floating_pane = revlens.VIEW.createFloatingPane(650, 160, False)
    diag = TransferPanel(floating_pane.body())

    global _DIALOGS
    _DIALOGS.append(diag)


    run_id = iosched.append('gcp', {
        'action': action_name,
        'args': arg_list
    })

    WAIT_MAX = 300
    wait_count = 0

    while wait_count < WAIT_MAX:

        task_state = iosched.get_task_state('gcp', run_id)
        if task_state == 'done':
            break

        for _ in range(10):
            time.sleep(0.1)
            QtCore.QCoreApplication.processEvents()

        wait_count += 1

    floating_pane.onCloseRequested()



def upload_file(storeRelPath, storeFullPath):

    print('UPLOAD FILE {} {}'.format(storeRelPath, storeFullPath))
    
    run_id = iosched.append('gcp', {
        'action': 'upload_file',
        'args': [storeRelPath, storeFullPath]
    })



def download_file(storeRelPath, storeFullPath, alias_path=None):

    print('DOWNLOAD FILE {} {}'.format(storeRelPath, storeFullPath))
    
    run_action_dialog(
        'download_file',
        [storeRelPath, storeFullPath, alias_path]
    )


def delete_file(store_path, entity_type, entity_uuid):

    print('DELETE FILE: {} {} {}'.format(store_path, entity_type, entity_uuid))

    run_action_dialog(
        'delete_file',
        [store_path, entity_type, entity_uuid]
    )

    