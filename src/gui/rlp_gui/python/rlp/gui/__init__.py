import traceback
import rlp.core.util

VIEW = None

def _init():

    import RlpGuiQTGUI

    from rlp import QtGui

    for lib in [
        'RlpGuiBASEmodule',
        'RlpGuiPAINTERmodule',
        'RlpGuiMENUmodule',
        'RlpGuiGRIDmodule',
        'RlpGuiWIDGETmodule'
    ]:
        lib_mod = __import__(lib)
        lib_mod.init_module()
        rlp.core.util.loft_to_module(__name__, lib_mod)


    global VIEW
    import RlpGuiPANEmodule
    rlp.core.util.loft_to_module(__name__, RlpGuiPANEmodule)

    if hasattr(RlpGuiPANEmodule, 'VIEW'):
        VIEW = RlpGuiPANEmodule.VIEW


try:
    _init()
    del _init
except:
    print(traceback.format_exc())