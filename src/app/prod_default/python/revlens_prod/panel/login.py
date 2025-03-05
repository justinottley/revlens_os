
import logging
import pprint

from rlp import QtCore
import rlp.gui as RlpGui
import rlp.util

import revlens

from revlens_prod.client_ws import SiteWsClient

LOG = logging.getLogger('rlp.{}'.format(__name__))
G_PANEL = None


def on_login_requested():

    global G_PANEL
    if G_PANEL:
        return

    floating_pane = revlens.VIEW.createFloatingPane(450, 340, True)

    G_PANEL = RevlensLoginPanel(floating_pane)


class LoginPanel(RlpGui.GUI_ItemBase):

    _WIDGET = None

    def __init__(self, floating_pane):
        RlpGui.GUI_ItemBase.__init__(self, floating_pane.body())

        self.setZValue(25)

        self._fpane = floating_pane

        app_globals = rlp.util.APPGLOBALS.globals()

        self.url = app_globals.get('edb.site_url') or '{}://127.0.0.1:8003'.format(SiteWsClient.SCHEME)

        site_client = SiteWsClient.init(self.url)

        self.LABEL_WIDTH = 100

        self.logo = RlpGui.GUI_Label(self, '')
        self.logo.setLeftImagePath(
            ":/misc/revlens_logo_alpha.png", 200, 0)
        self.logo.setLeftPadding(90)

        self.site_label = RlpGui.GUI_Label(self, "Site URL: ")
        self.site_label.setItemWidth(self.LABEL_WIDTH)

        self.site_text = RlpGui.GUI_TextEdit(self, 280, 60)
        self.site_text.setText(self.url) # https://site.revlens.io/

        self.site_layout = RlpGui.GUI_HLayout(self)
        self.site_layout.addItem(self.site_label, 0)
        self.site_layout.addItem(self.site_text, 0)

        self.username_label = RlpGui.GUI_Label(self, "Username: ")
        self.username_label.setItemWidth(self.LABEL_WIDTH)

        _username = app_globals.get('edb.username') or 'nobody'
        self.username_text = RlpGui.GUI_TextEdit(self, 200, 60)
        self.username_text.setText(_username)

        self.username_layout = RlpGui.GUI_HLayout(self)
        self.username_layout.addItem(self.username_label, 0)
        self.username_layout.addItem(self.username_text, 0)

        
        self.password_label = RlpGui.GUI_Label(self, "Password: ")
        self.password_label.setItemWidth(self.LABEL_WIDTH)

        self.password_text = RlpGui.GUI_TextEdit(self, 200, 60)

        _pw = ''
        if _username == 'nobody':
            _pw = 'nobody'

        self.password_text.setText(_pw)
        self.password_text.setTextHidden(True)

        self.password_layout = RlpGui.GUI_HLayout(self)
        self.password_layout.addItem(self.password_label, 0)
        self.password_layout.addItem(self.password_text, 0)

        self.login_button = RlpGui.GUI_IconButton(
            '', self, 20, 4)

        self.login_button.setText("Login")
        self.login_button.setOutlined(True)
        self.login_button.setItemWidth(70)
        self.login_button.setItemHeight(36)
        self.login_button.setTextHOffset(10)
        self.login_button.setTextYOffset(5)

        self.login_button.buttonPressed.connect(self.on_login_requested)

        self.skip_button = RlpGui.GUI_IconButton(
            '', self, 20, 4)
        self.skip_button.setText("Skip")
        self.skip_button.setOutlined(True)
        self.skip_button.setItemWidth(70)
        self.skip_button.setItemHeight(36)
        self.skip_button.setTextHOffset(10)
        self.skip_button.setTextYOffset(5)

        self.skip_button.buttonPressed.connect(self.skip_close)


        self.button_layout = RlpGui.GUI_HLayout(self)
        self.button_layout.addItem(self.login_button, 0)
        self.button_layout.addSpacer(60)
        self.button_layout.addItem(self.skip_button, 0)

        self.messages = RlpGui.GUI_Label(self, '')

        if site_client.username:
            self.messages.setText(
                "Logged in as: {}".format(site_client.username))

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.setHSpacing(30)

        self.layout.addItem(self.logo, 0)
        # self.layout.addSpacer(20)
        self.layout.addItem(self.site_layout, 0)
        # self.layout.addSpacer(15)
        self.layout.addItem(self.username_layout, 0)
        self.layout.addItem(self.password_layout, 0)
        # self.layout.addSpacer(10)
        self.layout.addItem(self.button_layout, 90) # self.login_button, 80)
        self.layout.addSpacer(5)
        self.layout.addItem(self.messages, 0)

        fgroup = self._fpane.focusGroup()
        fgroup.enterPressed.connect(self.on_login_requested)

        fgroup.addItem(self.site_text)
        fgroup.addItem(self.username_text)
        fgroup.addItem(self.password_text)


        self._fpane.setText('Login')
        self._fpane.setTextBold(True)
        self._fpane.setItemWidth(450)
        self._fpane.setItemHeight(400)

        self.cntrl.loginCntrl().loginFailed.connect(self.on_login_failed)
        self.cntrl.loginCntrl().loginSuccessful.connect(self.on_login_successful)

        self.setItemWidth(self._fpane.width())
        self.setItemHeight(self._fpane.height())


    @property
    def view(self):
        raise NotImplementedError()

    @property
    def cntrl(self):
        raise NotImplementedError()

    @property
    def login_callback(self):
        return ''


    def boundingRect(self):
        return self.parentItem().boundingRect()

    def on_login_requested(self, md):

        self.messages.setText("Attempting to Log in..")

        site_url = self.site_text.text()
        s = SiteWsClient.init(site_url)

        username = self.username_text.text()
        password = self.password_text.text()

        print('---')
        print(username)
        # print(password)
        print('----')
        self.cntrl.loginCntrl().loginCheck(
            s.url,
            username,
            password,
            self.login_callback
        )

        self.messages.setText(
            "Logging in '{}'..".format(username)
        )


    def on_login_failed(self):

        self.messages.setText("Login Failed")
        self.username_text.setText('')
        self.password_text.setText('')


    def on_login_successful(self):
        self.close()


    def close(self):
        self._fpane.requestClose()


    def skip_close(self, md):
        self.cntrl.emitStartupReady(100)
        self.close()


class RevlensLoginPanel(LoginPanel):

    _UI = []

    def __init__(self, floating_pane):
        LoginPanel.__init__(self, floating_pane)

    @property
    def view(self):
        return revlens.VIEW

    @property
    def cntrl(self):
        return revlens.CNTRL

    @property
    def login_callback(self):
        return 'revlens.gui.login_done'

    @classmethod
    def create(cls):

        floating_pane = revlens.VIEW.createFloatingPane(400, 400, True)
        panel = RevlensLoginPanel(floating_pane)
        cls._UI.append(panel)

        return panel