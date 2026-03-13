
import traceback

def _init():

    '''
    import rlp_util.env
    rlp_util.env._init_ldlib_path_key()

    env_dict = {'LD_LIBRARY_PATH': []}
    tb_dir = '/home/justinottley/dev/revlens_root/thirdbase/20_04/Linux-x86_64'

    rlp_util.env.setup_env_pythonqt(env_dict, tb_dir)

    print(env_dict)


    # /home/justinottley/dev/revlens_root/thirdbase/20_04/Linux-x86_64/PythonQt/20.03_Qt_5.14.1_Py_3.7/lib
    # /home/justinottley/dev/revlens_root/thirdbase/20_04/Linux-x86_64/Qt/5.14.1/gcc_64/lib
    # export LD_LIBRARY_PATH=/home/justinottley/dev/revlens_root/thirdbase/20_04/Linux-x86_64/PythonQt/20.03_Qt_5.14.1_Py_3.7/lib:/home/justinottley/dev/revlens_root/thirdbase/20_04/Linux-x86_64/Qt/5.14.1/gcc_64/lib
    os.environ['LD_LIBRARY_PATH'] = env_dict['LD_LIBRARY_PATH'][0]
    '''

    return

    import libRlpEdbDB_PYMODULE

    libRlpEdbDB_PYMODULE.bootstrap()


    from PythonQt.rlp_entitydb import (
        EdbCntrl_Query
    )

    from PythonQt import QtCore

    class_list = [
        EdbCntrl_Query
    ]

    for class_entry in class_list:
        globals()[class_entry.__name__] = class_entry


    # Builtin Qt objects
    #
    globals()['QUuid'] = QtCore.QUuid

try:
    _init()
    del _init
    pass
except:
    print(traceback.format_exc())
    pass