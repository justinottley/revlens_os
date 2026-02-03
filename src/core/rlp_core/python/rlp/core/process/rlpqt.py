
import logging

import rlp.core as RlpCore
from rlp import QtCore
from rlp.Qt import QStr

class RlpQtProcess(RlpCore.PROC_Process):

    def __init__(self):
        RlpCore.PROC_Process.__init__(self)

        self.LOG = logging.getLogger('{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self._listeners = []
        self.outputReady.connect(self._on_output_ready)


    def _on_output_ready(self, line):

        for listener in self._listeners:
            try:
                listener(line)
                
            except Exception as e:
                self.LOG.warning('error running listener {} - {}: {}'.format(
                    listener, e.__class__.__name__, str(e)))


    def add_listener(self, listener):
        self._listeners.append(listener)


    def set_program(self, cmd):
        self.setProgram(cmd)

    def set_args(self, arg_list):
        self.setArguments(arg_list)
