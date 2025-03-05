#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

from .shotgun.handler import ShotgunHandler

class MediaBrowserHandler(object):

    def __init__(self):
        pass

    def load_entity(self, entity_type, entity_id_list):
        pass


def startup():
    
    import PythonQt
    from PythonQt import QtCore
    
    
    import revlens.plugin
    
    import rlplug_qtbuiltin
    
    env = {}
    
    # rlplug_qtbuiltin.web_view = web_view
    
    mediabrowser_view = PythonQt.rlplug_qtbuiltin.RLQTDISP_WebEngineView(revlens.GUI)
    # mediabrowser_view.setUrl('http://192.168.1.149:8510/rlplug_review')
    mediabrowser_view.setUrl('http://{myip}:8510/rlplug_review'.format(
        myip=env.ip))
    # mediabrowser_view.setWebChannelHandler("lens", "rlplug_qtbuiltin.web_view.media_browser", "MediaBrowserHandler")
    
    mediabrowser_view.setWebChannelPyHandler(
        'revlens_media_browser',
        'rlplug_qtbuiltin.web_view.sg_handler',
        'ShotgunHandler')
    
    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {'name':'Media Browser',
         'gui':[
            {'widget': mediabrowser_view,
             'type':revlens.gui.TOOL_GUI_DOCK,
             'position':QtCore.Qt.RightDockWidgetArea
            }]
        })
    
    