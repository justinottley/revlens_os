'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import logging
import traceback

LOG = logging.getLogger('rlp.rlplug_annotate')


def startup():

    import os
    import json

    LOG = logging.getLogger('rlp.rlplug_annotate')
    LOG.debug('startup()')

    from rlp import QtCore

    import rlp.core.util

    import revlens
    import revlens.gui
    import revlens.media
    import revlens.plugin

    from revlens.keyboard import KeyBinding

    import rlplug_annotate

    from rlplug_annotate.locator import AnnBackgroundLocator

    for module_name in [
        'RlpExtrevlensRLANN_DSmodule',
        'RlpExtrevlensRLANN_MEDIAmodule',
        'RlpExtrevlensRLANN_MPmodule',
        'RlpExtrevlensRLANN_CNTRLmodule',
        'RlpExtrevlensRLANN_GUImodule'
    ]:
        lib_mod = __import__(module_name)
        lib_mod.init_module()
        rlp.core.util.loft_to_module('rlplug_annotate', lib_mod)
    
    import rlplug_annotate
    import rlplug_annotate.gui

    KeyBinding.register(
        'c',
        'rlplug_annotate.cmds.clearAnnotationOnCurrentFrame',
        'clear annotation on current frame')

    rlplug_annotate.RLANN_MEDIA_PLUGIN_OBJ.setDrawController(
        rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ
    )

    rlplug_annotate.RLANN_MEDIA_NODEFACTORYPLUGIN_OBJ.setDrawController(
        rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ
    )

    rlplug_annotate.RLANN_MEDIA_NODEFACTORYPLUGIN_OBJ.setMediaPlugin(
        rlplug_annotate.RLANN_MEDIA_PLUGIN_OBJ
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_EVENT,
        rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ)


    revlens.plugin.register(
        revlens.plugin.PLUGIN_MEDIA,
        rlplug_annotate.RLANN_MEDIA_PLUGIN_OBJ)
    
    
    revlens.plugin.register(
        revlens.plugin.PLUGIN_NODE_FACTORY,
        rlplug_annotate.RLANN_MEDIA_NODEFACTORYPLUGIN_OBJ)

    revlens.media.LOCATORS.append(AnnBackgroundLocator())


    # read sync settings
    #
    sync_settings_path = os.path.join(revlens._get_state_dir(), 'sync.json')
    if os.path.isfile(sync_settings_path):

        LOG.info('Loading annotate sync settings: {}'.format(sync_settings_path))
        with open(sync_settings_path) as fh:
            sync_settings = json.load(fh)
            if sync_settings.get('annotations'):
                ann_value = sync_settings['annotations']
                ann_value_bool = False
                if ann_value in [1, 2]:
                    ann_value_bool = True

                LOG.info('sync annotations: {}'.format(ann_value_bool))
                
                rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ.setSyncAnnotations(ann_value_bool)


    def _create_track():
        revlens.CNTRL.session().addTrackByType('Annotation')

    def _on_arrangement_loaded():

        m_create_track = 'menu.file.create_track'
        m_create_track_draw = '{}.annotation'.format(m_create_track)
        if m_create_track in revlens.gui._UI and m_create_track_draw not in revlens.gui._UI:
            act_create_annotation_track = revlens.gui._UI[m_create_track].addAction(
                'Annotation')

            act_create_annotation_track.triggered.connect(_create_track)
            revlens.gui._UI['menu.file.create_track.annotation'] = act_create_annotation_track


    revlens.GUI.arrangementLoaded.connect(_on_arrangement_loaded)

    revlens.CNTRL.toolCreated.connect(rlplug_annotate.gui.init_toolbar)


    #
    # GUI Panels
    #
    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Brushes',
            'pyitem': 'rlplug_annotate.gui.brushes'
        }
    )

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {
            'name': 'Annotations',
            'pyitem': 'rlplug_annotate.gui.annotations'
        }
    )

    evt_handler = rlplug_annotate.AnnotateEventHandler.instance()
    evt_handler.init_connections()

    LOG.info('startup(): done')



try:
    startup()

except:
    LOG.error(traceback.format_exc())
