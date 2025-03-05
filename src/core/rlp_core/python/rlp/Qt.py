

from . import QtCore

def QV(obj_in):
    '''
    Return a QVariant
    '''
    print('WARNING: QV() DEPRECATED')
    return QtCore.PyQVariant(obj_in).val()


def QStr(str_in):
    # print('WARNING: QStr() DEPRECATED')
    return QtCore.QString(str_in)

def QVMap(dict_in):
    print('WARNING: QVMap() DEPRECATED')
    raise Exception('stop')