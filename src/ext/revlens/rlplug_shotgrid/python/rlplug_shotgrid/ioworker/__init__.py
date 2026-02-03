
import os
import logging

import rlp.core.util
from rlp.core.service.ioscheduler import WorkerThread

import rlplug_shotgrid.sg_handler as sg_handler

from . import playlist, cmds

class ShotGridIOWorkerThread(WorkerThread):

    def __init__(self, *args, **kwargs):
        WorkerThread.__init__(self, *args, **kwargs)

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.sg = None


    def init(self):

        print('IOWorker ShotGrid startup')

        sg_config_type = os.getenv('RLP_SG_CONFIG_TYPE')
        sg_handler.init_sg_revlens(sg_config_type=sg_config_type)


    def dispatch(self, run_id, payload):

        func = eval(payload['method'])

        args = payload['args']
        kwargs = payload['kwargs']
        kwargs['run_id'] = run_id

        result = func(*args, **kwargs)
        
        self.LOG.result(self.LOG.RESULT_OK, run_id, result)

        # if hasattr(sg_cmds, method):

        #     cmd_func = getattr(sg_cmds, method)

        #     args = payload['args']

        #     kwargs = payload['kwargs']
        #     # kwargs['media_callback'] = self.media_callback
        #     result = cmd_func(*args, **kwargs)

        #     self.LOG.result(self.LOG.RESULT_OK, run_id, result)