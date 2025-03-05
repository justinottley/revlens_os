

import os
import logging
import traceback

LOG = logging.getLogger('rlp.{}'.format(__name__))

CONFIG_DIR = None


def _init_config_dir():

    rlp_root_dir = os.path.join(os.path.expanduser('~/.config/rlp'))

    if not os.path.isdir(rlp_root_dir):
        LOG.debug('Creating {}'.format(rlp_root_dir))
        os.makedirs(rlp_root_dir)

    global CONFIG_DIR
    CONFIG_DIR = rlp_root_dir

    LOG.debug('Config dir initialized: {}'.format(CONFIG_DIR))


def _init():
    _init_config_dir()



_init()
del _init



