
import os
import json
import pprint
import base64
import logging
import traceback

import ws4py
from ws4py.websocket import WebSocket
from ws4py.client import WebSocketBaseClient

import rlp.core.path as core_path

import RlpCorePYMODULE


from .. import API_KEY

rlogger = logging.getLogger('rlp.{ns}'.format(ns=__name__))


class WsRpcError(Exception): pass
class RemoteError(WsRpcError): pass
class CommunicationError(WsRpcError): pass
class PermissionDeniedError(WsRpcError): pass
class InvalidMethodError(WsRpcError): pass
class FileNotFoundError(WsRpcError): pass
class TransferNotStartedError(WsRpcError): pass

class JsonWebSocket(WebSocket):
    
    def __init__(self, *args, encrypted=False, **kwargs):
        WebSocket.__init__(self, *args, **kwargs)
    
        self.LOG = logging.getLogger('rlp.{mod}.{cls}'.format(
            mod=self.__class__.__module__, cls=self.__class__.__name__))

        self.app_name = ''
        self.encrypted = encrypted


    def _encrypt(self, msg):
        return RlpCorePYMODULE.CRYPT.encrypt(str(msg))

    def _decrypt(self, msg):
        return RlpCorePYMODULE.CRYPT.decrypt(str(msg))


    def handle_message(self, session_kwargs, method, *args, **kwargs):
        raise NotImplementedError
        
    def received_message(self, message):
        
        try:
            msg_data = message.data
            if self.encrypted:
                msg_data = self._decrypt(msg_data)

            params = json.loads(msg_data)
            
            method = params['method']
            args = params.get('args', [])
            kwargs = params.get('kwargs', {})
            session_kwargs = params.get('session', {})
            
            if session_kwargs['key'] != API_KEY:
                raise PermissionDeniedError('Unable to authenticate message')
                
            self.handle_message(session_kwargs, method, *args, **kwargs)
            
            
        except:
            
            exc_str = traceback.format_exc()

            rlogger.error(exc_str)

            result_str = self.build_return('error', exc_str)
            self.send(result_str, False)


    def build_message(self, method, args=None, kwargs=None, session=None):
        
        if args is None:
            args = {}
            
        if kwargs is None:
            kwargs = {}
            
        if session is None:
            session = {}
        
        # session['key'] = API_KEY
        # session['app_name'] = self.app_name

        msg_dict = {
            'method': method,
            'args': args,
            'kwargs': kwargs,
            'session': session
        }

        msg_str = json.dumps(msg_dict)
        if self.encrypted:
            msg_str = self._encrypt(msg_str)

        return msg_str

    def build_return(self, return_status, result, session_kwargs=None):
        
        session_kwargs = session_kwargs or {}
        # session_kwargs['app_name'] = self.app_name
        
        msg_dict = {
            'status': return_status,
            'result': result,
            'session': session_kwargs
        }
        
        msg_str = json.dumps(msg_dict)
        if self.encrypted:
            msg_str = self._encrypt(msg_str)

        return msg_str


class EchoJsonWebSocket(JsonWebSocket):
    
    def handle_message(self, method, *args, **kwargs):
        
        result = {
            'method': method,
            'args': args,
            'kwargs': kwargs
        }
        
        return result
        

class FileTransferWebSocket(JsonWebSocket):
    
    def __init__(self, *args, **kwargs):
        JsonWebSocket.__init__(self, *args, **kwargs)
        
        self.fd_map = {}
        
        
    def _format_response(self, response, result=None):
        return {'response': response, 'result': result}
        
        
    def _handle_file_download_start(self, file_path):
        
        p = core_path.Path(file_path)
        
        self.LOG.debug('file_download_start(): {p}'.format(p=p))
        
        result = False
        if os.path.isfile(str(p)):
            
            result = True
            
            fh = open(str(p), 'rb')
            
            self.fd_map[fh.fileno()] = {
                'fh': fh,
                'file_path': p,
                'pos': 0
            }
            
            data_response = {
                'fd_fileno': fh.fileno(),
                'file_path': p.asdict()
            }
            
            self.LOG.debug('file_download_start(): {p}: fd_fileno: {fn}'.format(
                p=p, fn=fh.fileno()))
            
            return self._format_response('file_download_start_ok', data_response)
            
            
            
        rlogger.error('file_download_start(): {p} - file not found'.format(
            p=p))
        
        raise FileNotFoundError(str(p))
        
    
    def _handle_file_download_chunk(self, fd_fileno, pos):
        
        if fd_fileno not in self.fd_map:
            raise TransferNotStartedError('file descriptor {fn} not found'.format(
                fn=fd_fileno))
            
        
        fh = self.fd_map[fd_fileno]['fh']
        file_path = self.fd_map[fd_fileno]['file_path']
        
        chunk_size = 8192
        
        if pos != fh.tell():
            self.LOG.debug('{fd} - {p}: seeking to pos {pos}'.format(
                fd=fd_fileno,
                p=file_path,
                pos=pos))
            
            fh.seek(pos)
            
        data = base64.b64encode(fh.read(8192))
        pos = fh.tell()
        self.fd_map[fd_fileno]['pos'] = pos
        
        data_response = {
            'data': data,
            'pos': pos,
            'fd_fileno': fd_fileno
        }
        
        if data == '':
            
            self.LOG.debug('{fd} - {p}: EOF reached, closing file descriptor'.format(
                fd=fd_fileno,
                p=file_path))
            
            fh.close()
            del self.fd_map[fd_fileno]
            
        return self._format_response('file_download_chunk_ok', data_response)
        
        
        
    def _handle_reset_all(self):
        '''
        Utility for cleaning all open file descriptors
        '''
        
        for fd_fileno, fd_info in list(self.fd_map.items()):
            
            self.LOG.debug('Closing file descriptor {fd} - {p}'.format(
                fd=fd_fileno, p=str(fd_info['file_path'])))
            
            fd_info['fh'].close()
            
        self.fd_map = {}
        
        return self._format_response('reset_all_ok')
        
        
    def _handle_get_file_info(self):
        
        # need a serializable representation
        data_result = {}
        for fd_fileno, fd_info in list(self.fd_map.items()):
            data_result[fd_fileno] = {
                'file_path': fd_info['file_path'].asdict(),
                'pos': fd_info['pos']
            }
            
        return self._format_response('get_file_info_ok', data_result)
        
        
    def handle_message(self, method, *args, **kwargs):
        
        # self.LOG.debug('handle_message(): {m}\nARGS:\n{a}\nKWARGS:\n{k}'.format(
        #    m=method, a=pprint.pformat(args), k=pprint.pformat(kwargs)))
        
        
        handler_name = '_handle_{m}'.format(m=method)
        if hasattr(self, handler_name):            
            return getattr(self, handler_name)(*args, **kwargs)
            
        else:
            raise InvalidMethodError(method)


class JsonRPCClient(WebSocketBaseClient):


    def __init__(self, *args, **kwargs):

        encrypted = False
        if 'encrypted' in kwargs:
            encrypted = kwargs.pop('encrypted')
        
        WebSocketBaseClient.__init__(self, *args, **kwargs)

        self.LOG = logging.getLogger('rlp.{mod}.{cls}'.format(
            mod=self.__class__.__module__, cls=self.__class__.__name__))

        self.encrypted = encrypted


    @property
    def connected(self):
        try:
            return self.sock is not None and self.sock.getpeername() is not None
        except:
            return False


    def _encrypt(self, msg):
        return RlpCorePYMODULE.CRYPT.encrypt(str(msg))

    def _decrypt(self, msg):
        return RlpCorePYMODULE.CRYPT.decrypt(str(msg))


    def build_message(self, method, args=None, kwargs=None, session=None):
        
        if args is None:
            args = {}
            
        if kwargs is None:
            kwargs = {}
            
        if session is None:
            session = {}

        session['key'] = API_KEY

        msg_dict = {
            'method': method,
            'args': args,
            'kwargs': kwargs,
            'session': session
        }
        msg_str = json.dumps(msg_dict)
        
        if self.encrypted:
            msg_str = self._encrypt(msg_str)

        return msg_str


class FileTransferClient(WebSocketBaseClient):
    
    _INSTANCES = {}
    
    @classmethod
    def instance(cls, host_info):
        if host_info not in cls._INSTANCES:
            instance = cls(host_info)
            cls._INSTANCES[host_info] = instance
            
        return cls._INSTANCES[host_info]
        
    def __init__(self, *args, **kwargs):
        
        self.LOG = logging.getLogger('{r}.{c}'.format(
            r=rlogger.name, c=self.__class__.__name__))
        
        self.__remote_file_path = None
        
        self.__local_file_path = None
        self.__local_fh = None
        
        self.autoclose = True
        
        WebSocketBaseClient.__init__(self, *args, **kwargs)
        
        
    def _build_message(self, method, args=None, kwargs=None):
        
        if args is None:
            args = []
            
        if kwargs is None:
            kwargs = {}
            
        msg_dict = {
            'method': method,
            'args': args,
            'kwargs': kwargs,
            'session': {
                'key': API_KEY
            }
        }
        msg_str = json.dumps(msg_dict)
        
        return msg_str
        
        
    def set_local_path(self, path):
        self.__local_file_path = core_path.Path(path)
        
    def set_remote_path(self, path):
        self.__remote_file_path = core_path.Path(path)
        
    def handshake_ok(self):
        self.LOG.info("Opening %s" % ws4py.format_addresses(self))
        
    def received_message(self, msg):
        
        # logger.info(str(msg))
        
        try:
            response_envelope = json.loads(str(msg))
            
            # rlogger.debug(pprint.pformat(response_envelope))
            
            if response_envelope['status'] == 'ok':
                
                result = response_envelope['result']
                
                if result['response'] == 'file_download_start_ok':
                    
                    data_response = result['result']
                    
                    rlogger.debug('Opening for write: {lp}'.format(lp=self.__local_file_path))
                    
                    self.__local_fh = open(str(self.__local_file_path), 'wb')
                    
                    fd_fileno = data_response['fd_fileno']
                    self.send(self._build_message('file_download_chunk', [fd_fileno, 0]))
                    
                elif result['response'] == 'file_download_chunk_ok':
                    
                    fd_fileno = result['result']['fd_fileno']
                    pos = result['result']['pos']
                    data = result['result']['data']
                    
                    if data != '':
                        
                        self.__local_fh.write(base64.b64decode(data))
                        self.send(self._build_message('file_download_chunk', [fd_fileno, pos]))
                        
                    else:
                        
                        self.LOG.debug('{p} - EOF reached'.format(p=self.__local_file_path))
                        
                        # Server automatically closes remote file descriptor
                        #
                        self.__local_fh.close()
                        
                        # Transfer done, close socket
                        #
                        if self.autoclose:
                            
                            self.LOG.debug('Closing WebSocket connection')
                            self.close()
                        
                    
                elif result['response'] == 'reset_all_ok':
                    
                    self.LOG.warning('server reset detected, closing connection')
                    self.close()
                    
                
            elif response_envelope['status'] == 'error':
                
                # print result['result']
                # raise RemoteError(result['result'])
                raise RemoteError('Error on peer/remote:\n\n' + response_envelope['result'])
                
            else:
                raise CommunicationError('Unknown return from peer/remote:\n' + pprint.pformat(response_envelope))
                
        except:
            
            rlogger.error(traceback.format_exc())
            self.close()
            
        
    def start(self):
        
        self.connect()
        self.send(self._build_message(
            'file_download_start', [self.__remote_file_path.asdict()]))
        self.run()


class JsonRPCLogHandler(logging.Handler):

    def __init__(self, ws_url, encrypted=False):
        logging.Handler.__init__(self)

        self.client = JsonRPCClient(ws_url, encrypted=encrypted)
        self.client.connect()


    def emit(self, logrecord):

        self.format(logrecord)

        log_payload = {
            'logger': logrecord.name,
            'levelname':logrecord.levelname,
            'levelno':logrecord.levelno,
            'message':logrecord.message
        }

        if hasattr(logrecord, 'extra'):
            log_payload['extra'] = logrecord.extra

        msg_str = json.dumps(
            {
                'method': '__logrecord__',
                'result': log_payload
            }
        )
        if self.client.encrypted:
            msg_str = self.client._encrypt(msg_str)

        self.client.send(msg_str)
