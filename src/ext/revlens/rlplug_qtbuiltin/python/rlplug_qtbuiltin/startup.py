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

    LOG = logging.getLogger('rlp.rlplug_qtbuiltin')

    LOG.info('startup()')

    import revlens
    import revlens.gui

    import rlplug_qtbuiltin


    try:
        LOG.debug('bootstrapping python bindings')

        for module_name in [
            'RlpExtrevlensRLQTMEDIAmodule',
            'RlpExtrevlensRLQTCNTRLmodule',
            'RlpExtrevlensRLQTDISPmodule',
            'RlpExtrevlensRLQTGUImodule'
        ]:
            lib_mod = __import__(module_name)
            lib_mod.init_module()
            rlp.core.util.loft_to_module('rlplug_qtbuiltin', lib_mod)


    except Exception as e:
        LOG.warning(traceback.format_exc())
        LOG.warning(e)



    import revlens
    import revlens.gui
    import revlens.plugin

    import rlplug_qtbuiltin


    submodules = ['hud', 'fademessage', 'keybinding']

    for submodule_name in submodules:

        try:
            LOG.info('starting submodule "{mname}"'.format(mname=submodule_name))

            builtin_module = rlp.core.util.import_module(
                'rlplug_qtbuiltin.{mod}'.format(mod=submodule_name))

            builtin_module.startup()

        except:
            LOG.error(traceback.format_exc())


    # import rlplug_qtbuiltin.thumbnail
    # rlplug_qtbuiltin.thumbnail.startup()

    def _menu_item_selected(func, check):
        func()


    def init_connections():

        # There are some python binding complications to calling name() on the plugin
        # object, so we use hardcoded value instead
        otgTFPlugin = rlplug_qtbuiltin.RLQTCNTRL_ONTHEGOTRACKFACTORYPLUGIN_OBJ
        revlens.CNTRL.session().registerTrackFactoryPlugin("OnTheGo", otgTFPlugin)

        '''
        LOG.debug('initializing context menu')

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
        rlplug_qtbuiltin.RLQTCNTRL_KEYBINDINGPLUGIN_OBJ
    )


    revlens.plugin.register(
        revlens.plugin.PLUGIN_CALLBACK_POSTLOAD,
        init_connections
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_COMPOSITE_MODE,
        rlplug_qtbuiltin.RLQTGUI_TILECOMPOSITEPLUGIN_OBJ
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_COMPOSITE_MODE,
        rlplug_qtbuiltin.RLQTGUI_SINGLECOMPOSITEPLUGIN_OBJ
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_COMPOSITE_MODE,
        rlplug_qtbuiltin.RLQTGUI_WIPECOMPOSITEPLUGIN_OBJ
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_COMPOSITE_MODE,
        rlplug_qtbuiltin.RLQTGUI_BLENDCOMPOSITEPLUGIN_OBJ
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Preview',
            'pyitem': 'rlplug_qtbuiltin.preview'
        }
    )

    LOG.info('startup complete')


try:
    startup()


except:
    print(traceback.format_exc())
    # LOG.error(traceback.format_exc())
