
import os
import sys
import logging
import traceback

import rlp.util
from rlp.util import buildopt

LOG = logging.getLogger(__name__)

def _init_plugins(env_var_name='REVLENS_PATH', prefix_keys=None):

    if not prefix_keys:
        prefix_keys = ['rlp']

    if not os.getenv(env_var_name):
        LOG.warning('{} not found, cannot load plugins!'.format(env_var_name))
        os.environ[env_var_name] = ''

    _seen_paths = []
    for entry in os.getenv(env_var_name).split(os.pathsep):

        if entry in _seen_paths:
            continue

        _seen_paths.append(entry)

        python_path = entry
        if 'wasm' not in buildopt.get_platform():
            python_path = os.path.join(
                entry,
                'lib',
                buildopt.get_python_dirname(),
                'site-packages')

        if not os.path.isdir(python_path):
            print('Skipping {}, not a directory'.format(python_path))
            continue

        print('Scanning "{}" for plugins..'.format(python_path))

        for dir_entry in sorted(os.listdir(python_path)):

            pydir = os.path.join(python_path, dir_entry)
            if os.path.isdir(pydir):

                # check that dir starts with one of the provided prefix keys
                if not any(dir_entry.startswith(key) for key in prefix_keys):
                    continue

                plugin_startup_path = os.path.join(pydir, 'startup.py')

                if not os.path.isfile(plugin_startup_path):
                    print('Skipping, not found: {}'.format(plugin_startup_path))
                    continue

                print('attempting to execute: {}'.format(plugin_startup_path))

                try:
                    exec(open(plugin_startup_path).read())
                except:

                    LOG.error('Error starting plugin "{plugname}"'.format(
                        plugname=dir_entry))

                    for line in traceback.format_exc().split('\n'):
                        LOG.error(line)

def _init():

    print('rlp.util._init()')

    from . import util

    for lib in [
        'RlpCoreDSmodule',
        'RlpCoreUTILmodule',
        'RlpCoreSEQmodule',
        'RlpCoreNETmodule',
        'RlpCorePROCmodule'
    ]:
        try:
            lib_mod = __import__(lib)
            lib_mod.init_module()
            util.loft_to_module(__name__, lib_mod)
        except Exception as e:
            print('Unable to import module {} - {}'.format(lib, e))


    import RlpCoreCNTRLmodule
    RlpCoreCNTRLmodule.init_module()
    util.loft_to_module(__name__, RlpCoreCNTRLmodule)

    import rlp.util
    rlp.util.AUTH = RlpCoreCNTRLmodule.CORECNTRL_AUTH_OBJ


try:
    _init()
    del _init
    print('rlp.util._init() done')
except:
    print(traceback.format_exc())
