
from pybindgen import ReturnValue, Parameter, Module

def get_module():
    
    mod = Module('libRlpRevlensGUI_TL2module')
    mod.add_include('"RlpRevlens/GUI_TL2/MediaTrackFactoryPlugin.h"')
    mod.add_include('"RlpRevlens/GUI_TL2/MediaTrack.h"')
    mod.add_include('"RlpRevlens/GUI_TL2/Track.h"')

    mod.add_class('GUI_TL2_MediaTrack')
    mod.add_class('GUI_TL2_Track')

    mcls = mod.add_class('GUI_TL2_MediaTrackFactoryPlugin')
    
    mcls.add_method(
        'convert',
        ReturnValue.new('GUI_TL2_MediaTrack*', 'result'),
        [Parameter.new('GUI_TL2_Track*', 'track', transfer_ownership=False)],
        is_static=True)
    
    return mod
    
    
