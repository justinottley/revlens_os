
import uuid
import pprint
import logging

import rlp.util
from rlp.core import CoreNet_WebSocketClient

from .rpc_future import RPCFuture

class ServerError(Exception): pass

class _CallbackWrapper:

    def __init__(self, callback):
        self.callback = callback

    def done(self, result):
        self.callback(result)


class RlpQtWsClient(object):
    '''
    This client is meant to be used by / compatible with the WASM deployment in the browser.
    The WASM environment appears to have no working way to make a syncronous
    call - ie, waiting while hitting the event loop, either from python or C++.

    To get around this (for now??) this client does not ever wait, and is fully
    asyncronous.
    '''
    def __init__(self, url, *args, **kwargs):
        
        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.__url = url

        self.__ident = None # this client's ident as seen from the server, registered when we receive a message

        self.connected = False

        self._wsclient = CoreNet_WebSocketClient()
        self._wsclient.messageReceived.connect(self.on_message_received)
        self._wsclient.onClientConnected.connect(self.on_connected)
        self._wsclient.onClientDisconnected.connect(self.on_disconnected)


        method_prefix = ''
        if 'method_prefix' in kwargs:
            method_prefix = kwargs.pop('method_prefix')

        encrypted = False
        if 'encrypted' in kwargs:
            encrypted = kwargs.pop('encrypted')

        self._wsclient.setModeEncrypted(encrypted)

        self._msg_done = False
        self._msg = None

        self.__client_type = 'rpc.ws'

        self.method_prefix = method_prefix
        # self.encrypted = encrypted

        self.skwargs = {}
        self._callback_map = {}


    @property
    def url(self):
        return self.__url

    @property
    def ident(self):
        return self.__ident


    @property
    def encrypted(self):
        return self._wsclient.isEncrypted()

    def client_type(self):
        return self.__client_type

    def _encrypt(self, msg):
        return rlp.util.AUTH.encrypt(msg)

    def _decrypt(self, msg):
        return rlp.util.AUTH.decrypt(msg)


    def _send(self, msg):

        # print('_send() {}'.format(msg))

        self._wsclient.sendTextMessage(msg)


    def _rpc(self, method, args=None, kwargs=None, skwargs=None):

        # print('_rpc {} {} {} {}'.format(method, args, kwargs, skwargs))

        args = args or []
        kwargs = kwargs or {}

        msg = self.build_message(method, args, kwargs, skwargs)

        return self._send(msg)


    def connect(self):

        if self.connected:
            print('ALREADY CONNECTED')
            self.on_connected()
            return

        self._wsclient.setUrl(self.url)


    def build_message(self, method, args, kwargs, skwargs):

        # print('build_message {} {} {} {}'.format(method, args, kwargs, skwargs))

        return self._wsclient.formatMethodCall(
            method,
            list(args),
            kwargs,
            skwargs
        )


    def on_connected(self):
        print('CONNECTED')
        self.connected = True


    def on_disconnected(self):
        print('{}: disconnected'.format(self.url))
        # self.sig_disconnected.emit()


    def on_message_received(self, msg):
        
        # print('')
        # print('RECEIVED_MESSAGE \n{}'.format(pprint.pformat(msg)))

        # self._msg = msg
        # self._msg_done = True

        # pprint.pprint(self._callback_map)

        return_result = False

        if 'ident' in msg and not self.__ident:
            client_ident = msg['ident']
            print('Registering ident: {}'.format(client_ident))
            self.__ident = client_ident

        if msg and 'status' in msg:
            if msg['status'] == 'ERR':
                # raise ServerError(msg['err_msg'])
                self.LOG.error(msg['err_msg'])
                self._wsclient.emitErrorReceived(msg)

            elif msg['status'] == 'OK':

                msg_result = msg['result']
                if 'run_id' in msg and msg['run_id'] in self._callback_map:
                    self._callback_map[msg['run_id']].done(msg_result)
                    return_result = True

                elif msg_result and type(msg_result) == dict and 'extra' in msg_result and 'metadata' in msg_result['extra']:
                    mdata = msg_result['extra']['metadata']
                    if 'run_id' in mdata and mdata['run_id'] in self._callback_map:
                        self._callback_map[mdata['run_id']].done(msg_result)
                        return_result = True


        else:
            # print('WARNING: skipping message:')
            # print(msg)
            pass

        return return_result



    def call(self, callback, method_name, *args, **kwargs):
        return RPCFuture(self, callback, method_name, args, kwargs)

    def callp(self, callback, method_name, *args, **kwargs):
        return RPCFuture(self, callback, method_name, args, kwargs, persistent=True)

    def register_callback(self, run_id, callback):

        print('REGISTER CALLBACK {} {}'.format(run_id, callback))
        self._callback_map[run_id] = _CallbackWrapper(callback)

    def deregister_callback(self, run_id):
        if run_id in self._callback_map:
            print('DEREGISTERING CALLBACK AT {}'.format(run_id))
            del self._callback_map[run_id]

        else:
            print('WARNING: DEREGISTER CALLBACK: CALLBACK NOT FOUND AT  {}'.format(run_id))