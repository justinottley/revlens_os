'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import os
import sys
import json
import pprint
import logging
import argparse
import traceback

import rlp.core as RlpCore
from rlp.core.net.websocket import RlpClient

import rlp.util as RlpUtil
from rlp.util import buildopt
from rlp.util import env as rlp_util_env
from rlp.util import config as rlp_util_config



RlpUtil._init()

PLATFORM = buildopt.get_platform()

LOG = logging.getLogger('rlp.revlens')

# Seeded objects from C++
GUI = None
CNTRL = None
TL_CNTRL = None


def _init():

    print('_init()')

    global CNTRL, TL_CNTRL, GUI

    import rlp.core.util


    for lib in [
        'RlpRevlensDSmodule',
        'RlpRevlensMEDIAmodule',
        'RlpRevlensAUDIOmodule',
        'RlpRevlensVIDEOmodule',
        'RlpRevlensDISPmodule',
        'RlpRevlensCNTRLmodule'
    ]:
        lib_mod = __import__(lib)
        lib_mod.init_module()
        rlp.core.util.loft_to_module(__name__, lib_mod)


    import revlens.gui
    for moduleName in [
        'GUI',
        'GUI_TL2',
        'GUI_TLT',
        'GUI_VIEW'
    ]:
        modFullName = 'RlpRevlens{}module'.format(moduleName)
        libMod = __import__(modFullName)
        libMod.init_module()
        rlp.core.util.loft_to_module('revlens.gui', libMod)


def _init_binding_objects():

    global CNTRL, GUI, TL_CNTRL

    import RlpRevlensCNTRLmodule
    CNTRL = RlpRevlensCNTRLmodule.CNTRL

    import RlpRevlensGUImodule
    if hasattr(RlpRevlensGUImodule, 'GUI'):
        GUI = RlpRevlensGUImodule.GUI

    else:
        print('WARNING: GUI global object not found')

    import RlpRevlensGUI_TL2module
    if hasattr(RlpRevlensGUI_TL2module, 'TL_CNTRL'):
        TL_CNTRL = RlpRevlensGUI_TL2module.TL_CNTRL

    else:
        print('WARNING: TL_CNTRL global object not found')


    import RlpCoreUTILmodule

    if hasattr(RlpCoreUTILmodule, 'APPGLOBALS'):
        RlpUtil.APPGLOBALS = RlpCoreUTILmodule.APPGLOBALS
    else:
        print('WARNING: APPGLOBALS global object not found')


LOG = logging.getLogger('rlp.revlens')

# For linter - these are singletons that are exposed from C++ at startup
#
# CNTRL = None
GUI = None
TIMELINE = None
TOOLBAR = None
#
# ------


RLPLUG_KEY = 'rlp'
REVLENS_KEY = 'revlens'

PLAYSTATE_UNKNOWN = 0
PLAYSTATE_PLAYING = 1
PLAYSTATE_PAUSED = 2

# DS_PlaybackMode::DirectionMode enum
#
DIRMODE_FORWARD = 0
DIRMODE_BACKWARD = 1

INCMODE_INC = 0
INCMODE_DEC = 1

SESSION_INDEX_POLICY_TRACK_IDX_AS_IS = 0
SESSION_INDEX_POLICY_TRACK_IDX_BY_TYPE = 1

TRACK_TYPE_MEDIA = 0

NODE_DATA_VIDEO = 0
NODE_DATA_AUDIO = 1
NODE_DATA_TEXT = 2
NODE_DATA_LOAD = 3
NODE_DATA_UNKNOWN = 4

def _get_initial_env():

    appname = os.getenv('REVLENS_APP_NAME', 'revlens')

    username = os.getenv('USER', os.getenv('USERNAME'))

    try:
        import socket
        hostname = socket.gethostname()

    except:
        hostname = 'unknown'

    try:
        ip = socket.gethostbyname(hostname)

    except Exception as e:
        ip = '127.0.0.1'

    return {
        'appname': appname,
        'username': username,
        'hostname': hostname,
        'ip': ip
    }


def _get_state_dir():
    return rlp_util_config.CONFIG_DIR


def _init_state_dir(subdir=''):

    state_dir = os.path.join(_get_state_dir(), subdir)

    if not os.path.isdir(state_dir):
        LOG.debug('Attempting to create state dir: {d}'.format(d=state_dir))
        os.makedirs(state_dir)

    return state_dir

def _init_locators():

    from . import media
    media._init_locators()


def _init_prefs():

    factory = CNTRL.getMediaManager().getMediaFactory()
    factory.setMediaPref('video.component_search_path', ['movie', 'exr_frames', 'frames'])
    factory.setMediaPref('audio.component_search_path', ['movie', 'audio_file'])


def _init_session(media=True, annotation=True):

    LOG.info('setting default session')

    # For signal propagation
    #
    CNTRL.setSession("Default")

    session = CNTRL.sessionPtr()

    if media and not session.hasTrackType(TRACK_TYPE_MEDIA):
        session.addTrackAllDefaults()

    # TODO FIXME: install Annotation track only if annotation plugin is loaded

    if annotation:
        try:
            import rlplug_annotate
            if not session.hasTrackType(rlplug_annotate.TRACK_TYPE_ANNOTATE):
                track = session.addTrackByType('Annotation')
                rlplug_annotate.RLANN_CNTRL_DRAWCONTROLLER_OBJ.setActiveTrack(track.uuid().toString())

        except:
            print(traceback.format_exc())


def _init_keyboard():

    LOG.info('registering keyboard mappings')

    # NOTE: late import
    from .keyboard import key_default
    key_default.register_keys()


def _init_plugins():

    RlpCore._init_plugins(prefix_keys=[RLPLUG_KEY, REVLENS_KEY])

    # Now that all the plugins have run startup() and have been registered /
    # made themselves visible, run the plugin postload callbacks
    #
    # NOTE: late import
    from . import plugin

    for plugin_callback in plugin.PLUGIN_CALLBACKS:

        try:
            LOG.debug('Attempting to run plugin postload callback: {c}'.format(
                c=plugin_callback))

            plugin_callback()

        except:
            LOG.error(traceback.format_exc())



def _init_gui(init_session_media_track=True, init_session_annotation_track=True):

    LOG.debug('Initializing GUI')
    from .gui import timeline

    timeline.startup()

    

    print('Gui startup done')


def _init_cmdline_options():

    app_globals = RlpUtil.APPGLOBALS.globals()
    cmd_args = list(app_globals['startup.args'])[1:]

    parser = argparse.ArgumentParser('revlens')
    parser.add_argument('--batch', dest='batch', action='store_true', help='batch mode')
    parser.add_argument('--service', dest='service', action='store_true', help='service mode, dont start subprocess services')
    parser.add_argument('--script', dest='script', default="")
    parser.add_argument('--noaudio', dest='noaudio', action='store_true', default=False, help='do not run audio engine')
    parser.add_argument('--gotoFrame', dest='goto_frame', help='go to a specific frame', default=0)
    parser.add_argument('--layout', dest='arrangement', default=None, help='pane arrangement to load')
    parser.add_argument('--arrangement', dest='arrangement', default=None, help='pane arrangement to load (same as --layout)')
    parser.add_argument('--sg', dest='sg_config_type', default=None, help='Shotgun config type')
    parser.add_argument('--auth', default='auth')
    parser.add_argument('media', nargs='*')

    parser_args = parser.parse_args(cmd_args)

    LOG.debug('app_globals: {} cmd_args: {} parser_args: {}'.format(
        app_globals, cmd_args, parser_args))

    if parser_args.batch:
        app_globals['startup.mode.batch'] = True

    if parser_args.service:
        app_globals['startup.mode.service'] = True

    else:
        pass
        # CNTRL.getServerManager().startIOService()
        # CNTRL.getServerManager().startBatchService()

    app_globals['app.config_dir'] = _get_state_dir()
    app_globals['startup.auth'] = parser_args.auth
    app_globals['startup.script'] = parser_args.script
    app_globals['startup.media'] = parser_args.media
    app_globals['startup.goto_frame'] = parser_args.goto_frame
    app_globals['startup.audio_enabled'] = not parser_args.noaudio
    if parser_args.arrangement:
        app_globals['startup.arrangement'] = parser_args.arrangement

    if parser_args.sg_config_type:
        os.environ['RLP_SG_CONFIG_TYPE'] = parser_args.sg_config_type
        app_globals['startup.sg_config_type'] = parser_args.sg_config_type


    print('---------------')
    print(pprint.pformat(app_globals))
    print('---------------')

    RlpUtil.APPGLOBALS.setGlobals(app_globals)


def _init_startup_context():

    context_path = os.path.join(_get_state_dir(), 'appcontext.json')
    if os.path.isfile(context_path):
        with open(context_path) as fh:
            try:
                app_context = json.load(fh)
                if app_context.get('project.name'):

                    RlpUtil.APPGLOBALS.update({
                        'project.name': app_context['project.name'],
                        'project.info': app_context['project.info']
                    })

            except:
                print('Error loading app context')
                print(traceback.format_exc())



def _init_arrangement():
    LOG.info('')

    app_globals = RlpUtil.APPGLOBALS.globals()
    if app_globals.get('startup.mode.batch'):
        LOG.info('Batch mode, skipping load arrangement')
        return

    if app_globals.get('gui.arrangement'):
        LOG.info('Arrangement already loaded, skipping')
        return


    startup_arrangement = app_globals.get('startup.arrangement')
    if startup_arrangement is None:
        startup_arrangement = 'playview2'

    # elif startup_arrangement == 'empty':
    #     if 'project.name' in app_globals:
    #         startup_arrangement = 'project'
    #     else:
    #         startup_arrangement = 'project_chooser'


    LOG.info('Loading arrangement: "{}"'.format(startup_arrangement))


    import revlens.gui.menu
    revlens.gui.menu.on_layout_load_triggered(startup_arrangement)


def _init_load_startup_media():

    startup_script = RlpUtil.APPGLOBALS.value('startup.script')
    if startup_script:
        startup_script = os.path.abspath(startup_script)
        LOG.info('Executing startup script: {}'.format(startup_script))
        try:
            exec(open(startup_script).read())
        except SystemExit as e:
            print(traceback.format_exc())
            LOG.info('Caught SystemExit - shutting down')
            sys.exit(e.code)
        except:
            print(traceback.format_exc())
        return


    LOG.info('Loading startup media..')

    from . import media
    media.startup_load_media()


def _init_login():

    print('init_login()')

    global CNTRL

    from revlens_prod.client_ws import SiteWsClient

    if os.getenv('RLP_NO_LOGIN'):
        from rlp.core.net.websocket import RlpClient
        rlpClient = SiteWsClient.init('ws://localhost:8003')
        RlpClient._INSTANCE = rlpClient
        CNTRL.emitStartupReady(10)
        return

    from .gui.dialogs import InfoDialog

    import revlens_prod.panel.login

    appGlobals = RlpUtil.APPGLOBALS.globals()
    auth = appGlobals.get('startup.auth', 'auth')

    print('GOT AUTH: {}'.format(auth))
    loginInfo = RlpUtil.read_login_info(auth=auth)
    print('init_login(): Got login info: {}'.format(loginInfo))
    if 'site_url'  not in loginInfo:
        return revlens_prod.panel.login.on_login_requested()

    # loginInfo['site_url'] = 'ws://vmpostweb-dev-01:8003'
    # loginInfo['site_url'] = 'ws://172.30.48.108:8003'

    if os.getenv('RLP_APP_PLATFORM') == 'ios':
        loginInfo['site_url'] = 'ws://192.168.1.67:8003'
    # loginInfo['site_url'] = 'ws://127.0.0.1:8003'

    rlpClient = SiteWsClient.init(loginInfo['site_url'])

    CNTRL.getServerManager().setWebSocketClient(rlpClient._wsclient)

    CNTRL.loginCntrl().loginFailed.connect(revlens_prod.panel.login.on_login_requested)
    CNTRL.loginCntrl().loginSuccessful.connect(revlens_prod.panel.login.on_login_successfull)

    def _handle_server_error(errInfo):
        LOG.error('Server Error: {}'.format(errInfo))
        InfoDialog.create('Server Error', errInfo['err_msg'])


    rlpClient._wsclient.errorReceived.connect(_handle_server_error)
    if hasattr(rlpClient, 'rfsc'):
        rlpClient.rfsc._wsclient.errorReceived.connect(_handle_server_error)


    rlpClient._wsclient.clientDisconnected.connect(
        revlens_prod.panel.login.on_disconnected)

    rlpClient._wsclient.tryReconnect.connect(
        revlens_prod.panel.login.on_reconnect_attempted)


    # Initiate Login flow
    #
    CNTRL.loginCntrl().loginCheck(
        loginInfo['site_url'],
        loginInfo['username'],
        loginInfo['password'], # WARNING: not cryptographically secure here
        'revlens.gui.login.login_done'
    )


def _init_env():

    LOG.debug('_init_env()')

    if 'wasm' not in PLATFORM:
        return

    _PLUG_ROOT_DIR = '/wasm_fs_root/pylib'
    LOG.debug('_init_env() - pylib root: {}'.format(_PLUG_ROOT_DIR))

    revlens_path_entries = []
    python_path_entries = []

    # add plugins to REVLENS_PATH
    sys.path.insert(0, _PLUG_ROOT_DIR)
    os.environ['REVLENS_PATH'] = _PLUG_ROOT_DIR


    LOG.debug('--')
    LOG.debug('sys.path:')
    for entry in sys.path:
        LOG.debug(entry)
    LOG.debug('--')


def _init_startup_ready():

    LOG.info('startup ready')

    _init_startup_context()
    _init_arrangement()
    _init_gui()
    _init_load_startup_media()


def _startup():

    _init_binding_objects()

    LOG.debug('')
    LOG.debug('starting up')
    LOG.debug('platform: {}'.format(PLATFORM))

    RlpUtil.APPGLOBALS.update(_get_initial_env())

    LOG.debug('startup app globals: {}'.format(RlpUtil.APPGLOBALS.globals()))

    _init_env()
    # _init_state_dir()
    #_init_prefs()
    _init_locators()
    _init_keyboard()

    # run from arrangement for isolation
    # _init_gui()

    _init_plugins()
    _init_cmdline_options()

    # startupReady emitted after successful login
    CNTRL.startupReady.connect(_init_startup_ready)

    _init_login()

    # GUI.pyGuiReady.connect(_init_login)


def startup():

    try:
        _startup()

    except SystemExit:
        raise

    except:
        print(traceback.format_exc())
        LOG.error(traceback.format_exc())

def init_standalone_gui():
    '''
    TODO FIXME: UNUSED REMOVE?
    '''
    import libRlpProdPYMODULE

    rlp.util_env.setup_env(update_sys_path=True)

    pybinary_path = sys.executable
    libRlpProdPYMODULE.initStandaloneGui(pybinary_path)



try:
    _init()
    del _init

except:
    print(traceback.format_exc())

