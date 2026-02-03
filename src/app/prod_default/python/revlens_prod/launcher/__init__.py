
import os
import sys
import logging
import tempfile
import platform
import functools
import traceback
import subprocess

import rlp.util
import rlp.core
from rlp.core.net.websocket import RlpClient

import rlp.gui as RlpGui


LOG = logging.getLogger('rlp.launcher')

# seeded from C++
CNTRL = None
VIEW = None

_UI = {}

def on_triggered(func, checked=True):
    func()


def run_quit():
    sys.exit(0)


def init_binding():

    rlp.util._init()
    # rlp.core._init()
    # RlpGui._init()

    import RlpProdLAUNCHmodule
    rlp.core.util.loft_to_module(__name__, RlpProdLAUNCHmodule)


    import RlpGuiPANEmodule

    global VIEW
    VIEW = RlpGuiPANEmodule.VIEW


def init_tools():

    print('init_tools()')

    global VIEW

    all_tools = [
        {
            'name': 'Launch',
            'item_type': 'tool',
            'pyitem': 'revlens_prod.panel.launch.main'
        }
        # {
        #     'name': 'Transfers',
        #     'item_type': 'tool',
        #     'pyitem': 'revlens_prod.panel.transfers'
        # }
    ]

    for tool_entry in all_tools:
        VIEW.registerToolGUI(tool_entry)


def init_menu():

    global _UI
    global VIEW
    global CNTRL

    menubar = CNTRL.menuBar()
    menu_file = menubar.addMenu('File')

    act_quit = menu_file.addAction('Quit')
    act_quit.setStatusTip('Quit')
    act_quit.triggered.connect(functools.partial(on_triggered, run_quit))

    _UI['menu.file'] = menu_file
    # _UI['menu.file.quit'] = act_quit


def init_connections():

    print('init_connections()')
    
    # from ..panel import transfers

    # global CNTRL
    # pass


def init_login():
    
    print('init_login()')

    global CNTRL
    CNTRL.loginCntrl().loginFailed.connect(show_login_panel)
    CNTRL.loginCntrl().loginInit("revlens_prod.launcher.login_done")



def login_done(result):
    print('LAUNCHER - LOGIN DONE!!')
    print(result)

    rlpClient = RlpClient.instance()

    if result and 'result' in result and result['result']:
        print('Login OK')
        loginResult = rlpClient.info
        loginResult['result'] = True

        CNTRL.loginCntrl().loginDone(loginResult)

        CNTRL.initLayout()

    else:
        print('Login failed, aborting')
        return



def show_login_panel():

    global VIEW
    global _UI

    from ..panel.login import launcher_login

    print('SHOW LOGIN PANEL')

    if _UI.get('panel.login'):
        return _UI['panel.login'].showItem()

    print('Creating new Login Panel')

    floating_pane = VIEW.createFloatingPane(400, 400, False) # True)
    launcher_panel = launcher_login.LauncherLoginPanel(floating_pane)

    launcher_login.G_PANEL = launcher_panel

    _UI['panel.login'] = launcher_panel


def startup():

    print('Starting Up')

    init_binding()
    init_connections()
    init_tools()
    init_menu()
    init_login()
