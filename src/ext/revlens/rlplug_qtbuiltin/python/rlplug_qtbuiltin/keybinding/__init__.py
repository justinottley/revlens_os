'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import logging

import revlens.plugin
from revlens.keyboard import KeyBinding

rlogger = logging.getLogger('rlp.rlplug_qtbuiltin.keybinding.panel')



def startup():
    
    rlogger.info('building keyboard bindings..')
    
    KeyBinding.register(
        'left',
        'rlplug_qtbuiltin.cmds.gotoPreviousFrame',
        'goto previous frame')
    
    KeyBinding.register(
        'right',
        'rlplug_qtbuiltin.cmds.gotoNextFrame',
        'goto next frame')
    
    KeyBinding.register(
        'h',
        'rlplug_qtbuiltin.cmds.toggleHud',
        'toggle heads-up display')
    
    KeyBinding.register(
        'i',
        'rlplug_qtbuiltin.cmds.toggleMediaHud',
        'toggle media info'
    )
    
    KeyBinding.register(
        'f',
        'rlplug_qtbuiltin.cmds.toggleFpsHud',
        'toggle fps info'
    )

    KeyBinding.register(
        'space',
        'revlens.cmds.toggle_play_pause',
        'toggle play/pause')
    
    KeyBinding.register(
        'esc',
        'revlens.cmds.exit_presentation_mode',
        'exit presentation mode')
    

    KeyBinding.register(
        '[',
        'revlens.cmds.set_in_frame',
        'set in point')

    KeyBinding.register(
        ']',
        'revlens.cmds.set_out_frame',
        'set out point')

    KeyBinding.register(
        '\\',
        'revlens.cmds.clear_in_out_frames',
        'clear in/out points')

    KeyBinding.register(
        '1',
        'rlplug_qtbuiltin.cmds.resetPanZoom',
        'reset pan/zoom')

    KeyBinding.register(
        'r',
        'revlens.gui.timeline.reset_timeline',
        'Reset Timeline'
    )

    
    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Keybindings',
            'pyitem': 'rlplug_qtbuiltin.keybinding.panel'
        }
    )


