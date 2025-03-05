
import os
import sys
import json

import logging
import traceback

from . import config

# Seeded from C++
APPGLOBALS = None
AUTH = None

LOG = logging.getLogger('rlp.util')

def _init():

    import RlpCoreUTILmodule

    global APPGLOBALS
    if hasattr(RlpCoreUTILmodule, 'APPGLOBALS'):
        APPGLOBALS = RlpCoreUTILmodule.APPGLOBALS

    try:
        import tempfile

    except:
        print('monkey patching tempfile module')

        from . import _temp
        sys.modules['tempfile'] = _temp


def get_app_globals():
    global APPGLOBALS
    return APPGLOBALS.globals()

def set_app_globals(new_globals):
    global APPGLOBALS
    APPGLOBALS.setGlobals(new_globals)


def write_login_info(data):

    LOG.debug(data)

    auth_str = AUTH.encrypt(json.dumps(data))

    auth_file_path = os.path.join(config.CONFIG_DIR, 'auth')
    with open(auth_file_path, 'w') as wfh:
        wfh.write(auth_str)


    LOG.debug('Wrote {}'.format(auth_file_path))


def read_login_info():
    
    data = ''
    auth_file_path = os.path.join(config.CONFIG_DIR, 'auth')
    if os.path.isfile(auth_file_path):
        
        try:
            with open(auth_file_path) as fh:
                token = fh.read()
                data = json.loads(AUTH.decrypt(token))

        except:
            LOG.error(traceback.format_exc())

    return data
    

