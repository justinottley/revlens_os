'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''


from pybindgen import ReturnValue, Parameter, Module, FileCodeSink

from scaffold.buildsys.buildlib.PyBindGen import utils

def get_module():
    
    mod = Module('libRlpExtrevlensRLANN_GUImodule')
    mod.add_include('"RlpExtrevlens/RLANN_GUI/PyModule.h"')
    
    
    cls = mod.add_class('RLANN_GUI_PyModule')
    cls.add_method(
        'bootstrap',
        None,
        [],
        is_static=True)
    
    
    return mod
    