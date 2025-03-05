import os
import sys
import logging

import rlp.core
from rlp.core.net.websocket import JsonRPCClient

import revlens
import rlp.util
from revlens.service.batch import InteractiveClient



app_globals = rlp.util.APPGLOBALS.globals()
startup_args = app_globals['startup.args']
print('rlp_batch_service: {}'.format(startup_args))

interactive_port = startup_args[-1]
my_port = revlens.CNTRL.getServerManager().webSocketServerPort()

print('rlp_batch_service: Interactive port: {} My Port: {}'.format(interactive_port, my_port))

c = InteractiveClient.instance(interactive_port)
c.connect()
c.send(c.build_message('register_service', ['rlp_batch', my_port]))


print('rlp_batch_service: Ready')

# os.system('kill -9 {}'.format(os.getpid()))