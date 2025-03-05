
import rlp.gui as RlpGui

_PANELS = []

class WebEnginePanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent, url='https://wikipedia.org'):
        RlpGui.GUI_ItemBase.__init__(self, parent)


        self.setItemWidth(parent.width())
        self.setItemHeight(parent.height())

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":/feather/lightgrey/refresh-cw.svg",
            self,
            20
        )
        self.refresh_button.buttonPressed.connect(self.on_set_url)

        self.url = url
        self.url_text = RlpGui.GUI_TextEdit(self, 400, 60)
        self.url_text.enterPressed.connect(self.on_set_url)

        self.status = RlpGui.GUI_Label(self, "-")

        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.addSpacer(10)
        self.toolbar_layout.addItem(self.refresh_button, 0)
        self.toolbar_layout.addSpacer(10)
        self.toolbar_layout.addItem(self.url_text, 0)
        self.toolbar_layout.addSpacer(10)
        self.toolbar_layout.addItem(self.status, 0)

        self.layout = RlpGui.GUI_VLayout(self)

        self.browser = RlpGui.GUI_WebEngineItem(self, url)
        self.browser.setYOffset(70)
        self.browser.loadingChanged.connect(self.on_loading_changed)

        self.layout.addItem(self.toolbar_layout, 0)
        self.layout.addItem(self.browser, 0)

        self.startup()
        self.onParentSizeChangedItem(parent.width(), parent.height())


    def startup(self):

        self.url_text.setText(self.url)
        self.browser.setUrl(self.url)


    def onParentSizeChangedItem(self, width, height):
        self.setItemWidth(width)
        self.setItemHeight(height)


    def on_set_url(self, md=None):
        print('SET URL')
        self.browser.setUrl(self.url_text.text())

        # to work around bug where web viewport doesn't refresh when changing panel size
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


def create(parent):
    global _PANELS
    panel = WebEnginePanel(parent)
    _PANELS.append(panel)

    return panel