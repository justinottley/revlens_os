#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

def startup():

    import os
    import logging
    import traceback

    import rlp.core.util

    import revlens
    import revlens.media
    import revlens.plugin

    import rlplug_shotgrid.sg_handler

    # from rlplug_shotgrid.web_view import shotgun as web_view_shotgun

    LOG = logging.getLogger('rlp.rlplug_shotgrid')

    for module_name in [
        'RlpExtrevlensRLSGCMDmodule',
    ]:
        try:
            lib_mod = __import__(module_name)
            lib_mod.init_module()

            rlp.core.util.loft_to_module('rlplug_shotgrid', lib_mod)

        except:
            print(traceback.format_exc())

    def _menu_item_selected(func, check):
        func()

    def _init_context_menu():
        import rlplug_shotgrid.cmds.shot
        for display_idx in range(revlens.CNTRL.getVideoManager().getDisplayCount()):
            display_entry = revlens.CNTRL.getVideoManager().getDisplayByIdx(display_idx)
            context_menu = display_entry.contextMenu() # rlplug_qtbuiltin.RLQTCNTRL_PLUGIN.contextMenu()

            sg_header = context_menu.addItem("ShotGrid")
            sg_header.setSelectable(False)

            action_load_previous_shot = context_menu.addItem("Load Previous Shot")
            action_load_previous_shot.triggered.connect(
                rlplug_shotgrid.cmds.shot.load_previous_shot_by_current_frame
            )

            action_load_next_shot = context_menu.addItem("Load Next Shot")
            action_load_next_shot.triggered.connect(
                rlplug_shotgrid.cmds.shot.load_next_shot_by_current_frame
            )

            # context_menu.addSeparator()

            action_load_surrounding_shots = context_menu.addItem("Load Surrounding Shots")
            action_load_surrounding_shots.triggered.connect(
                rlplug_shotgrid.cmds.shot.load_surrounding_shots_by_current_frame
            )

            # context_menu.addSeparator()

    try:
        import revlens_prod.site.server
        if revlens_prod.site.server.CNTRL:

            import rlplug_shotgrid.cmds
            rlplug_shotgrid.cmds._init_sg_all()


            # Server init
            #
            sg_find_cmd = rlplug_shotgrid.RLSGCMD_SgFindCommand()
            sg_find_cmd.registerCommand(revlens_prod.site.server.CNTRL)

            sg_custom_cmd = rlplug_shotgrid.RLSGCMD_SgExtensionCommand()
            sg_custom_cmd.registerCommand(revlens_prod.site.server.CNTRL)

            print('Registered SG command endpoint with server controller')

        else:
            # Desktop init
            #
            sg_config_type = os.getenv('RLP_SG_CONFIG_TYPE')
            rlplug_shotgrid.sg_handler.init_sg_revlens(sg_config_type=sg_config_type)


    except:
        print(traceback.format_exc())

    


    #
    # Register
    #

    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_CALLBACK_POSTLOAD,
    #     _init_context_menu
    # )

    # Turn on if not done elsewhere
    #
    # def _onServiceStartupReady(serviceInfo):
    #     if serviceInfo.get('service_name') != 'iosched':
    #         return

    #     envelope = {
    #         'method': 'register_queue',
    #         'kwargs': {
    #             'qname': 'iosg',
    #             'worker_cls': 'rlplug_shotgrid.ioworker.ShotGridIOWorkerThread',
    #             'worker_info': {}
    #         }
    #     }

    #     revlens.CNTRL.getServerManager().sendCallToService('iosched', envelope)


    # revlens.CNTRL.getServerManager().serviceReady.connect(_onServiceStartupReady)

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {'name':'ShotGrid',
         'pyitem': 'rlplug_shotgrid.web_view.panel.main'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'ShotGrid Notes',
            'pyitem': 'rlplug_shotgrid.web_view.panel.notes'
        }
    )
    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_DISPLAY_CONTEXT_MENU_ITEM,
    #     {
    #         'name': 'SG: Load Previous Shot',
    #         'callback': ''
    #     }
    # )

    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_DISPLAY_CONTEXT_MENU_ITEM,
    #     {
    #         'name': 'SG: Load Next Shot',
    #         'callback': ''
    #     }
    # )

    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_DISPLAY_CONTEXT_MENU_ITEM,
    #     {
    #         'name': 'SG: Load Surrounding Shot',
    #         'callback': ''
    #     }
    # )




startup()
