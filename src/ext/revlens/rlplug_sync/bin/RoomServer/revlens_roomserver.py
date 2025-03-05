#!/usr/bin/env python3

import os
import sys
import socket
import logging
import argparse
import traceback

revlens_lib_dir = os.path.join(
    os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))),
    'revlens', 'global', 'lib', 'python2.7', 'site-packages')

print(revlens_lib_dir)
sys.path.insert(0, revlens_lib_dir)

from rlp_entitydb.client_ws import EdbWsClient

import rlp_util.env
rlp_util.env.setup_env(update_sys_path=True)


import cherrypy
from ws4py.server.cherrypyserver import WebSocketTool # , WebSocketPlugin
from ws4py.websocket import EchoWebSocket

from rlp_core import rlp_logging
import rlp_core.remoting.util

from rlplug_sync.rooms.service import RoomServerWebSocket
from rlplug_sync.rooms.service.cherrypy_ext import RoomWebSocketPlugin

ROOMSERVER_DEFAULT_PORT = 8004


class Root(object):
    @cherrypy.expose
    def index(self):
        return 'some HTML with a websocket javascript connection'

    @cherrypy.expose
    def ws(self):
        # you can access the class instance through
        handler = cherrypy.request.ws_handler



def init_logging(loglevel=logging.ERROR):
    
    rlp_logging.basic_config('rlp', loglevel)


def main():

    parser = argparse.ArgumentParser('revlens_roomserver')

    parser.add_argument('--loglevel', default='ERROR')
    parser.add_argument('--localhost', action='store_true', default=False, help='start using localhost')
    parser.add_argument('--hostname', help='use this hostname')
    parser.add_argument('--port', default=ROOMSERVER_DEFAULT_PORT)

    args = parser.parse_args()

    init_logging(getattr(logging, args.loglevel))

    hostname = None
    try:
        hostname = rlp_core.remoting.util.get_ip()
    except:
        print(traceback.format_exc())

    if args.localhost:
        hostname = '127.0.0.1'
    
    elif args.hostname:
        hostname = args.hostname

    os.environ['REVLENS_ROOMSERVER_PORT'] = str(args.port)

    edbc_port = int(args.port) - 1
    edbc_uri = 'ws://127.0.0.1:{}'.format(edbc_port)
    print('Attempting EDBC Connection: {}'.format(edbc_uri))

    edbc_ws_client = EdbWsClient(edbc_uri)
    edbc_ws_client.connect()

    RoomServerWebSocket.EDBC = edbc_ws_client


    cherrypy.config.update({
        'server.socket_host': hostname,
        'server.socket_port': int(args.port)
    })

    websocket_plugin = RoomWebSocketPlugin(cherrypy.engine)
    websocket_plugin.subscribe()
    
    cherrypy.tools.websocket = WebSocketTool()
    RoomServerWebSocket.MANAGER = websocket_plugin.manager
    
    print('Running..')
    
    cherrypy.quickstart(Root(), '/', config={
        '/ws': {'tools.websocket.on': True,    
                'tools.websocket.handler_cls': RoomServerWebSocket}
    })
    
    

if __name__ == '__main__':
    main()