

def startup():
    # print('BINDING WARNING: skipping rlp_prod startup()')
    # return

    import os
    import logging

    import revlens
    import revlens.gui
    import revlens.media
    import revlens.plugin

    import rlp.util as RlpUtil


    LOG = logging.getLogger('revlens.prod')


    def _on_selection_changed(sig_info):
        print('_SELECTION CHANGED {}'.format(sig_info))
        if sig_info['source'] == 'project_chooser':
            revlens.gui.on_layout_load_triggered('default')

        elif sig_info['source'] == 'project_dropdown':
            revlens.gui.on_layout_load_triggered('project_chooser')

    revlens.VIEW.selectionChanged.connect(_on_selection_changed)


    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Browser',
            'pyitem': 'revlens_prod.panel.browser'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Chat',
            'pyitem': 'revlens_prod.panel.chat'
        }
    )

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
            'name': 'Tablet',
            'pyitem': 'revlens_prod.panel.tablet',
            'selectable': False
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Library',
            'pyitem': 'revlens_prod.panel.media'
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
            'name': 'Playlist',
            'pyitem': 'revlens_prod.panel.playlist'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Controls',
            'pyitem': 'revlens_prod.panel.controls'
        }
    )

    LOG.info('revlens.prod - startup done')



startup()