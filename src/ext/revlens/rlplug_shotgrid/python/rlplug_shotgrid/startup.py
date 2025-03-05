#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

def startup():

    import logging
    import traceback

    import revlens
    import revlens.media
    import revlens.plugin

    import rlplug_shotgrid
    import rlplug_shotgrid.cmds.shot

    from rlplug_shotgrid.web_view import shotgun as web_view_shotgun

    LOG = logging.getLogger('rlp.rlplug_shotgrid')

    def _menu_item_selected(func, check):
        func()

    def _init_context_menu():

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


    web_view_shotgun.init_sg_revlens()


    #
    # Register
    #

    revlens.plugin.register(
        revlens.plugin.PLUGIN_CALLBACK_POSTLOAD,
        _init_context_menu
    )


    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {'name':'ShotGrid',
         'pyitem': 'rlplug_shotgrid.web_view.shotgun.panel'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_DISPLAY_CONTEXT_MENU_ITEM,
        {
            'name': 'SG: Load Previous Shot',
            'callback': ''
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_DISPLAY_CONTEXT_MENU_ITEM,
        {
            'name': 'SG: Load Next Shot',
            'callback': ''
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_DISPLAY_CONTEXT_MENU_ITEM,
        {
            'name': 'SG: Load Surrounding Shot',
            'callback': ''
        }
    )




startup()
