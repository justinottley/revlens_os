

def startup():
    # print('BINDING WARNING: skipping rlp_prod startup()')
    # return

    import os
    import logging
    import traceback

    import rlp.util as RlpUtil
    import rlp.core.util

    import revlens
    import revlens.gui
    import revlens.media
    import revlens.plugin
    from revlens.keyboard import KeyBinding

    import revlens_prod

    LOG = logging.getLogger('revlens.prod')
    
    try:
        for moduleName in [
            'RlpProdCNTRLmodule'
        ]:
            libMod = __import__(moduleName)
            libMod.init_module()
            rlp.core.util.loft_to_module('revlens_prod', libMod)

    except:
        print(traceback.format_exc())

    try:
        import rlplug_qtbuiltin
        import rlplug_annotate

        revlens.CNTRL.getMediaManager().setTrackTypeIndexOrder(
            [
                revlens.TRACK_TYPE_MEDIA,
                rlplug_qtbuiltin.TRACK_TYPE_ONTHEGO,
                rlplug_annotate.TRACK_TYPE_ANNOTATE
            ]
        )
    except Exception as e:
        print('Warning: Could not set track type index order: {} - {}'.format(e.__class__.__name__, e))


    # def _on_selection_changed(sig_info):
    #     print('_SELECTION CHANGED {}'.format(sig_info))
    #     if sig_info['source'] == 'project_chooser':
    #         revlens.gui.on_layout_load_triggered('default')

    #     elif sig_info['source'] == 'project_dropdown':
    #         revlens.gui.on_layout_load_triggered('project_chooser')

    # revlens.GUI.mainView().selectionChanged.connect(_on_selection_changed)


    def _initStreamVideoSource(toolInfo):

        import revlens_prod

        if toolInfo['name'] == 'Viewer':

            display = toolInfo['tool']

            streamCntrl = revlens_prod.CNTRL_STREAMINGCONTROLLER_OBJ
            streamAnnotationPlugin = streamCntrl.initAnnotationDisplayPlugin()

            streamCntrl.getVideoManager().registerDisplay(display.view(), False)
            display.view().registerEventPlugin(streamAnnotationPlugin)


    revlens.CNTRL.toolCreated.connect(_initStreamVideoSource)


    KeyBinding.register(
        'c',
        'revlens_prod.cmds.clearAnnotationsOnCurrentFrame',
        'clear annotation on current frame')

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Browser',
            'pyitem': 'revlens_prod.panel.browser'
        }
    )

    # Now integrated into Rooms
    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_TOOL_GUI,
    #     {
    #         'name': 'Chat',
    #         'pyitem': 'revlens_prod.panel.chat'
    #     }
    # )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Notes',
            'pyitem': 'revlens_prod.panel.notes'
        }
    )


    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'PySide Example',
            'pyitem': 'revlens_prod.panel.pyside'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Library',
            'pyitem': 'revlens_prod.panel.library.main'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Projects',
            'pyitem': 'revlens_prod.panel.project.chooser'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Session',
            'pyitem': 'revlens_prod.panel.session.main'
        }
    )

    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_TOOL_GUI,
    #     {
    #         'name': 'Controls',
    #         'pyitem': 'revlens_prod.panel.controls.misc'
    #     }
    # )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Desktop Streams',
            'pyitem': 'revlens_prod.panel.streaming'
        }
    )

    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_TOOL_GUI,
    #     {
    #         'name': 'Stream Snapshots',
    #         'pyitem': 'revlens_prod.panel.streaming_snapshot'
    #     }
    # )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Snapshots',
            'pyitem': 'revlens_prod.panel.snapshot'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_VIEW_TOOLBAR,
        {
            'name': 'Playbar',
            'callback_ns': 'revlens.gui.timeline.toolbar',
            'alignment': 'bottom'
        }
    )

    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_VIEW_TOOLBAR,
    #     {
    #         'name': 'Tools',
    #         'callback_ns': 'revlens_prod.panel.viewer.toolbar_tools',
    #         'alignment': 'bottom'
    #     }
    # )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_VIEW_TOOLBAR,
        {
            'name': 'Display Controls',
            'callback_ns': 'revlens_prod.panel.viewer.toolbar_display',
            'alignment': 'top'
        }
    )


    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Tablet',
            'pyitem': 'revlens_prod.panel.tablet.main',
            'selectable': False
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Display Controls',
            'pyitem': 'revlens_prod.panel.tablet.display',
            'selectable': False
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Tablet Stream',
            'pyitem': 'revlens_prod.panel.tablet.stream',
            'selectable': False
        }
    )


    # revlens.plugin.register(
    #     revlens.plugin.PLUGIN_TOOL_GUI,
    #     {
    #         'name': 'ViewerControls',
    #         'pyitem': 'revlens_prod.panel.controls.viewer'
    #     }
    # )

    LOG.info('revlens.prod - startup done')



startup()