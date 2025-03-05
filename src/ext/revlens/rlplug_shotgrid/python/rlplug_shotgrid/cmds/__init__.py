#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import os
import tempfile

import revlens


from rlplug_shotgrid.sg_handler import ShotgunHandler
from rlplug_shotgrid.dialogs import ShotgridLoadDialog

_UI = []

def request_load(entity_type, entity_id_val):

    # print('Shotgun request load: {} {}'.format(entity_type, entity_id_val))

    sgh = ShotgunHandler.instance()

    # uniquify, preserve order
    result = []
    seen = set()
    if type(entity_id_val) == list:
        for eid in entity_id_val:
            if eid not in seen:
                result.append(int(eid))
                seen.add(eid)


    print('Got: {}'.format(result))
    ver_info = sgh.sg.find('Version', [['id', 'in', result]], ['code', 'image', 'sg_path_to_frames'])
    print(ver_info)

    ver_info = ver_info[0]

    # download thumb
    temp_fd, temp_thumb_path = tempfile.mkstemp(prefix='tmp_sg_thumbnail_', suffix='.jpg')
    os.close(temp_fd)
    sgh.sg.download_attachment({'url': ver_info['image']}, temp_thumb_path)

    print('Got thumb: {}'.format(temp_thumb_path))

    fpane = revlens.VIEW.createFloatingPane(350, 400, False)
    diag = ShotgridLoadDialog(fpane, ver_info['code'], ver_info['sg_path_to_frames'], temp_thumb_path)
    _UI.append(diag) # YIKES


def register_mouse_down(args):
    print('register mouse down')
    print(args)



def test_request_load():
    version_id = 'None'
    request_load(['Version', [version_id]])