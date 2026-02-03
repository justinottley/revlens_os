
import os
import json
import pprint
import tempfile
import functools

import rlp.util as RlpUtil
from rlp.core.net.websocket import RlpClient

import revlens

from .. import sg_handler

# Yes that's right - forward to this namespace
from ..ioworker.playlist import *


SG = None
_UI = []

def _init_sg_all():
    for sgConfigType in ['prod_cloud', 'prod_onprem']:
        sg_handler.init_sg_revlens(sgConfigType, update=False)


def sg_find(entityType, filters, returnFields, *args, **kwargs):

    sg_config_name = None
    if 'sg_config_name' in kwargs:
        sg_config_name = kwargs['sg_config_name']
        del kwargs['sg_config_name']

    sgh = sg_handler.ShotgunHandler.instance(sg_config_name)

    return sgh.find(entityType, filters, returnFields, **kwargs)



def request_load_version(version_list, **kwargs):

    from rlplug_shotgrid.dialogs import ShotgridLoadDialog

    sgh = sg_handler.ShotgunHandler.instance()

    print('Got: {}'.format(version_list))

    ver_info = sgh.find('Version', [['id', 'in', version_list]], ['code', 'image', 'sg_path_to_frames'])
    ver_info = ver_info[0]

    # download thumb
    temp_fd, temp_thumb_path = tempfile.mkstemp(prefix='tmp_sg_thumbnail_', suffix='.jpg')
    os.close(temp_fd)
    sgh.sg.download_attachment({'url': ver_info['image']}, temp_thumb_path)

    print('Got thumb: {}'.format(temp_thumb_path))

    fpane = revlens.GUI.mainView().createFloatingPane(350, 400, False)
    diag = ShotgridLoadDialog(fpane, ver_info['code'], ver_info['sg_path_to_frames'], temp_thumb_path)
    _UI.append(diag) # YIKES


def request_load_reviewlist(review_list_entity, review_list_name=None, media_callback=None, **kwargs):
    print('LOAD REVIEW LIST')
    print(review_list_entity)

    edbc = RlpClient.instance()

    def _versions_ready(revlist_name, revlist_versions):

        media_list = []
        for ver_entry in revlist_versions:
            
            mcode = ver_entry['code']
            if media_callback:
                mcode, mediaMd = media_callback(ver_entry)

            '''
            
            codeParts = ver_entry['code'].split(':')
            testId = ''
            if len(codeParts) > 2:
                mcode = codeParts[2]
                testId = codeParts[-1]

            userName = ''
            if ver_entry.get('user'):
                userName = ver_entry['user']['name']

            {
                'entity.name': mcode,
                'user': userName,
                'description': ver_entry.get('description', ''),
                'task': testId, # For compatibility with existing HUD metadata setup
                'testid': testId
            }
            '''
            
            media_list.append({
                'media.name': mcode,
                'media.frame_count': ver_entry['frame_count'],
                'media.video': ver_entry['sg_path_to_frames'],
                'media.metadata': mediaMd
            })

            pprint.pprint(media_list)

        revlens.CNTRL.getMediaManager().loadMediaList(
            media_list, kwargs.get('track_idx', 0), revlist_name, 1, True, True)

    edbc.sg_find(
        functools.partial(_versions_ready, review_list_entity['code']),
        'Version',
        [['playlists', 'is', {'type': 'Playlist', 'id': review_list_entity['id']}]],
        ['code', 'sg_path_to_frames', 'frame_count', 'description', 'user']
    ).run()


def request_load_sequence_cached(sequence_info, **kwargs):

    print('REQUEST LAOD SEQUENCE')
    app_globals = RlpUtil.APPGLOBALS.globals()
    pprint.pprint(app_globals)

    sequence_cache_path = '/tmp/sg_cache/sequence/{}_{}.json'.format(
        app_globals['project.name'],
        sequence_info['code']
    )
    
    print(sequence_cache_path)
    if os.path.isfile(sequence_cache_path):
        print('Found: {}'.format(sequence_cache_path))

        with open(sequence_cache_path) as fh:
            media_list = json.load(fh)
            print(media_list)

            revlens.CNTRL.getMediaManager().loadMediaList(
                media_list, -1, sequence_info['code'], 1, True, True)


def get_sequence_media(sequence_info, media_callback=None, **kwargs):

    sg = sg_handler.ShotgunHandler.instance()
    app_globals = RlpUtil.APPGLOBALS.globals()
    # pprint.pprint(app_globals)
    project_entity = {'type': 'Project', 'id': app_globals['project.info']['id']}
    seq_entity = {'type': 'Sequence', 'id': sequence_info['id']}

    shot_result = sg.find('Shot',
        [['project', 'is', project_entity],
         ['sg_sequence', 'is', seq_entity],
         ['sg_status_list', 'is_not', 'oop']],
         ['code', 'image', 'sg_sequence'],
        order=[{'field_name': 'sg_orderno', 'direction': 'asc'}]
    )

    shot_media_list = []

    pref_dept_list = kwargs.get('pref_dept_list')

    for shot_entry in shot_result:

        print(shot_entry['code'])

        ver_filters = []
        if kwargs.get('filters'):
            ver_filters += kwargs['filters']

        ver_filters.append(
            ['entity', 'is', {'type': 'Shot', 'id': shot_entry['id']}]
        )


        print('')
        ver_result = sg.find('Version',
            ver_filters,
            ['sg_path_to_movie', 'sg_path_to_frames', 'sg_department', 'sg_final', 'code', 'user', 'image', 'frame_count'],
            order=[{'field_name': 'created_at', 'direction': 'desc'}]
        )

        if ver_result:
            use_ver = None
            if pref_dept_list:
                for dept_pref in pref_dept_list:

                    # print('')
                    # print('------=== CHECKING : {} ===------'.format(dept_pref))
                    # print('')

                    if use_ver:
                        break
                    
                    for ver_entry in ver_result:

                        if ver_entry['sg_department'] == dept_pref:

                            if kwargs.get('pref_order') == 'most_recent':
                                use_ver = ver_entry
                                break

                            elif kwargs.get('pref_order') == 'final' and ver_entry['sg_final']:
                                use_ver = ver_entry
                                break
                if not use_ver:
                    print('{}: WARNING: unable to locate a version, falling back to latest'.format(shot_entry['code']))
                    use_ver = ver_result[0]

            else:
                use_ver = ver_result[0]


            mediaName = use_ver['code']
            mediaMd = {}

            if media_callback:
                mediaName, mediaMd = media_callback(use_ver)

            media_result = {
                'media.video': use_ver['sg_path_to_frames'],
                'media.name': mediaName,
                'media.frame_count': use_ver['frame_count'],
                'media.metadata': mediaMd
            }
            shot_media_list.append(media_result)


    # pprint.pprint(shot_media_list)
    # print('LOADING')


    title = kwargs.get('title', sequence_info['code'])

    revlens.CNTRL.getMediaManager().loadMediaList(
        shot_media_list,
        kwargs.get('track_idx', -1),
        title,
        1,
        True,
        True
    )



def request_load(entity_type, entity_info, **kwargs):

    print('Shotgun request load: {} {} {}'.format(entity_type, entity_info, kwargs))

    # uniquify, preserve order
    result = []
    seen = set()
    if type(entity_info) == list:
        for eid in entity_info:
            if eid not in seen:
                result.append(int(eid))
                seen.add(eid)

    else:
        result = [entity_info]


    if entity_type == 'Version':
        request_load_version(result, **kwargs)

    elif entity_type in ['Reviewlist', 'Playlist']:
        request_load_reviewlist(result[0], **kwargs)

    elif entity_type == 'Sequence':
        get_sequence_media(result[0], **kwargs)
        # request_load_sequence_cached(result[0], **kwargs)



def find(entityType, filters, returnFields, **kwargs):
    return sg_handler.ShotgunHandler.find(entityType, filters, returnFields, **kwargs)