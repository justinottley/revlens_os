import traceback
import rlp.core.util

from .style import GUI_Style

GRID_SEL_REPLACE = 0
GRID_SEL_APPEND = 1
GRID_SEL_REMOVE = 2

GRID_VSCROLLBAR_POLICY_AUTO = 0
GRID_VSCROLLBAR_POLICY_PARENT_WIDTH = 1


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


    import RlpGuiPANEmodule
    rlp.core.util.loft_to_module(__name__, RlpGuiPANEmodule)


    # enums from python
    import RlpGuiWIDGETmodule
    RlpGuiWIDGETmodule.GUI_Slider.O_SL_HORIZONTAL = 0
    RlpGuiWIDGETmodule.GUI_Slider.O_SL_VERTICAL = 1


try:
    _init()
    del _init
except:
    print(traceback.format_exc())