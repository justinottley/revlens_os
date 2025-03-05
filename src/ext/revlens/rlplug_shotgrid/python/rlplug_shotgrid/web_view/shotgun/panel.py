#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import logging
import traceback

from rlp import QtCore
import rlp.gui as RlpGui

import revlens
import revlens.media

from rlplug_shotgrid.sg_handler import ShotgunHandler

from . import (
    init_shotgun_plugin,
    write_site_config,
)

G_PANEL = None
G_SETTINGS = None

class ShotgunSettingsPanel(RlpGui.GUI_ItemBase):

    def __init__(self, fpane):
        RlpGui.GUI_ItemBase.__init__(self, fpane.body())

        self.settingsUpdated = QtCore.PySignal()

        self._fpane = fpane
        self._fpane.setText('ShotGrid Settings')
        self._fpane.setTextBold(True)

        self.layout = RlpGui.GUI_VLayout(self)

        # self.label = RlpGui.GUI_Label(self, "ShotGrid Settings")

        self.url_layout = RlpGui.GUI_HLayout(self)
        self.url_layout.setSpacing(10)
        self.url_label = RlpGui.GUI_Label(self, "URL: ")
        self.url = RlpGui.GUI_TextEdit(self, 260, 50)
        
        self.url_layout.addItem(self.url_label, 0)
        self.url_layout.addItem(self.url, 0)

        self.api_user_layout = RlpGui.GUI_HLayout(self)
        self.api_user_layout.setSpacing(10)
        self.api_user_label = RlpGui.GUI_Label(self, "API User: ")
        self.api_user = RlpGui.GUI_TextEdit(self, 200, 50)

        self.api_user_layout.addItem(self.api_user_label, 0)
        self.api_user_layout.addItem(self.api_user, 0)

        self.api_key_layout = RlpGui.GUI_HLayout(self)
        self.api_key_layout.setSpacing(10)
        self.api_key_label = RlpGui.GUI_Label(self, "API Key: ")
        self.api_key = RlpGui.GUI_TextEdit(self, 200, 50)

        self.api_key_layout.addItem(self.api_key_label, 0)
        self.api_key_layout.addItem(self.api_key, 0)


        self.save_button = RlpGui.GUI_IconButton("", self, 20, 4)
        self.save_button.setText("Save")
        self.save_button.setOutlined(True)
        self.save_button.buttonPressed.connect(self.on_save_requested)

        # self.layout.addItem(self.label, 0)
        self.layout.addSpacer(5)
        self.layout.addItem(self.url_layout, 0)
        self.layout.addItem(self.api_user_layout, 0)
        self.layout.addItem(self.api_key_layout, 0)
        self.layout.addSpacer(10)
        self.layout.addItem(self.save_button, 80)

        self.setItemWidth(self._fpane.width())
        self.setItemHeight(self._fpane.height())


    def on_save_requested(self):
        
        sg_url = self.url.text()
        sg_api_user = self.api_user.text()
        sg_api_key = self.api_key.text()

        site_config = {
            'sg_url': sg_url,
            'api_user': sg_api_user,
            'api_key': sg_api_key
        }

        write_site_config(site_config)
        
        self.settingsUpdated.emit(site_config)

        self.close()


    def close(self):

        # should be a floating pane
        self._fpane.onCloseRequested()


class ShotgunBrowserPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":/feather/lightgrey/refresh-cw.svg",
            self,
            20
        )
        self.refresh_button.buttonPressed.connect(self.on_refresh_requested)

        self.settings_button = RlpGui.GUI_SvgButton(
            ":/feather/lightgrey/settings.svg",
            self,
            20
        )

        self.settings_button.buttonPressed.connect(self.on_settings_requested)

        self.status = RlpGui.GUI_Label(self, "-")

        self.url = ShotgunHandler.SG_URL

        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.addSpacer(10)
        self.toolbar_layout.addItem(self.refresh_button, 0)
        self.toolbar_layout.addSpacer(5)
        self.toolbar_layout.addItem(self.settings_button, 0)
        self.toolbar_layout.addSpacer(5)
        self.toolbar_layout.addItem(self.status, 0)

        self.layout = RlpGui.GUI_VLayout(self)

        self.browser = RlpGui.GUI_WebEngineItem(self, self.url)
        self.browser.setYOffset(30)
        self.browser.loadingChanged.connect(self.on_loading_changed)

        self.layout.addItem(self.toolbar_layout, 0)
        self.layout.addItem(self.browser, 0)

        self.startup()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def startup(self):

        print('web browser startup')
        init_shotgun_plugin(self.browser)
        self.browser.setUrl(self.url)


    def on_refresh_requested(self, md=None):
        self.browser.setUrl(self.url)
        self.browser.forceResetProfile()

    def on_loading_changed(self, state):

        # print('pn_loading_changed {}'.format(state))

        smap ={
            0: 'Loading..',
            1: 'Stopped',
            2: 'Done',
            3: 'Load Failed'
        }

        sstate = smap[state]
        self.status.setText(sstate)
        self.status.update()


    def on_settings_requested(self, md=None):

        global G_SETTINGS
        fpane = revlens.VIEW.createFloatingPane(400, 300, False)

        settings_panel = ShotgunSettingsPanel(fpane)
        settings_panel.settingsUpdated.connect(self.startup)

        G_SETTINGS = settings_panel


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)


def create(parent):
    global G_PANEL

    panel = ShotgunBrowserPanel(parent)
    G_PANEL = panel

    return panel
