

import rlp.util as RlpUtil
import rlp.gui as RlpGui

import revlens.gui.menu
from revlens.keyboard import KeyBinding

def toggle_navtools(md=None):

    ntb = revlens.gui.menu._UI['menu.toggle_navtools']
    ntb.setToggled(not ntb.isToggled())
    ntb.setOutlined(ntb.isToggled())
    ntb.update()

    mpane = revlens.GUI.mainPane()
    ntSplitter = mpane.panePtr(0).splitterPtr(0)

    vpos = ntb.metadataValue('viewer_pos')

    if ntb.isToggled():
        ntSplitter.setPosPercent(vpos, True)

    else:
        ntb.setMetadata('viewer_pos', ntSplitter.splitterPos())
        ntSplitter.setPosPercent(0.995, True)

    mpane.update()
    mpane.updatePanes(0)


def toggle_timeline(md=None):

    ttb = revlens.gui.menu._UI['menu.toggle_timeline']
    ttb.setToggled(not ttb.isToggled())
    ttb.setOutlined(ttb.isToggled())
    ttb.update()

    mpane = revlens.GUI.mainPane()
    tlSplitter = mpane.splitterPtr(0)

    npos = ttb.metadataValue('timeline_pos')

    if ttb.isToggled():
        tlSplitter.setPosPercent(npos, True)

    else:
        ttb.setMetadata('timeline_pos', tlSplitter.splitterPos())
        tlSplitter.setPosPercent(0.995, True)

    mpane.update()
    mpane.splitterPtr(0).update()
    mpane.updatePanes(0)




def init_playview_layout():

    mmb = revlens.GUI.mainMenuBar()

    toggle_timeline_button = RlpGui.GUI_IconButton(':misc/video-editor.svg', mmb, 15, 6)
    ttbIcon = toggle_timeline_button.svgIcon()
    ttbIcon.setPos(5, 5)

    toggle_timeline_button.setTextYOffset(0)
    toggle_timeline_button.setText('Timeline')
    toggle_timeline_button.setMetadata('timeline_pos', 0.8)
    toggle_timeline_button.buttonPressed.connect(toggle_timeline)

    mmb.layout().addSpacer(10)
    mmb.layout().addDivider(20, 20, 3)
    mmb.layout().addSpacer(10)
    mmb.layout().addItem(toggle_timeline_button, 2)

    revlens.gui.menu._UI['menu.toggle_timeline'] = toggle_timeline_button

    toggle_navtools_button = RlpGui.GUI_IconButton(':misc/compass.svg', mmb, 15, 6)
    toggle_navtools_button.setToggled(True)
    toggle_navtools_button.setOutlined(True)
    toggle_navtools_button.setTextYOffset(0)
    toggle_navtools_button.setText('Navigation')
    toggle_navtools_button.setMetadata('viewer_pos', 0.44)
    toggle_navtools_button.buttonPressed.connect(toggle_navtools)

    navIcon = toggle_navtools_button.svgIcon()
    navIcon.setPos(5, 5)

    revlens.gui.menu._UI['menu.toggle_navtools'] = toggle_navtools_button


    mmb.layout().addSpacer(10)
    mmb.layout().addItem(toggle_navtools_button, 2)

    # if we started with something on the commandline, hide nav tools
    startupMedia =  RlpUtil.APPGLOBALS.value('startup.media')
    if startupMedia and len(startupMedia) > 0:
        toggle_navtools()


    # ---

    mpane = revlens.GUI.mainPane()
    tlPane = mpane.panePtr(1)
    tlPane.setHeaderVisible(False, False)


    KeyBinding.register(
        't',
        'revlens.gui.arrangement.playview.toggle_timeline',
        'Toggle Timeline'
    )

    KeyBinding.register(
        'n',
        'revlens.gui.arrangement.playview.toggle_navtools',
        'Toggle Navigation Tools'
    )

