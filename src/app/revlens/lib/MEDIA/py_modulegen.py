
from pybindgen import ReturnValue, Parameter, Module

def get_module():
    
    mod = Module('libRlpRevlensMEDIAmodule')
    mod.add_include('"RlpRevlens/MEDIA/Plugin.h"')
    mod.add_include('"RlpRevlens/MEDIA/Factory.h"')
    
    pcls = mod.add_class('MEDIA_Plugin')
    
    pcls.add_constructor([Parameter.new('const char*', 'name')])
    pcls.add_method('get_name', ReturnValue.new('const char*'), [])
    
    fcls = mod.add_class('MEDIA_Factory')
    fcls.add_constructor([])
    fcls.add_method(
        'registerPlugin',
        ReturnValue.new('bool', 'result'),
        [Parameter.new('MEDIA_Plugin*', 'plugin', transfer_ownership=True)],
        is_static=True)
    
    return mod
    
    
