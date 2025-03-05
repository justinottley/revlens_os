import uuid

class RPCFuture(object):

    def __init__(self, client, callback, rpc_method=None, args=None, kwargs=None, persistent=False):

        self.run_id = str(uuid.uuid4())

        self._client = client

        self.callback = callback
        self.rpc_method = rpc_method
        self.args = args
        self.kwargs = kwargs

        self.skwargs = self._client.skwargs
        self.skwargs.update({'run_id': self.run_id})

        self.next = None
        self.prev = None

        self.persistent = persistent
        self.persistent_run_id = None

        self._client._callback_map[self.run_id] = self


    def call(self, callback, rpc_method=None, *args, **kwargs):
        self.next = RPCFuture(self._client, callback, rpc_method, args, kwargs)
        self.next.prev = self

        return self.next


    def done(self, result):

        # print('RPCFUTURE DONE {}'.format(result))
        if self.persistent and self.persistent_run_id is None:
            self.persistent_run_id = result
            self._client.register_callback(self.persistent_run_id, self.callback)
            return

        self.callback(result)

        if not self.persistent:
            del self._client._callback_map[self.run_id]

        if self.next:
            self.next.run()



    def run(self):

        if self.prev:
            prev_future = self.prev
            self.prev = None
            prev_future.run()


        if self.rpc_method:
            self._client._rpc(self.rpc_method, self.args, self.kwargs, self.skwargs)

        else:
            self.callback()
