#!/usr/bin/env python

import os
import sys
import json

import ws4py
from ws4py.client import WebSocketBaseClient

from rlp_core.remoting.websocket import JsonRPCClient

class MessageReceiver(JsonRPCClient):

    def received_message(self, msg):
        print('RECEIVED {msg}'.format(msg=msg))
        # self.send(self.build_message('wait_message'))


    def start(self):
        self.connect()
        # self.send(self.build_message('wait_message'))
        self.run()


class TestClient(JsonRPCClient):

    def received_message(self, msg_str):
        print(msg_str)
        print(type(msg_str))
        msg = json.loads(str(msg_str))
        print(msg)
        
        print('made it here?')
        self.prompt()


    def prompt(self):
        
        print('IN PROMPT!')
        
        message = raw_input('Enter something: >')
        message_parts = message.split()
        command = message_parts[0]
        args = message_parts[1:]
        
        handler = '_send_{cmd}'.format(cmd=command)
        if hasattr(self, handler):
            getattr(self, handler)(args)
            
        else:
            print('SENDING ' + command)
            return self.send(self.build_message(command, args))


    def start(self):
        
        self.connect()
        self.prompt()
        self.run()


class RPCTest(JsonRPCClient):

    def start(self):

        args = []
        self.connect()
        self.send(self.build_message('dpix.query', args))
        self.run()


def main():

    port = sys.argv[1]
    # server_host = '192.168.11.93:8401'
    server_host = '127.0.1.1:{}'.format(port)
    ws_url = 'ws://{sh}/ws'.format(sh=server_host)

    print('Using {ws}'.format(ws=ws_url))

    if sys.argv[2] == 'msg':
        tc = TestClient(ws_url)
        tc.start()

    elif sys.argv[2] == 'wait':

        print('Starting message receiver')
        mr = MessageReceiver(ws_url)
        mr.start()

    elif sys.argv[2] == 'dpix':
        mr = RPCTest(ws_url)
        mr.start()

if __name__ == '__main__':
    main()