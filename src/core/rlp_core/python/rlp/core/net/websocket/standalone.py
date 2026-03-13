
import json

from .wsjson import JsonRPCClient
from .rpc_future import RPCFuture

from . import RlpClient


class ServerError(Exception): pass

class _StandaloneFutureHelper:
    '''
    Meant to mimic the API call structure of the async client so
    client code can use the async-style or the sync-style client
    the same way
    '''
    def __init__(self, callback, result):
        self.callback = callback
        self.result = result

    def run(self):
        self.callback(self.result)



class StandaloneWsClient(JsonRPCClient):

    def __init__(self, *args, **kwargs):
        method_prefix = ''
        if 'method_prefix' in kwargs:
            method_prefix = kwargs.pop('method_prefix')


        do_return_msg_envelope = False
        if 'return_msg_envelope' in kwargs:
            do_return_msg_envelope = kwargs.pop('return_msg_envelope')

        if 'encrypted' not in kwargs:
            kwargs['encrypted'] = True

        JsonRPCClient.__init__(self, *args, **kwargs)

        self._msg_done = False
        self._msg = None
        self._run_id = None
        self.return_msg_envelope = do_return_msg_envelope

        self.__client_type = 'rpc.ws'

        self.method_prefix = method_prefix

        self.skwargs = {}
        self._callback_map = {}


    @classmethod
    def init(cls, site_info):
        client_obj = cls(site_info)
        RlpClient._INSTANCE = client_obj

        return client_obj


    def _wait(self):
        while not self._msg_done:
            self.once()

        self._msg_done = False

    def _sync_send(self, *args, **kwargs):

        self.send(*args, **kwargs)
        self._wait()

        if 'status' in self._msg and self._msg['status'] == 'ERR':
            raise ServerError(self._msg['err_msg'])

        if self.return_msg_envelope:
            return self._msg

        if 'result' not in self._msg:
            # raise ServerError('no result: {}'.format(self._msg))
            self.LOG.error('no result: {}'.format(self._msg))
            self._msg['result'] = None

        return self._msg['result']

    def _rpc(self, method, args=None, kwargs=None, skwargs=None):

        args = args or []
        kwargs = kwargs or {}

        msg = self.build_message(method, args, kwargs, skwargs)

        result = self._sync_send(msg)

        return result


    def received_message(self, msg_str):

        if self.encrypted:
            msg_str = self._decrypt(msg_str)

        self._msg = json.loads(str(msg_str))
        self._msg_done = True



    def calls(self, method_name, *args, **kwargs):
        '''
        Syncronous call - return result
        '''
        future = RPCFuture(self, None, method_name, args, kwargs)
        self._run_id = future.run_id
        future.run()

        return self._msg['result']


    def call(self, callback, method_name, *args, **kwargs):

        future = RPCFuture(self, None, method_name, args, kwargs)
        self._run_id = future.run_id
        future.run()

        helper_obj = _StandaloneFutureHelper(callback, self._msg['result'])
        return helper_obj