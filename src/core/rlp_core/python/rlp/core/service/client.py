
import uuid
import pprint
import traceback

from rlp import QtCore
from rlp.core.net.websocket.rpc_future import RPCFuture

from rlp.core import CoreNet_WebSocketClient


class RlpIOServiceClient(QtCore.QObject):

    CNTRL_SERVICE = None

    def __init__(self, qname, serviceName='iosched'):
        QtCore.QObject.__init__(self)

        self._serviceName = serviceName
        self._qname = qname

        self.skwargs = {}
        self._callback_map = {}

        self.__ready = False # client is fully ready

        if self.__class__.CNTRL_SERVICE:
            cntrl = self.__class__.CNTRL_SERVICE
            cntrl.serviceMessageReady.connect(self.onMessageReceived)


    @property
    def ready(self):
        return self.__ready


    def call(self, callback, methodName, *args, **kwargs):
        skwargs = {} # ensure unique run_id by creating a new dict
        return RPCFuture(self, callback, methodName, args, kwargs, skwargs)


    def _rpc(self, method, args=None, kwargs=None, skwargs=None):

        if self.__class__.CNTRL_SERVICE:

            envelope = self._build_msg(method, args=args, kwargs=kwargs, skwargs=skwargs)
            
            cntrl = self.__class__.CNTRL_SERVICE
            cntrl.sendCallToService(self._serviceName, envelope)

        else:
            print('WARNING: No Server Controller available for RPC')


    def _build_msg(self, method, args=None, kwargs=None, skwargs=None):

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

        return envelope
        # print('sending message to service {}'.format(self._serviceName))
        # pprint.pprint(envelope)

        # self._smgr.sendCallToService(self._serviceName, envelope)


    def onMessageReceived(self, msgDict):
        # print('ON MESSAGE RECEIVED!!: {}'.format(msgDict))

        try:
            runId = None
            result = None


            if 'extra' in msgDict:
                runId = msgDict['extra'].get('run_id')
                result = msgDict['extra'].get('result')

                if 'method' in msgDict['extra'] and 'result' in msgDict['extra']:

                    qName = msgDict['extra']['result'].get('queue_name')
                    if qName == self._qname:

                        methodName = msgDict['extra']['method']
                        if methodName == 'queue_ready':
                            self.__ready = True


                        # TODO FIXME
                        import revlens
                        revlens.CNTRL.emitNoticeEvent('ioservice_event', msgDict['extra'])


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



class RlpWsProxyIOServiceClient(RlpIOServiceClient):

    def __init__(self, qname, serviceName='iosched'):
        RlpIOServiceClient.__init__(self, qname, serviceName=serviceName)

        self._wsClient = CoreNet_WebSocketClient()
        self._wsClient.messageReceived.connect(self.onMessageReceived)

        if RlpIOServiceClient.CNTRL_SERVICE:
            RlpIOServiceClient.CNTRL_SERVICE.registerNamedClient(qname, self._wsClient)

        self._wsClient.setUrl('ws://192.168.1.68:8141')


    def _rpc(self, method, args=None, kwargs=None, skwargs=None):

        envelope = self._build_msg(method, args=args, kwargs=kwargs, skwargs=skwargs)

        msgStr = self._wsClient.formatMessage(envelope)
        self._wsClient.sendTextMessage(msgStr)
