'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''
import traceback

def startup():

    import logging
    import functools
    import traceback

    from rlp.core import rlp_logging
    import rlp.core.util

    rlogger = logging.getLogger('rlp.rlplug_qtbuiltin')
    # rlp_logging.basic_config(rlogger, logging.DEBUG)

    rlogger.info('startup()')

    import revlens
    import revlens.gui

    import rlplug_qtbuiltin


    try:
        rlogger.debug('bootstrapping python bindings')

        import RlpExtrevlensRLQTMEDIAmodule
        RlpExtrevlensRLQTMEDIAmodule.init_module()
        rlp.core.util.loft_to_module('rlplug_qtbuiltin', RlpExtrevlensRLQTMEDIAmodule)

        import RlpExtrevlensRLQTCNTRLmodule
        RlpExtrevlensRLQTCNTRLmodule.init_module()
        rlp.core.util.loft_to_module('rlplug_qtbuiltin', RlpExtrevlensRLQTCNTRLmodule)

        import RlpExtrevlensRLQTDISPmodule
        RlpExtrevlensRLQTDISPmodule.init_module()
        rlp.core.util.loft_to_module('rlplug_qtbuiltin', RlpExtrevlensRLQTDISPmodule)



    except Exception as e:
        rlogger.warning(traceback.format_exc())
        rlogger.warning(e)




    import revlens.gui
    import revlens.plugin

    import rlplug_qtbuiltin


    # export symbols
    # rlplug_qtbuiltin.RLQTMEDIA_PreviewMakerNode = RLQTMEDIA_PreviewMakerNode

    submodules = ['hud', 'fademessage', 'keybinding'] # ['keybinding', 'hud', 'settings', 'fademessage']

    for submodule_name in submodules:

        try:
            rlogger.info('starting submodule "{mname}"'.format(mname=submodule_name))

            builtin_module = rlp.core.util.import_module(
                'rlplug_qtbuiltin.{mod}'.format(mod=submodule_name))

            builtin_module.startup()

        except:
            rlogger.error(traceback.format_exc())


    # import rlplug_qtbuiltin.thumbnail
    # rlplug_qtbuiltin.thumbnail.startup()

    def _menu_item_selected(func, check):
        func()


    def init_connections():

        import rlplug_qtbuiltin

        '''
        rlogger.debug('initializing context menu')

        rlplug_qtbuiltin.RLQTCNTRL_PLUGIN_OBJ.initContextMenu()

        context_menu = rlplug_qtbuiltin.RLQTCNTRL_PLUGIN_OBJ.contextMenu()

        act_fullscreen = context_menu.addAction('Presentation Mode')
        act_fullscreen.triggered.connect(
            functools.partial(
                _menu_item_selected,
                revlens.gui.on_window_presentation_mode_requested
            )
        )

        act_docked = context_menu.addAction('Window Mode')
        act_docked.triggered.connect(
            functools.partial(
                _menu_item_selected,
                revlens.gui.on_window_normal_requested
            )
        )

        context_menu.addSeparator()

        '''
        #import rlplug_qtbuiltin.settings
        #rlplug_qtbuiltin.settings.load_state()



    revlens.plugin.register(
        revlens.plugin.PLUGIN_MEDIA,
        rlplug_qtbuiltin.RLQTMEDIA_PLUGIN_OBJ
    )


    revlens.plugin.register(
        revlens.plugin.PLUGIN_MEDIA,
        rlplug_qtbuiltin.RLQTMEDIA_DEFERREDLOADPLUGIN_OBJ
    )


    revlens.plugin.register(
        revlens.plugin.PLUGIN_EVENT,
        rlplug_qtbuiltin.RLQTDISP_PANZOOM_OBJ
    )


    revlens.plugin.register(
        revlens.plugin.PLUGIN_EVENT,
        rlplug_qtbuiltin.RLQTCNTRL_PLUGIN_OBJ
    )


    revlens.plugin.register(
        revlens.plugin.PLUGIN_EVENT,
        rlplug_qtbuiltin.RLQTDISP_COMPOSITE_OBJ
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_CALLBACK_POSTLOAD,
        init_connections
    )


    rlogger.info('startup complete')


try:
    startup()


except:
    print(traceback.format_exc())
    # rlogger.error(traceback.format_exc())
