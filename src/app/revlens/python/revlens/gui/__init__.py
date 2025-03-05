'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import os
import json
import pprint
import logging
import traceback
import functools
import importlib

import rlp.core.util
from rlp.util import buildopt

from rlp import QtGui

import rlp.util as RlpUtil
import rlp.gui as RlpGui
from rlp.gui.dialogs import InfoDialog, InputDialog

import revlens
import revlens.plugin

from .dialogs import ExportDialog, AboutDialog, SetupDialog
from . import arrangement

LOG = logging.getLogger('rlp.revlens.gui')

TOOL_GUI_DOCK = 0
TOOL_GUI_TOOLBAR = 1

PRESENTATION_WIN = None

# TODO FIXME: improve
_UI = {}

def _init():
    import RlpRevlensGUImodule
    rlp.core.util.loft_to_module(__name__, RlpRevlensGUImodule)

    import RlpRevlensGUI_TL2module
    rlp.core.util.loft_to_module(__name__, RlpRevlensGUI_TL2module)

    if hasattr(RlpRevlensGUI_TL2module, 'TL_CNTRL'):
        revlens.TL_CNTRL = RlpRevlensGUI_TL2module.TL_CNTRL
    else:
        print('WARNING: TL_CNTRL global object not found')

    if hasattr(RlpRevlensGUImodule, 'GUI'):
        revlens.GUI = RlpRevlensGUImodule.GUI
    else:
        print('WARNING: GUI global object not found')

    if hasattr(RlpGui, 'VIEW'):
        revlens.VIEW = RlpGui.VIEW
    else:
        print('WARNING: VIEW global object not found')


_init()
del _init


def _get_layout(name):

    layout_dir = revlens._init_state_dir('layouts')
    layout_filename = 'arrangement_{}.json'.format(name)
    
    layout_path = os.path.join(layout_dir, layout_filename)
    if os.path.isfile(layout_path):
        return layout_path


    for entry in os.getenv('REVLENS_PATH').split(os.pathsep):
        etc_dir = os.path.join(entry, 'etc')
        if not os.path.isdir(etc_dir):
            continue

        layout_path = os.path.join(etc_dir, layout_filename)
        if os.path.isfile(layout_path):
            return layout_path

    print('_get_layout(): ERROR: layout not found: {}'.format(name))


def _get_layouts():

    layout_list = []

    # User - saved ones first
    for file_entry in os.listdir(revlens._init_state_dir('layouts')):
        entry = file_entry.replace('arrangement_', '').replace('.json', '')
        layout_list.append(entry)


    for entry in os.getenv('REVLENS_PATH').split(os.pathsep):
        etc_path = os.path.join(entry, 'etc')
        if not os.path.isdir(etc_path):
            continue

        for file_entry in os.listdir(etc_path):
            if file_entry.startswith('arrangement_'):
                layout_list.append(
                    file_entry.replace('arrangement_', '').replace('.json', ''))


    
    return layout_list


def _restore_window_state_1(descriptor='default'):
    
    
    state_file = _get_layout(descriptor)
    LOG.debug('attempting to restore state: {s}'.format(s=state_file))
    
    try:
        if os.path.isfile(state_file):
            
            with open(state_file, 'rb') as fh:
                revlens.GUI.restoreState(fh.read())
            
                LOG.debug('window state restored')
            
            
        else:
            LOG.debug('skip window state restore, state file not found')
            
            
    except:
        LOG.error('could not restore {f}'.format(f=state_file))
        LOG.error(traceback.format_exc())


def save_window_state():
    
    LOG.info('saving window state')

    def _on_accept(result):
        arrangement.save_arrangement(result['value'])


    panel = InputDialog.create('Save Layout', 'Layout Name:', accept_label='Create')
    panel.accept.connect(_on_accept)


def on_window_presentation_mode_requested():

    global _UI

    presentation_win = revlens.GUI.presentationWindow()
    if presentation_win:

        # Get screen
        for screen_info in _UI['menu.window.screens']:
            if screen_info['act'].isChecked():
                presentation_win.setGeometry(screen_info['screen'].availableGeometry)
                break

        revlens.GUI.enablePresentationMode()



def on_window_normal_requested():

    if revlens.GUI.hasPresentationViewer():
        revlens.GUI.disablePresentationMode()



def on_presentation_device_changed(screen_name):

    global _UI
    LOG.debug('presentation device: {}'.format(screen_name))

    for screen_info in _UI['menu.window.screens']:
        if screen_info['act'].text() == screen_name:
            screen_info['act'].setChecked(True)
        else:
            screen_info['act'].setChecked(False)

    
    presentation_settings_path = os.path.join(revlens._get_state_dir(), 'presentation.json')
    with open(presentation_settings_path, 'w') as wfh:
        wfh.write(json.dumps({'device': screen_name}))

    LOG.debug('Wrote {}'.format(presentation_settings_path))





def register_audio_mixer():

    # from .audio_mixer.widget import AudioMixerPanel

    # panel = AudioMixerPanel
    # revlens.AUDIO_MIXER = panel

    revlens.plugin.register(
        revlens.plugin.PLUGIN_TOOL_GUI,
        {'name': 'Audio Mixer',
         'pywidget': 'revlens.gui.audio_mixer.widget.AudioMixerPanel'
        })


def startup_timeline():

    from . import timeline
    timeline.startup()


def on_new_window_requested():
    revlens.GUI.createNewWindow()


def on_toggle_tab_headers():
    revlens.VIEW.setTabHeadersVisible(not revlens.VIEW.tabHeadersVisible())

    # Terrible hack to redraw the screen, fixes misdrawn viewer and timeline
    revlens.VIEW.resizeView(revlens.VIEW.widthValue(), revlens.VIEW.heightValue() + 1)
    revlens.VIEW.resizeView(revlens.VIEW.widthValue(), revlens.VIEW.heightValue() - 1)
    revlens.VIEW.update()


def on_toggle_splitter_autohide():
    print('toggle splitter autohide')

    revlens.VIEW.setSplittersVisible(not revlens.VIEW.splittersVisible())


def load_session():

    LOG.info('Load session BROKEN')

    print('BINDING WARNING: load_session(): Need load dialog')
    result = None # QtGui.QFileDialog.getOpenFileName(None, 'Load Session')
    if result and result.endswith('.rls'):
        revlens.CNTRL.getMediaManager().loadSessionFile(result)



def save_session():

    def _on_accept(md):
        save_path = md['value']
        print('Saving to {}'.format(save_path))

        dirname, basename = os.path.split(save_path)
        prefix, ext = basename.split(basename)

        if not save_path.endswith('.rls'):
            save_path = '{}.rls'.format(save_path)

        LOG.info('Saving to: {}'.format(save_path))
        revlens.CNTRL.getMediaManager().saveSessionFile(save_path, False)

    diag = InputDialog.create('Save Session', 'Save To:')
    diag.accept.connect(_on_accept)



def save_session_current():

    app_globals = RlpUtil.APPGLOBALS.globals()
    curr_session_path = app_globals.get('session.path')
    if curr_session_path:
        revlens.CNTRL.getMediaManager().saveSessionFile(curr_session_path, True)
        print('Saved {}'.format(curr_session_path))



def export_session():

    diag = ExportDialog.create()
    # diag.accept

def show_about():
    AboutDialog.create()

def show_setup():
    SetupDialog.create()


def show_plugin_ui(md):

    pane_module = md['data']['pymodule']
    if pane_module:

        pane = RlpGui.VIEW.createFloatingPane(400, 300, False)
        pane.setText(md['text'])
        pane.setTextBold(True)

        mod = importlib.import_module(pane_module)
        create_func = getattr(mod, 'create')
        create_func(pane)

    else:
        InfoDialog.create(md['text'], 'No Settings Available')


def show_project_chooser():

    from revlens_prod.panel.project.chooser import ProjectChooserPanel
    ProjectChooserPanel.create_panel()

    # RlpGui.VIEW.emitSelectionChanged(
    #     {
    #         'source': 'project_dropdown',
    #         'data': 'select_project'
    #     }
    # )


def clear_session():
    
    LOG.info('clearing session')
    
    revlens.CNTRL.session().clear('')
    revlens.CNTRL.session().addTrackNoSync('', 'Default')
    # revlens.CNTRL.session().addTrackNoSync('Draw', 'Annotation')




def set_component_pref_movie():

    import rlplug_qtbuiltin.cmds

    global _UI

    factory = revlens.CNTRL.getMediaManager().getMediaFactory()
    factory.setMediaPref('video.component_search_path', ['movie', 'exr_frames', 'frames'])

    _UI['menu.media']['movie'].setChecked(True)
    _UI['menu.media']['exr'].setChecked(False)

    rlplug_qtbuiltin.cmds.display_message('Set Preferred Component: Movie')


def set_component_pref_exr():

    import rlplug_qtbuiltin.cmds

    global _UI

    factory = revlens.CNTRL.getMediaManager().getMediaFactory()
    factory.setMediaPref('video.component_search_path', ['exr_frames', 'frames', 'movie'])

    _UI['menu.media']['movie'].setChecked(False)
    _UI['menu.media']['exr'].setChecked(True)

    rlplug_qtbuiltin.cmds.display_message('Set Preferred Component: EXR')


def add_media_track():
    revlens.CNTRL.session().addTrackAllDefaults()

def goto_start():
    revlens.cmds.goto_frame(revlens.CNTRL.session().inFrame())

def goto_end():
    revlens.cmds.goto_frame(revlens.CNTRL.session().outFrame())

def run_quit():
    revlens.CNTRL.forceQuit()

def on_triggered(func, checked=True):
    func()


def on_layout_load_triggered(layout_name):

    on_layout_clear_triggered()

    arrangement_path = arrangement.get_arrangement_path(layout_name)

    print('Attempting to load: {}'.format(arrangement_path))

    arrangement_data = None
    with open(arrangement_path) as fh:
        arrangement_data = json.load(fh)

    if not arrangement_data:
        LOG.error('Invalid arrangement data - {}'.format(arrangement_path))
        return

    LOG.info('Load layout: {}'.format(layout_name))

    # TEMP TODO FIXME
    # grab the pane in the mainwindow
    #
    if 'window' in arrangement_data and 'wasm' not in buildopt.get_platform():
        window_size = arrangement_data['window']['size']
        revlens.VIEW.resizeView(window_size['width'], window_size['height'])
        mmb = revlens.GUI.mainMenuBar()
        if mmb:
            mmb.setItemWidth(window_size['width'])

    main_pane = revlens.GUI.mainPane()
    main_arr = revlens.GUI.arrangement()
    main_arr.loadArrangement(main_pane, arrangement_data['arrangement'])

    RlpUtil.APPGLOBALS.update({'gui.arrangement': layout_name})


def on_layout_clear_triggered():

    global _UI
    for ui_menu in _UI.get('menu.rmenus', []):
        ui_menu.hideItem()

    # revlens.GUI.mainMenuBar().clear()

    # TODO FIXME: hardcode going straight to the available pane for layout
    main_pane = revlens.GUI.mainPane()
    main_pane.clear()

    main_pane.setHeaderVisible(True, False)

    # clear the timeline view map in the timeline controller
    revlens.TL_CNTRL.clearViews()



def init_menu_default_layout():

    global _UI

    rmenus = _UI.get('menu.rmenus.layout', [])
    if rmenus:

        for menu_entry in rmenus:
            menu_entry.showItem()
        return


    _UI['menu.rmenus.layout'] = []

    menu_file = revlens.GUI.addMenu('File')
    _UI['menu.file'] = menu_file
    _UI['menu.rmenus.layout'].append(menu_file)


    menu_create_track = menu_file.addMenu('Create Track', {})

    act_create_track_media = menu_create_track.addAction('Media')
    act_create_track_media.triggered.connect(functools.partial(on_triggered, add_media_track))

    menu_file.addSeparator()

    act_save = menu_file.addAction('Save')
    act_save.triggered.connect(functools.partial(on_triggered, save_session_current))

    act_save_session = menu_file.addAction('Save Session..')
    act_save_session.triggered.connect(functools.partial(on_triggered, save_session))

    act_load_session = menu_file.addAction('Load Session..')
    act_load_session.triggered.connect(functools.partial(on_triggered, load_session))

    act_clear = menu_file.addAction('Clear Session')
    act_clear.triggered.connect(functools.partial(on_triggered, clear_session))
    
    menu_file.addSeparator()

    act_export = menu_file.addAction('Export..')
    act_export.triggered.connect(functools.partial(on_triggered, export_session))

    menu_file.addSeparator()

    act_quit = menu_file.addAction('Quit')
    # act_quit.setShortcuts(PythonQt.Qt.QKeySequence.Quit)
    act_quit.setStatusTip('Quit')
    act_quit.triggered.connect(functools.partial(on_triggered, run_quit))
    
    _UI['menu.rmenus.layout'].append(menu_file)
    _UI['menu.file'] = menu_file
    _UI['menu.file.create_track'] = menu_create_track
    _UI['menu.file.create_track.media'] = act_create_track_media
    _UI['menu.file.save_session'] = act_save_session
    _UI['menu.file.load_session'] = act_load_session
    _UI['menu.file.clear_session'] = act_clear
    _UI['menu.file.export'] = act_export
    _UI['menu.file.quit'] = act_quit


    
    # -------


    menu_control = revlens.GUI.addMenu("Control")
    act_play = menu_control.addAction("Play")

    def _on_play():
        revlens.CNTRL.play(True)

    def _on_pause():
        revlens.CNTRL.pause(-1, True)

    act_play.triggered.connect(_on_play)
    
    act_pause = menu_control.addAction("Pause")
    act_pause.triggered.connect(_on_pause)
    
    act_goto_start = menu_control.addAction("Goto Start")
    act_goto_start.triggered.connect(functools.partial(on_triggered, goto_start))
    
    act_goto_end = menu_control.addAction("Goto End")
    act_goto_end.triggered.connect(functools.partial(on_triggered, goto_end))
    
    _UI['menu.rmenus.layout'].append(menu_control)
    _UI['menu.control'] = menu_control
    _UI['menu.control.play'] = act_play
    _UI['menu.control.pause'] = act_pause
    _UI['menu.control.goto_start'] = act_goto_start
    _UI['menu.control.goto_end'] = act_goto_end

    '''
    
    media_menu = revlens.GUI.addMenu('Media')

    movie_action = media_menu.addAction('Movie')
    movie_action.setCheckable(True)
    movie_action.setChecked(True)
    movie_action.triggered.connect(functools.partial(on_triggered, set_component_pref_movie))

    exr_action = media_menu.addAction('EXR')
    exr_action.setCheckable(True)
    exr_action.triggered.connect(functools.partial(on_triggered, set_component_pref_exr))

    _UI['menu.media'] = {
        'movie': movie_action,
        'exr': exr_action
    }

    '''

    menu_window = revlens.GUI.addMenu('Window')


    menu_presentation_device = menu_window.addMenu('Presentation Device', {})

    _UI['menu.window.screens'] = []

    # load presentation mode saved setting
    #
    presentation_settings = {}
    presentation_settings_path = os.path.join(revlens._get_state_dir(), 'presentation.json')
    if os.path.isfile(presentation_settings_path):
        with open(presentation_settings_path) as fh:
            presentation_settings = json.load(fh)


    print('BINDING WARNING: screen_list[] bypassed')
    screen_list = [] # QtGui.QApplication.screens()
    for screen_entry in screen_list:
        act_screen = menu_presentation_device.addAction(screen_entry.name)
        act_screen.setCheckable(True)
        act_screen.triggered.connect(functools.partial(
            on_presentation_device_changed,
            screen_entry.name
        ))

        _UI['menu.window.screens'].append({
            'act': act_screen,
            'screen': screen_entry
        })

    
    presentation_device_found = False
    if presentation_settings.get('device'):
        presentation_device = presentation_settings['device']
        for screen_entry in _UI['menu.window.screens']:
            if screen_entry['screen'].name == presentation_device:

                LOG.info('Setting presentation device: "{}"'.format(presentation_device))
                screen_entry['act'].setChecked(True)
                presentation_device_found = True

    if not presentation_device_found:
        LOG.warning('could not reload presentation device, setting first available')
        # _UI['menu.window.screens'][0]['act'].setChecked(True)

    act_presentation_mode = menu_window.addAction('Presentation Mode')
    act_presentation_mode.triggered.connect(
        functools.partial(
            on_triggered,
            on_window_presentation_mode_requested
        )
    )

    act_docked = menu_window.addAction('Window Mode')
    act_docked.triggered.connect(
        functools.partial(
            on_triggered,
            on_window_normal_requested
        )
    )


    menu_window.addSeparator()

    menu_load_state = menu_window.addMenu(
        'Load Layout', {})

    layout_list = _get_layouts()
    for layout_entry in layout_list:

        act_layout = menu_load_state.addAction(layout_entry)
        act_layout.triggered.connect(
            functools.partial(
                on_layout_load_triggered,
                layout_entry
            )
        )

        ui_ns = 'menu.window.load_layout.{}'.format(layout_entry)
        _UI[ui_ns] = act_layout

    act_save_state = menu_window.addAction('Save Layout..')
    act_save_state.triggered.connect(functools.partial(on_triggered, save_window_state))

    act_clear_layout = menu_window.addAction('Clear Layout')
    act_clear_layout.triggered.connect(functools.partial(on_triggered, on_layout_clear_triggered))

    menu_window.addSeparator()

    act_new_win = menu_window.addAction('New Window')
    act_new_win.triggered.connect(on_new_window_requested)
    
    menu_window.addSeparator()

    act_tab_headers = menu_window.addAction('Toggle Tab Headers')
    act_tab_headers.triggered.connect(on_toggle_tab_headers)

    act_splitter_vis = menu_window.addAction('Toggle Splitter Autohide')
    act_splitter_vis.triggered.connect(on_toggle_splitter_autohide)
    '''
    act_single_win = menu_window.addAction("Single Window")
    act_single_win.triggered.connect(functools.partial(on_triggered, single_window))
    
    act_split_win = menu_window.addAction("Split Window")
    act_split_win.triggered.connect(functools.partial(on_triggered, split_window))
    '''
    
    _UI['menu.rmenus.layout'].append(menu_window)
    _UI['menu.window'] = menu_window
    _UI['menu.window.pd'] = menu_presentation_device
    _UI['menu.window.presentation_mode'] = act_presentation_mode
    _UI['menu.window.window_mode'] = act_docked
    _UI['menu.window.load_layout'] = menu_load_state
    _UI['menu.window.save_layout'] = act_save_state
    _UI['menu.window.clear_layout'] = act_clear_layout
    _UI['menu.window.new_window'] = act_new_win
    _UI['menu.window.tab_headers'] = act_tab_headers
    _UI['menu.window.splitter_vis'] = act_splitter_vis



def init_menu_global():
    
    global _UI

    for lmenu in _UI.get('menu.rmenus.layout', []):
        lmenu.hideItem()

    rmenus = _UI.get('menu.rmenus', [])
    if rmenus:

        init_menu_project_list()

        for menu_entry in rmenus:
            menu_entry.showItem()
        return


    _UI['menu.rmenus'] = []

    menu_bar = revlens.GUI.mainMenuBar()
    if not menu_bar:
        print('No Main Menu Bar, skipping menubar populate')
        return

    menu_bar.addSpacer(10)
    
    revlens_icon = RlpGui.GUI_IconButton(':misc/revlens_r_bw_small.png', menu_bar, 20, 0)
    revlens_icon.setIconYOffset(3)
    revlens_icon.setIconHOffset(3)

    revlens_menu = revlens_icon.menu()
    switch_project = revlens_menu.addItem('Switch Project..', {}, False)
    switch_project.triggered.connect(show_project_chooser)

    about_item = revlens_menu.addItem('About..', {}, False)
    about_item.triggered.connect(show_about)

    setup_item = revlens_menu.addItem('Setup..', {}, False)
    setup_item.triggered.connect(show_setup)

    revlens_menu.addItem('', {}, False)

    plugins_item = revlens_menu.addItem('Plugins', {}, False)
    plugins_item.setSelectable(False)

    for plugin in revlens.CNTRL.getPluginManager().getMediaPlugins():
        plugin_ui = plugin.getSettingsUI()
        pymodule = ''
        if plugin_ui:
            pymodule = plugin_ui

        plugin_item = revlens_menu.addItem(
            plugin.name(),
            {
                'plugin_type': 'media',
                'pymodule': plugin_ui
            },
            False
        )
        plugin_item.menuItemSelected.connect(show_plugin_ui)
        _UI['menu.revlens.plugin.{}'.format(plugin.name())] = plugin_item

    revlens_menu.addSeparator()

    for eplugin in sorted(revlens.CNTRL.getEventPluginNames()):
        plugin = revlens.CNTRL.getEventPluginByName(eplugin)
        plugin_ui = plugin.getSettingsUI()
        plugin_item = revlens_menu.addItem(
            eplugin,
            {
                'plugin_type': 'event',
                'pymodule': plugin_ui
            },
            False
        )
        plugin_item.menuItemSelected.connect(show_plugin_ui)
        _UI['menu.revlens.plugin.event.{}'.format(eplugin)] = plugin_item

    menu_bar.addMenuButton(revlens_icon)
    
    _UI['menu.rmenus'].append(revlens_icon)
    _UI['menu.revlens'] = revlens_icon
    _UI['menu.revlens.about'] = about_item
    _UI['menu.revlens._plugins'] = plugins_item


    init_menu_project_list()

    #
    # --
    #

    # help_menu = revlens.GUI.mainMenuBar().addRightMenu('Help')
    # about_action = help_menu.addAction('About')
    # about_action.triggered.connect(show_about)
    # 
    # _UI['menu.help'] = help_menu
    # _UI['menu.help.about'] = about_action


def init_menu_project_list():

    global _UI

    ag = RlpUtil.get_app_globals()

    if _UI.get('menu.projlist'):

        proj_list = _UI['menu.projlist']
        if ag.get('project.name'):
            proj_list.setText(ag['project.name'])
            proj_list.showItem()

        return

    menu_bar = revlens.GUI.mainMenuBar()
    menu_bar.addSpacer(10)


    proj_list = RlpGui.GUI_IconButton('', menu_bar, 20, 0)
    proj_list.setSvgIconRight(False)
    proj_list.setSvgIconPath(':feather/lightgrey/package.svg', 20, True)
    proj_list.setText(ag.get('project.name', ''))
    proj_list.setTextYOffset(1)
    proj_list_menu = proj_list.menu()
    
    def _menu_item_selected(md):
        if md['data']['name'] == 'new_project':
            RlpGui.VIEW.emitSelectionChanged(
                {
                    'source': 'project_dropdown',
                    'data': 'select_project'
                }
            )


    proj_list_menu.menuItemSelected.connect(_menu_item_selected)

    icon = proj_list.svgIcon()
    icon.setPos(2, 3)
    icon.setBgColour(QtGui.QColor(140, 140, 140))
    icon.setFgColour(QtGui.QColor(140, 140, 140))

    _UI['menu.projlist'] = proj_list

    proj_list_pages = proj_list_menu.addItem('Layouts', {}, False)

    layout_list = _get_layouts()
    for layout_entry in layout_list:

        # HACK
        if 'project_chooser' in layout_entry:
            continue

        # act_layout = menu_load_state.addAction(layout_entry)
        act_layout = proj_list_pages.addItem(layout_entry, {})
        act_layout.triggered.connect(
            functools.partial(
                on_layout_load_triggered,
                layout_entry
            )
        )

        ui_ns = 'menu.projlist.layout.load_layout.{}'.format(layout_entry)
        _UI[ui_ns] = act_layout



    menu_bar.addMenuButton(proj_list)
    menu_bar.addSpacer(10)


    # TODO FIXME HACK
    try:
        import rlp_prod.panel.user
        rlp_prod.panel.user.init_user_menu()
    except:
        print('Warning: Could not init user menu')

    appCtxLabel = RlpGui.GUI_Label(menu_bar, '')
    af = appCtxLabel.font()
    af.setBold(True)
    appCtxLabel.setFont(af)
    appCtxLabel.setBgColor(QtGui.QColor(60, 60, 60))
    appCtxLabel.hideItem()

    menu_bar.addRightItem(appCtxLabel)

    _UI['menu.rmenus.appctx_label'] = appCtxLabel


    if not ag.get('project.name'):
        proj_list.setWidth(0)
        proj_list.hideItem()


def login_done(result):

    LOG.info('revlens: login done: {}'.format(result))

    if not result.get('connect_state') or result.get('connect_state') != 'OK':
        revlens.CNTRL.loginCntrl().emitLoginFailed()
        return


    from rlp.core.net.websocket import RlpClient

    rlpClient = RlpClient.instance()

    if rlpClient is None:
        print('WARNING: no Site Client instance!')
        return


    LOG.info('revlens: login done on "{}" - {} url: "{}" site_name: {}'.format(
        rlpClient.url,
        rlpClient,
        rlpClient._wsclient.url().toString().toStdString(),
        rlpClient.site_name
    ))

    # TODO FIXME
    # ???? WHY IS THIS NOT SET FOR WASM?
    #
    # ws_client_url = rlpClient._wsclient.url().toString()
    # if not ws_client_url:
    #     rlpClient._wsclient.setUrl(SiteWsClient.instance().url)

    lresult = {'result': False}
    if rlpClient and rlpClient.auth_key:
        lresult = rlpClient.info
        lresult['result'] = True

    revlens.CNTRL.getServerManager().setWebSocketClient(
        rlpClient._wsclient)
    revlens.CNTRL.loginCntrl().loginDone(lresult)


    def _handle_server_error(errInfo):
        LOG.error('Server Error: {}'.format(errInfo))
        InfoDialog.create('Server Error', errInfo['err_msg'])
        

    rlpClient._wsclient.errorReceived.connect(_handle_server_error)
    if hasattr(rlpClient, 'rfsc'):
        rlpClient.rfsc._wsclient.errorReceived.connect(_handle_server_error)


def show_load_dialog(sel_data):

    from .dialogs import LoadDialog

    media_uuid = sel_data.get('uuid')
    local_path = sel_data.get('local_path')
    title = os.path.basename(sel_data.get('title', 'unknown'))

    if not local_path and not media_uuid:
        LOG.error('NO path, cannot load')
        print('NO PATH, cannot load')
        return

    LOG.info('Loading {}'.format(local_path))

    media_info = {
        'path': local_path
    }

    if media_uuid:
        media_info['graph.uuid'] = media_uuid

    if 'thumb_frame_data' in sel_data:
        media_info['media.thumbnail.data'] = sel_data['thumb_frame_data']


    # pprint.pprint(media_info)

    floating_pane = revlens.VIEW.createFloatingPane(400, 280, False)
    diag = LoadDialog(title, media_info, floating_pane)

    if 'dialogs' not in _UI:
        _UI['dialogs'] = []

    _UI['dialogs'].append(diag)



def startup():

    init_menu_global()
    startup_timeline()

    print('Done')

