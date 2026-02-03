
import os
import json
import pprint

from rlp.util import buildopt

import revlens

def _serialize_current_arrangement():

    main_pane = revlens.GUI.mainPane()
    print('{} {}'.format(main_pane, main_pane.paneCount()))

    # pane_list = []

    def _populate_panes(pane_in):

        # pane_list = []

        print('populate_panes(): {} num subpanes: {}'.format(pane_in, pane_in.paneCount()))
        
        pane_info = {}
        num_tabs = pane_in.tabCount()
        if num_tabs > 0:
            print('Getting tabs')
            pane_info['tabs'] = []
            for tab_idx in range(pane_in.tabCount()):
                tab_name = pane_in.header().tabHeader(tab_idx).name()
                print('Got tab: {}'.format(tab_name))
                pane_info['tabs'].append({'name': tab_name})
        
        pane_info['panes'] = []

        print('calling paneCount() on {}'.format(pane_in))
        subpane_count = pane_in.paneCount()
        for pidx in range(subpane_count):

            print('ITERATING: {} {} {}'.format(pane_in, pidx, pane_in.paneCount()))
            subpane = pane_in.panePtr(pidx)

            print('GOT SUBPANE: {}'.format(subpane))
            
            if (pidx < (subpane_count - 1)):
                
                splitter = pane_in.splitterPtr(pidx)
                orientation = pane_in.orientationInt()
                pane_pos = splitter.splitterPos()
                pane_info['pos'] = pane_pos
                pane_info['orientation'] = orientation

            print('Getting subpanes')
            subpane_info = _populate_panes(subpane)
            
            
            
            pane_info['panes'].append(subpane_info)

            print(pane_info)

        print('Done: {}'.format(pane_in))

        return pane_info

    
    arr_result = _populate_panes(main_pane)

    result = {
        'arrangement': arr_result,
        'window': {
            'size': {
                'width': revlens.GUI.mainView().width(),
                'height': revlens.GUI.mainView().height()
            }
        }
    }

    
    return result


def get_arrangement_path(layout_name):

    if 'wasm' in buildopt.get_platform():
        return '/wasm_fs_root/etc/arrangement_{}.json'.format(layout_name)


    layout_dir = revlens._init_state_dir('windowstate')
    layout_filename = 'arrangement_{}.json'.format(layout_name)
    
    layout_path = os.path.join(layout_dir, layout_filename)
    if os.path.isfile(layout_path):
        return layout_path


    for entry_path in os.getenv('REVLENS_PATH').split(os.pathsep):

        # check 'static' subdir for ios
        entry_dir, entry_basename = os.path.split(entry_path)
        static_etc_dir = os.path.join(entry_dir, 'static', entry_basename, 'etc')
        if os.path.isdir(static_etc_dir):
            etc_dir = static_etc_dir
        else:
            etc_dir = os.path.join(entry_path, 'etc')

        if not os.path.isdir(etc_dir):
            print('get_arrangement_path(): not found: {}'.format(etc_dir))
            continue

        layout_path = os.path.join(etc_dir, layout_filename)
        if os.path.isfile(layout_path):
            return layout_path

    print('_get_layout(): ERROR: layout not found: {}'.format(layout_name))


def save_arrangement(layout_name):

    print('save arrangement: {}'.format(layout_name))

    layout_dir = revlens._init_state_dir('windowstate')
    layout_filename = 'arrangement_{}.json'.format(layout_name)

    arr_path = os.path.join(layout_dir, layout_filename)
    arr_data = _serialize_current_arrangement()

    with open(arr_path, 'w') as wfh:
        wfh.write(json.dumps(arr_data, indent=2))

    print('Wrote {}'.format(arr_path))


def _init_compact(tab_headers=False):

    import revlens
    import revlens.gui.menu
    import revlens.gui.timeline
    import rlplug_qtbuiltin.cmds

    revlens._init_gui()

    revlens.gui.menu.init_menu_default_layout()
    if not tab_headers:
        revlens.gui.menu.on_toggle_tab_headers()

    revlens.gui.timeline.on_style_compact_toggled()
    rlplug_qtbuiltin.cmds.toggleHud()


def _init_playview_layout():

    import rlp.gui as RlpGui
    import revlens.gui.menu

    mmb = revlens.GUI.mainMenuBar()

    def _toggleTimeline(mb):

        ttb = revlens.gui.menu._UI['menu.toggle_timeline']
        ttb.setToggled(not ttb.isToggled())
        ttb.setOutlined(ttb.isToggled())

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


    toggle_timeline_button = RlpGui.GUI_IconButton(':misc/video-editor.svg', mmb, 15, 6)
    ttbIcon = toggle_timeline_button.svgIcon()
    ttbIcon.setPos(5, 5)

    toggle_timeline_button.setTextYOffset(0)
    toggle_timeline_button.setText('Timeline')
    toggle_timeline_button.setMetadata('timeline_pos', 0.8)
    toggle_timeline_button.buttonPressed.connect(_toggleTimeline)

    mmb.layout().addSpacer(10)
    mmb.layout().addDivider(20, 20, 3)
    mmb.layout().addSpacer(10)
    mmb.layout().addItem(toggle_timeline_button, 2)

    revlens.gui.menu._UI['menu.toggle_timeline'] = toggle_timeline_button

    mpane = revlens.GUI.mainPane()
    tlPane = mpane.panePtr(1)
    tlPane.setHeaderVisible(False, False)


def _init_min_layout():

    print('INIT MIN LAYOUT')
    import revlens

    def _onToolCreated(md):

        if md['name'] == 'Viewer':

            viewer = md['tool']
            viewer.setFillerHeight(5)
            pbar = viewer.getToolbarByName('Playbar')
            if pbar:
                pbar.timelineView().mediaManager().setMediaEnabled(False) # For performance on very large sessions


    revlens.CNTRL.toolCreated.connect(_onToolCreated)
