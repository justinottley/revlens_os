

from pybindgen import ReturnValue, Parameter, Module, FileCodeSink, param, retval

from scaffold.buildsys.buildlib.PyBindGen import utils

def get_module():
    
    mod = Module('libRlpRevlensQPSDUTILmodule')
    mod.add_include('"RlpRevlens/QPSDUTIL/QPsdUtil.h"')
    
    cls = mod.add_class('MEDIA_QPsdUtilPyModule')
    
    cls.add_method(
        'convertPsd',
        retval('int'),
        [param('const char*', 'psdPath'),
         param('const char*', 'destPath'),
         param('int', 'width')],
        is_static=True)
    
    return mod
    