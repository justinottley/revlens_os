

from pybindgen import ReturnValue, Parameter, Module, FileCodeSink

from scaffold.buildsys.buildlib.PyBindGen import utils

def get_module():
    
    mod = Module('libRlpRevlensRLQPSD_MEDIAmodule')
    mod.add_include('"RlpRevlens/RLQPSD_MEDIA/PyModule.h"')
    
    
    cls = mod.add_class('RLQPSD_MEDIA_PyModule')
    cls.add_method(
        'bootstrap',
        None,
        [],
        is_static=True)
    
    
    return mod
    