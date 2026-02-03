
import uuid
import pprint
import traceback

from rlp import QtCore
from rlp.core.net.websocket.rpc_future import RPCFuture

import revlens

class RlpIOServiceClient(QtCore.QObject):

    def __init__(self, qname, serviceName='iosched'):
        QtCore.QObject.__init__(self)

        self._serviceName = serviceName
        self._qname = qname

        self.skwargs = {}
        self._callback_map = {}

        self._smgr = revlens.CNTRL.getServerManager()
        self._smgr.serviceMessageReady.connect(self.onMessageReceived)


    def call(self, callback, methodName, *args, **kwargs):
        skwargs = {} # ensure unique run_id by creating a new dict
        return RPCFuture(self, callback, methodName, args, kwargs, skwargs)


    def _rpc(self, method, args=None, kwargs=None, skwargs=None):

        args = args or []
        kwargs = kwargs or {}
        skwargs = skwargs or {} # need run_id...

        msgDict = {
            'method': method,
            'args': list(args),
            'kwargs': kwargs,
            'session': skwargs
        }

        envelope = {
            'run_id': skwargs['run_id'], # pull run_id forward into envelope... (ARGH FIX?)
            'method': 'append',
            'kwargs': {
                'qname': self._qname,
                'info': msgDict
            }
        }

        # print('sending message to service {}'.format(self._serviceName))
        # pprint.pprint(envelope)

        self._smgr.sendCallToService(self._serviceName, envelope)


    def onMessageReceived(self, msgDict):
        # print('ON MESSAGE RECEIVED!!: {}'.format(msgDict))

        try:
            runId = None
            result = None


            if 'extra' in msgDict:
                runId = msgDict['extra'].get('run_id')
                result = msgDict['extra'].get('result')

            elif 'run_id' in msgDict:
                runId = msgDict['run_id']
                result = msgDict['result']


            if runId in self._callback_map:
                if msgDict['levelname'] == 'PROGRESS':
                    self._callback_map[runId].progress(msgDict)

                else:
                    self._callback_map[runId].done(result)


        except:
            print(traceback.format_exc())


