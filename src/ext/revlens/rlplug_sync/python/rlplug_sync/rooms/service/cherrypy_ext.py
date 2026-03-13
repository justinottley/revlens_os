
from cherrypy.process import plugins

from ws4py.compat import py3k
from ws4py.manager import WebSocketManager

class RoomWebSocketManager(WebSocketManager):

    def broadcast(self, message, binary=False, room_name=None):

        # print 'broadcast {}'.format(room_name)

        with self.lock:
            websockets = self.websockets.copy()
            if py3k:
                ws_iter = iter(websockets.values())
            else:
                ws_iter = websockets.itervalues()


        for ws in ws_iter:

            # print 'ws: {t} {rn}'.format(t=ws.terminated, rn=ws.room_name)

            if not ws.terminated:
                try:
                    if ((room_name is None) or (room_name == ws.room_name)):
                        ws.send(message, binary)
                except:
                    pass

        


class RoomWebSocketPlugin(plugins.SimplePlugin):
    def __init__(self, bus):
        plugins.SimplePlugin.__init__(self, bus)
        self.manager = RoomWebSocketManager()

    def start(self):
        self.bus.log("Starting WebSocket processing")
        self.bus.subscribe('stop', self.cleanup)
        self.bus.subscribe('handle-websocket', self.handle)
        self.bus.subscribe('websocket-broadcast', self.broadcast)
        self.manager.start()

    def stop(self):
        self.bus.log("Terminating WebSocket processing")
        self.bus.unsubscribe('stop', self.cleanup)
        self.bus.unsubscribe('handle-websocket', self.handle)
        self.bus.unsubscribe('websocket-broadcast', self.broadcast)

    def handle(self, ws_handler, peer_addr):
        """
        Tracks the provided handler.

        :param ws_handler: websocket handler instance
        :param peer_addr: remote peer address for tracing purpose
        """
        self.manager.add(ws_handler)

    def cleanup(self):
        """
        Terminate all connections and clear the pool. Executed when the engine stops.
        """
        self.manager.close_all()
        self.manager.stop()
        self.manager.join()

    def broadcast(self, message, binary=False, room_name=None):
        """
        Broadcasts a message to all connected clients known to
        the server.

        :param message: a message suitable to pass to the send() method
          of the connected handler.
        :param binary: whether or not the message is a binary one
        """
        self.manager.broadcast(message, binary, room_name)
