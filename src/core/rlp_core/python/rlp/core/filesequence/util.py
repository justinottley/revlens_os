import logging

rlogger = logging.getLogger('rlp.{}'.format(__name__))

warning_msg = 'DeprecationWarning: rlp.core.filesequence.util is deprecated, '\
              'use rlp.core.filesequence.cmds instead'

print(warning_msg)
rlogger.info(warning_msg)

from .cmds import *