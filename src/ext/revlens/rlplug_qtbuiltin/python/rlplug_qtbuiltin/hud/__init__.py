'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import os
import json
import pprint
import logging
import traceback


from rlp.Qt import QStr
from rlp import QtGui
from rlp.util import buildopt

import revlens
import revlens.plugin

import rlplug_qtbuiltin

LOG = logging.getLogger('rlp.rlplug_qtbuiltin.hud')

def _get_sftset_dir():
    
    if 'wasm' in buildopt.get_platform():
        return '/wasm_fs_root'

    sftset_dir = os.path.dirname(__file__)
    for x in range(5):
        sftset_dir = os.path.dirname(sftset_dir)
        
    return sftset_dir

def toggleMediaHud():

    LOG.info('toggle media hud')
    
    hud_cntrl = rlplug_qtbuiltin.RLQTDISP_HUD_OBJ
    n = revlens.CNTRL.session().getNodeByFrame(revlens.cmds.get_current_frame_num())
    if n:

        curr_visible = hud_cntrl.isHudVisible('metadata')
        if curr_visible:
            hud_cntrl.setHudVisible('metadata', False)
            return

        hud_settings = hud_cntrl.getHudInfo('metadata')

        x_offset = hud_settings.get('position.offset.x', 0)
        y_offset = hud_settings.get('position.offset.y', 0)

        font_metric = QtGui.QFontMetrics(QtGui.QFont(QStr('Andale Mono')))
        font_col_list = [200, 200, 200]
        
        line_height = font_metric.height()
        props = n.getProperties()
        

        hud_list = []
        y_pos = 20 + line_height

        # get length of longest key
        key_len = 0
        max_line = ''
        prop_keys = sorted(props.keys())
        for prop_name in prop_keys:
            name_len = len(prop_name)
            if name_len > key_len:
                key_len = name_len

        
        for prop_name in prop_keys:

            prop_val = props[prop_name]
            prop_name_adjusted = str(prop_name).ljust(key_len + 1)
            # print prop_name

            # HACK: handle video.frame_info specifically
            #
            if prop_name == 'video.frame_info':
                # LOG.info(dir(prop_val))
                prop_val = '[list] - {} entries'.format(len(prop_val))
                
            if prop_name == 'media.thumbnail.data':
                prop_val = '<base64 data>'

            if type(prop_val) == dict:
                
                hud_list.append({
                    'item.value': '{n}: {{dict}}:'.format(n=prop_name.ljust(key_len + 1)),
                    'position.x.value': 20,
                    'position.y.value': y_pos,
                    'font.size': 10,
                    'font.opacity': 0.8,
                    'font.colour': font_col_list
                })
                y_pos += line_height
                
                fi_len = 0
                for fi_name in prop_val:
                    fi_val = prop_val[fi_name]
                    if len(fi_name) > fi_len:
                        fi_len = len(fi_name)

                for fi_name in sorted(prop_val):
                    fi_val = prop_val[fi_name]
                    hud_list.append({
                        'item.value': '{n}:   {fi}: {fv}'.format(
                            n=' '.ljust(key_len + 1),
                            fi=str(fi_name).ljust(fi_len + 1),
                            fv=fi_val),
                        'position.x.value': 20,
                        'position.y.value': y_pos,
                        'font.size': 10,
                        'font.opacity': 0.8,
                        'font.colour': font_col_list
                    })
                    y_pos += line_height

            else:
                
                item_str = '{n}: {v}'.format(n=prop_name_adjusted, v=str(prop_val))
                hud_list.append({
                    'item.value': item_str,
                    'position.x.value': 20,
                    'position.y.value': y_pos,
                    'font.size': 10,
                    'font.opacity': 0.8,
                    'font.colour': font_col_list
                })

                y_pos += line_height

            item_len = len(item_str)
            if item_len > len(max_line):
                max_line = item_str

        
        rect_width = font_metric.horizontalAdvance(QStr(max_line), -1)
        hud_cntrl.addHudConfig(
            'metadata',
            {
                'item_list': hud_list,
                'visible': True,
                'rect.x': 5,
                'rect.y': 5,
                'rect.height': y_pos,
                'rect.width': rect_width,
                'movable': True,
                'position.offset.x': x_offset,
                'position.offset.y': y_offset
            }
        )

        hud_cntrl.setHudVisible('metadata', True)

        # pprint.pprint(props)
        # pprint.pprint(hud_list)

def startup():
    
    LOG.info('startup()')

    
    hud_cntrl = rlplug_qtbuiltin.RLQTDISP_HUD_OBJ
    
    hud_config_dir = os.path.join(_get_sftset_dir(), 'etc', 'hud')
    LOG.info(hud_config_dir)
    
    hud_cntrl.addHudConfig(
        'metadata', {})

    for hud_config_file in os.listdir(hud_config_dir):
        
        hud_config_path = os.path.join(hud_config_dir, hud_config_file)
        hud_name = os.path.splitext(hud_config_file)[0].replace('hud_', '')
        try:
            with open(hud_config_path) as fh:
                hud_data = json.load(fh)
                
                LOG.info('Registering HUD "{h}" - {path}'.format(
                    h=hud_name,
                    path=hud_config_path))
                
                hud_cntrl.addHudConfig(
                    hud_name, hud_data)
                
        except:
            
            LOG.error('could not register hud {h}'.format(h=hud_name))
            LOG.error(traceback.format_exc())
            
    
    
    # hud_cntrl.setCurrentHud("min")
    
    revlens.plugin.register(
        revlens.plugin.PLUGIN_EVENT,
        hud_cntrl)



