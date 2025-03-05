#!/usr/bin/env python3
import os
import traceback

os.environ['RLP_IO_SERVICE'] = '1'
os.environ['RLP_LOGGING_SUFFIX'] = '.ioservice.log'

def _init_logging():

    def ioprint(msg, *args, **kwargs):

        stack = traceback.extract_stack()[:-1]
        last = stack[-1]

        module = last.filename
        if 'site-packages' in module:
            module = module.split('site-packages')[1].replace('/', '.').replace('.py', '')[1:]
        else:
            module = os.path.split(module)[-1]

        msg = '[IOService] : {}:{} - {}'.format(module, last.lineno, msg)
        __builtins__._print(msg)


    __builtins__._print = __builtins__.print
    __builtins__.print = ioprint


_init_logging()

print('rlp_io_service: starting up')

from rlp.core import rlp_logging
from rlp.core.service import ioservice

if __name__ == '__main__':
    ioservice.main()
