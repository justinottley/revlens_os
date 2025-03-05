
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
                
                splitter = pane_in.splitter(pidx)
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
            'size': {'width': revlens.VIEW.widthValue(), 'height': revlens.VIEW.heightValue()}
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


def _init_compact():

    import revlens
    import revlens.gui
    import revlens.gui.timeline
    import rlplug_qtbuiltin.cmds

    revlens._init_gui()
    revlens.gui.init_menu_default_layout()
    revlens.gui.on_toggle_tab_headers()
    revlens.gui.timeline.on_style_compact_toggled()
    rlplug_qtbuiltin.cmds.toggleHud()
