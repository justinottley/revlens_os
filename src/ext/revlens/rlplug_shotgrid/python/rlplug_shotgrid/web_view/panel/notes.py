
import rlp.gui as RlpGui

from rlplug_shotgrid.sg_handler import ShotgunHandler

from .. import shotgun as rlplug_sg

class ShotgunNotesBrowserPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.url = 'about:blank'
        sgh = ShotgunHandler.instance()
        if sgh:
            sgh_url = ShotgunHandler.instance().url
            self.url = '{}/page/notes_app'.format(sgh_url)
        self.browser = RlpGui.GUI_WebEngineItem(self, self.url)

        self.startup()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def startup(self):

        rlplug_sg.init_shotgun_plugin(self.browser)


    def onParentSizeChangedItem(self, width, height):
        self.setWidth(width)
        self.setHeight(height)
        # self.browser.setUrl(self.url)


def create(parent):
    return ShotgunNotesBrowserPanel(parent)