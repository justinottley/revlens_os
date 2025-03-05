#!/usr/bin/env python3

import os
import sys
import json
import time
import pprint
import logging
import argparse
import threading
import traceback

from rlp.core.net.websocket.wsjson import (
    JsonRPCClient,
    JsonRPCLogHandler
)

from rlp.core.service.ioscheduler import IOScheduler

LOG = logging.getLogger('rlp.io_service')



class MessageReceiver(JsonRPCClient):

    def __init__(self, *args, **kwargs):
        JsonRPCClient.__init__(self, *args, **kwargs)

        self._msg_done = False

        self.sch = IOScheduler()

        self._sched_t = threading.Thread(target=self.sch.run)
        self._sched_t.start()


    def _wait(self):
        while not self._msg_done:
            self.once()

        self._msg_done = False


    def _init_service_mode(self):

        self.send(self.build_message('rfs.register_service', ['iosched'], {}))
        # self._wait()


    def _dispatch(self, rpc_info):

        # print('DISPATCH {}'.format(rpc_info))

        method_name = rpc_info.get('method')
        args = rpc_info.get('args', [])
        kwargs = rpc_info.get('kwargs', {})

        if not method_name or not hasattr(self.sch, method_name):
            print('WARNING: cannot dispatch method "{}" - no handler'.format(method_name))
            return

        if 'run_id' in rpc_info:
            kwargs['run_id'] = rpc_info['run_id']

        # print('DISPATCH SKIPPED')
        result = None

        result = None
        status = 'unknown'
        err_msg = None

        try:
            func = getattr(self.sch, method_name)
            result = func(*args, **kwargs)
            print('{}() result: {}'.format(method_name, result))
            status = 'OK'

            
        except:
            err_msg = traceback.format_exc()
            print(err_msg)
            status = 'err'

        if 'run_id' in rpc_info:
            run_id = rpc_info['run_id']
            return_payload = {
                'run_id': run_id,
                'method': '__return__',
                'result': result,
                'status': status
            }

            if status == 'err':
                return_payload['err_msg'] = err_msg


            msg_str = json.dumps(return_payload)
            if self.encrypted:
                msg_str = self._encrypt(msg_str)

            self.send(msg_str)

        
            # self._wait()
        
        

    def received_message(self, msg_str):
        
        if self.encrypted:
            msg_str = self._decrypt(msg_str)

        rpc_info = json.loads(str(msg_str))
        
        self._msg_done = True

        self._dispatch(rpc_info)

    def closed(self, code, reason=None):

        print('Closed!!! {} {}'.format(code, reason))
        self.sch.trigger_shutdown()

        if os.name == 'posix':

            kill_cmd = 'kill -9 {}'.format(os.getpid())
            print(kill_cmd)
            os.system(kill_cmd)

        sys.exit()



def main_sched():

    sch = IOScheduler()
    sch.register_queue('gcp')

    t = threading.Thread(target=sch.run)
    t.start()

    time.sleep(5)
    
    tp = {'test': 'the'}
    sch.append('gcp', tp)


    time.sleep(6)

    sch.append('gcp',tp)

    time.sleep(10)


def main():

    port = 8051
    if len(sys.argv) > 1:
        port = sys.argv[1]

    ws_protocol = 'ws'
    ws_url = '{}://127.0.0.1:{}'.format(ws_protocol, port)
    print(ws_url)

    io_log_handler = JsonRPCLogHandler(ws_url, encrypted=True)
    io_log_handler.setLevel(1)

    rlp_logger = logging.getLogger('rlp')
    rlp_logger.setLevel(1)
    rlp_logger.addHandler(io_log_handler)
    
    
    LOG.info('Starting IOService Message Receiver')


    mr = MessageReceiver(ws_url, encrypted=True)
    mr.connect()
    mr._init_service_mode()
    mr.run()


def test_cmd():

    data = {'method': 'append', 'args': ['gcp', {'path': '/path/to/something'}]}

    # revlens.CNTRL.getServerManager().sendCallInternalTo('iosched', {'method': 'register_queue', 'args': ['gcp', 'rlp_fs.stor_google.GoogleBucketClientWorkerThread', {'creds': '', 'bucket_name': 'test'}]})
    # revlens.CNTRL.getServerManager().sendCallInternalTo('iosched', {'method': 'append', 'args': ['gcp', {'path': '/path/to/something'}]})

if __name__ == '__main__':
    main()