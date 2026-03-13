'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import traceback
import logging

def load_state():

    from .panel import SettingsController
    _ = SettingsController.instance()



def startup():
    
    import revlens
    import revlens.plugin
    
    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {'name': 'Settings',
         'pywidget': 'rlplug_qtbuiltin.settings.panel.SettingsOptionsPanel'
        })
