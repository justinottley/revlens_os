
import sys

import RlpProdLAUNCHmodule

CNTRL = RlpProdLAUNCHmodule.CNTRL


class RevlensMediaInterface:

    LOCATORS = []


class _MediaLocator:
    pass


class RevlensMediaLocatorInterface:

    MediaLocator = _MediaLocator


class RevlensPluginInterface:

    PLUGIN_MEDIA = 1
    PLUGIN_MEDIA_LOCATOR = 9

    def register(self, plugin_name, plugin_item):

        if plugin_name == self.PLUGIN_MEDIA:
            CNTRL.registerMediaPlugin(plugin_item)

        elif plugin_name == self.PLUGIN_MEDIA_LOCATOR:
            CNTRL.registerMediaLocator(plugin_item)


class RevlensInterface:

    def __init__(self):
        self.media = RevlensMediaInterface()
        self.plugin = RevlensPluginInterface()


class _KeyBinding:

    @classmethod
    def register(key, funcName, desc):
        pass


class RevlensKeyboardInterface:

    KeyBinding = _KeyBinding()


media = RevlensMediaInterface()
media_locator = RevlensMediaLocatorInterface()
plugin = RevlensPluginInterface()
keyboard = RevlensKeyboardInterface()





