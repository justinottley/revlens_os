'''
Useful in cases where python's built-in tempfile module may not be available
'''

import RlpCoreUTILmodule

_utemp = RlpCoreUTILmodule.UTIL_Temp()

def mkdtemp(prefix=''):
    return _utemp.mkdtemp(prefix)

def gettempdir():
    return _utemp.gettempdir()
