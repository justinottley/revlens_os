import time

from rlp import QtCore

import revlens

WAIT_MAX = 10

def _call(method, args=None, kwargs=None, wait=False, retry_count=1):

    smgr = revlens.CNTRL.getServerManager()

    args = args or []
    kwargs = kwargs or {}
    payload = {'method': method, 'args': args, 'kwargs': kwargs}

    if wait:

        for retry_num in range(retry_count):
            
            wait_count = 0
            print('Calling {} - try {} / {}'.format(method, retry_num, retry_count))
    
            run_id = smgr.sendCallInternalTo('iosched', payload)
            while wait_count < WAIT_MAX:

                if smgr.hasCallResult(run_id):
                    result_envelope = smgr.getCallResult(run_id)
                    return result_envelope['result']

                for _ in range(5):

                    time.sleep(0.2)
                    QtCore.QCoreApplication.processEvents()

                print('Waiting on {}..'.format(method))
                wait_count += 1

        raise TimeoutError('max wait exceeded for {}'.format(run_id))

    else:
        smgr.sendCallInternalTo(
            'iosched', payload)


def get_queue_names():
    return _call('get_queue_names', wait=True, retry_count=3)


def register_queue(name, worker_ns, worker_kwargs, size=1):
    return _call('register_queue', [name, worker_ns, worker_kwargs], {'size': size})


def append(q_name, payload):
    return _call('append', [q_name, payload], wait=True)


def get_task_state(q_name, run_id):
    return _call('get_task_state', [q_name, run_id], wait=True)

def force_quit():
    return _call('force_quit')

