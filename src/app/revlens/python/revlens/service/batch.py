
from rlp.core.net.websocket import JsonRPCClient

class InteractiveClient(JsonRPCClient):

    _INSTANCE = None

    def __init__(self, port):
        i_url = 'ws://127.0.0.1:{}'.format(port)
        JsonRPCClient.__init__(self, i_url)


    @classmethod
    def instance(cls, port=None):
        if cls._INSTANCE is None:
            assert(port)
            cls._INSTANCE = cls(port)

        return cls._INSTANCE