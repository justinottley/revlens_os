

import rlp.gui as RlpGui

import revlens

'''
DOESNT WORK WITH WEB PLAYER ON MACOS CODECS MAYBE??
'''


class ReviewPreviewPlayer(RlpGui.GUI_ItemBase):

    _INSTANCE = None

    def __init__(self, fpane):
        RlpGui.GUI_ItemBase.__init__(self, fpane.body())

        self._fpane = fpane
        self._fpane.setPersistent(True)
        self.browser = RlpGui.GUI_WebEngineItem(self, 'about:blank')

        self.setWidth(fpane.width())
        self.setHeight(fpane.height())


    @classmethod
    def show(cls, filePath):
        if cls._INSTANCE is None:
            print('Creating new floating player instance')

            fpane = revlens.GUI.mainView().createFloatingPane(340, 360, False)
            cls._INSTANCE = cls(fpane)

        rpp = cls._INSTANCE

        url = 'file://{}'.format(filePath)
        print('Attempting to show: {}'.format(url))
        rpp.browser.setUrl(url)
        rpp.setVisible(True)
        rpp._fpane.requestShow()



