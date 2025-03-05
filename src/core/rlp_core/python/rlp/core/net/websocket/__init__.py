





def _init():
    try:
        import RlpCorePYMODULE
        RlpCorePYMODULE.bootstrap()
    except:
        pass

_init()
del _init


class RlpClient:
    '''
    Instance meant to be populated by client implementations (standalone, wsqt).
    Meant to provide an interface for client code to interact with a client regardless of
    client implementation
    '''

    _INSTANCE = None

    @classmethod
    def instance(cls):
        return cls._INSTANCE