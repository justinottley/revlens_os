'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import traceback

def startup():

    import logging
    import traceback

    from rlp import QtCore
    import rlp.core.util

    import revlens
    import revlens.plugin

    import rlplug_sync

    LOG = logging.getLogger('rlp.rlplug_sync')

    LOG.info('startup()')

    try:
        import RlpExtrevlensRLSYNCmodule
        RlpExtrevlensRLSYNCmodule.init_module()
        rlp.core.util.loft_to_module('rlplug_sync', RlpExtrevlensRLSYNCmodule)

    except Exception as e:
        LOG.warning(traceback.format_exc())


    # TODO FIXME: move this?
    #
    def _display_message(evt_info):

        message = evt_info['message']
        duration = evt_info['duration']

        import rlplug_qtbuiltin.cmds
        rlplug_qtbuiltin.cmds.display_message(message, duration)



    # scntrl = SettingsController.instance()
    # sync_settings = scntrl.get_settings(scntrl.SETTINGS_SYNC)
    # if sync_settings:
    #    do_data_sync = sync_settings.get('media_data', True)

    do_data_sync = True
    

    revlens.CNTRL.loginCntrl().loginSuccessful.connect(
        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.initEdbFs
    )

    rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.eventMessage.connect(_display_message)

    #
    # Register
    #


    revlens.plugin.register(
        revlens.plugin.PLUGIN_EVENT,
        rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Rooms',
            'pyitem': 'rlplug_sync.panel.rooms'
        }
    )


    '''
    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Transfers',
            'pyitem': 'rlplug_sync.panel.transfers'
        }
    )
    '''


try:
    startup()
except:
    print(traceback.format_exc())