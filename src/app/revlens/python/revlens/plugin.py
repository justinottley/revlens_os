'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import logging

from rlp.Qt import QVMap
from rlp import QtCore

import rlp.gui as RlpGui # WARNING: non-gui operation risk

import revlens

rlogger = logging.getLogger('rlp.revlens.plugin')

PLUGIN_EVENT = 0
PLUGIN_MEDIA = 1
PLUGIN_TOOL_GUI = 2
PLUGIN_DISPLAY_CONTEXT_MENU_ITEM = 3
PLUGIN_NODE_FACTORY = 4
PLUGIN_TRACK_FACTORY = 5
PLUGIN_PAINT_ENGINE = 6
PLUGIN_VIDEO_PROBE = 7
PLUGIN_CALLBACK_POSTLOAD = 8
PLUGIN_MEDIA_LOCATOR = 9

# -----

PLUGIN_CALLBACKS = []

def register(plugin_type, plugin_item):

    plugin_manager = revlens.CNTRL.getPluginManager()
    result = False
    
    # braindead switch
    if (plugin_type == PLUGIN_EVENT):
        result = plugin_manager.registerEventPlugin(plugin_item)
        
    elif (plugin_type == PLUGIN_PAINT_ENGINE):
        result = plugin_manager.registerPaintEngine(plugin_item)
        
    elif (plugin_type == PLUGIN_MEDIA):
        result = plugin_manager.registerMediaPlugin(plugin_item)
    
    elif (plugin_type == PLUGIN_MEDIA_LOCATOR):
        result = plugin_manager.registerMediaLocator(plugin_item)

    elif (plugin_type == PLUGIN_NODE_FACTORY):
        result = plugin_manager.registerNodeFactoryPlugin(plugin_item)
        
    elif (plugin_type == PLUGIN_TRACK_FACTORY):
        result = revlens.CNTRL.session().registerTrackFactoryPlugin(
            plugin_item['name'],
            plugin_item['obj']
        )

    elif (plugin_type == PLUGIN_VIDEO_PROBE):
        result = plugin_manager.registerVideoProbe(plugin_item)
        
    elif (plugin_type == PLUGIN_TOOL_GUI):
        plugin_item['item_type'] = 'tool'
        result = RlpGui.VIEW.registerToolGUI(plugin_item)

    elif (plugin_type == PLUGIN_DISPLAY_CONTEXT_MENU_ITEM):
        plugin_item['item_type'] = 'display_context_menu_item'
        revlens.CNTRL.getVideoManager().registerDisplayContextMenuItem(plugin_item)

    elif (plugin_type == PLUGIN_CALLBACK_POSTLOAD):
        
        rlogger.info('Registering plugin postload callback: {c}'.format(
            c=plugin_item))
        
        PLUGIN_CALLBACKS.append(plugin_item)
        
    return result
    
    