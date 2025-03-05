#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import os
import time
import uuid
import pprint
import logging
import tempfile
import traceback

import shotgun_api3

from rlp import QtCore

try:
    import rlp.util as RlpUtil
    import revlens
    
    import revlens.cmds
    import revlens.media
    from revlens.media.builder import MediaBuilder

except:
    print('WARNING: revlens imports failed:')
    print(traceback.format_exc())

try:
    import rlplug_qtbuiltin.cmds

except:
    pass


from .sequence import SequenceMixin
from .playlist import PlaylistMixin
from .version import VersionMixin

from ..objmodel.version import SGVersion

class ShotgunHandler(VersionMixin, SequenceMixin, PlaylistMixin):
    
    _INSTANCE = None

    _PROJECT_CACHE = {
        'name': {},
        'id': {}
    }

    SG_URL = None
    SG_API_USER = None
    SG_API_KEY = None

    def __init__(self):

        self.logger = logging.getLogger('rlp.{ns}.{cls}'.format(
            ns=__name__, cls=self.__class__.__name__))

        self.username = self._get_username()

        self.logger.debug('Creating ShotGrid API instance..')

        self.sg = shotgun_api3.Shotgun(
            self.SG_URL,
            self.SG_API_USER,
            self.SG_API_KEY
        )

        self.logger.debug('ShotGrid OK: {}'.format(self.SG_URL))

        self.revlens_cache_dir = os.path.join(tempfile.gettempdir(), 'revlens', 'file_cache')
        if not os.path.isdir(self.revlens_cache_dir):
            
            self.logger.info('creating {d}'.format(d=self.revlens_cache_dir))
            os.makedirs(self.revlens_cache_dir)


    @classmethod
    def instance(cls):
        if cls._INSTANCE is None:
            cls._INSTANCE = cls()

        return cls._INSTANCE


    def set_helper(self, webchannel_helper):
        '''
        Called from C++ - allows communication back to Javascript through C++ object
        '''
        self.logger.debug('set_helper() {}'.format(webchannel_helper))
        self.webchannel_helper = webchannel_helper


    def reset(self):
        self.SEQ_CACHE + {}


    def make_project(self, project_in):

        def _lookup(sg_filter):

            project_result = self.sg.find('Project',
                    [sg_filter],
                    ['id', 'name']
                )

            if project_result and len(project_result) == 1:
                self._PROJECT_CACHE['name'][project_result[0]['name']] = project_result[0]['id']
                self._PROJECT_CACHE['id'][project_result[0]['id']] = project_result[0]['name']

        sg_id = None
        project_in_cls = project_in.__class__.__name__
        if project_in_cls in ['dict']:
            return project_in
        
        elif project_in_cls in ['basestring', 'str', 'unicode']:
            if project_in not in self._PROJECT_CACHE['name']:
                _lookup(['name', 'is', project_in])
            
            sg_id = self._PROJECT_CACHE['name'][project_in]

        elif project_in_cls in ['int', 'float', 'double']:
            
            project_in = int(project_in)
            if project_in not in self._PROJECT_CACHE['id']:

                # just to make sure this is a valid project..
                _lookup(['id', 'is', project_in])
            
            # just to make sure this is a valid project.. will error otherwise
            _ = self._PROJECT_CACHE['id'][project_in]

            sg_id = project_in

        if sg_id:
            return {'type': 'Project', 'id': sg_id}


    def _get_username(self):
        try:
            return RlpUtil.APPGLOBALS.globals()['username']

        except:
            return os.getenv('USER', os.getenv('USERNAME'))


    def _display_progress_message(self, message):

        if hasattr(revlens, 'CNTRL'):

            try:
                rlplug_qtbuiltin.cmds.display_message(message)
                revlens.CNTRL.getVideoManager().update()
                QtCore.QCoreApplication.processEvents()

            except Exception as e:
                self.logger.warning('_update_for_progress(): error - {}'.format(e))


    def _get_frame_num_from_pos(self, track_idx, pos):

        frame_num = None
        do_clear = False

        if pos == 'Start':
            frame_num = 1
            # do_clear = True # clear the track if loading at the start

        elif pos == 'End':
            
            track = revlens.CNTRL.session().getTrackByIndex(track_idx)
            frame_num = max(track.getFrames() or [0]) + 1

        elif pos == 'Current Frame':
            frame_num = revlens.cmds.get_current_frame_num()

        else:
            raise Exception('invalid frame position input: "{}"'.format(pos))

        return (frame_num, do_clear)

    #
    #  --------
    #


    def load_entity_sequence(self, seq_id, track_idx, frame_pos):
        # self.logger.debug(pprint.pformat(ver_list))
        frame_num, _ = self._get_frame_num_from_pos(track_idx, frame_pos)

        self.logger.debug('load_entity_sequence(): {} track_idx: {} pos: {} - {}'.format(
            seq_id, track_idx, frame_pos, frame_num
        ))

        seq_id = int(seq_id)
        rl_media_list, seq_name = self.build_media_list_from_sequence(seq_id)
        
        track = revlens.CNTRL.session().getTrackByIndex(track_idx)
        track.setName(seq_name)

        revlens.media.load_media_list(rl_media_list, track_idx, start_frame=frame_num, clear=True)


    def load_entity_playlist(self, playlist_id, track_idx, frame_pos):

        frame_num, _ = self._get_frame_num_from_pos(track_idx, frame_pos)

        self.logger.debug('load_entity_playlist(): {} track_idx: {} position: {} - {}'.format(
            playlist_id, track_idx, frame_pos, frame_num))

        playlist_id = int(playlist_id)
        rl_media_list, playlist_name = self.build_media_list_from_playlist(playlist_id)

        track = revlens.CNTRL.session().getTrackByIndex(track_idx)
        track.setName(playlist_name)

        # always clear the track if we are loading a playlist
        #
        revlens.media.load_media_list(rl_media_list, track_idx, start_frame=frame_num, clear=True)



    def load_entity_version(self, entity_id_list, track_idx, frame_pos):
        '''
        NOTE: Must be run inside revlens
        @param track_idx: track index
        @param frame_pos: frame position, either 'Start', 'End', or 'Current Frame'
        '''

        if not revlens.CNTRL:
            raise Exception('Requires revlens!')

        frame_num, do_clear = self._get_frame_num_from_pos(track_idx, frame_pos)

        self.logger.info('load_entity_version(): {} track_idx: {} position: {} - {}'.format(
            entity_id_list, track_idx, frame_pos, frame_num
        ))

        entity_id_list = [int(eid) for eid in entity_id_list]
        result_list = self.sg.find(
            'Version',
            [['id', 'in', entity_id_list]],
            SGVersion.VERSION_FIELDS
        )
        
        self.logger.info(result_list)
        
        rl_media_list = []
        
        if result_list:
            
            for ver_info in result_list:

                for field_name in ['sg_path_to_movie', 'sg_path_to_frames']:
                    if field_name in ver_info:
                        field_value = ver_info[field_name].replace('\\', '/')
                        ver_info[field_name] = field_value

                self.logger.debug(ver_info)
                
                media_builder = self.version_make_media(ver_info)
                rl_media_list.append(media_builder.payload)

        revlens.media.load_media_list(rl_media_list, track_idx, start_frame=frame_num, clear=do_clear)


    def load_entity(self, entity_type, entity_id_list, track_idx, frame_pos):
        
        self.logger.info('load entity')
        self.logger.info('args:')
        self.logger.info(entity_type)
        self.logger.info(entity_id_list)
        
        track_idx = int(track_idx)

        media_list = []
        if entity_type == 'Version':
            self.load_entity_version(entity_id_list, track_idx, frame_pos)
            
        elif entity_type == 'Playlist':
            self.load_entity_playlist(entity_id_list[0], track_idx, frame_pos)

        elif entity_type == 'Sequence':
            self.load_entity_sequence(entity_id_list[0], track_idx, frame_pos)
            
        else:
            self.logger.error('Entity type not supported for load: {et}'.format(
                et=entity_type))



    def sendCallToClient(self, method, args, kwargs):
        self.webchannel_helper.emitSendCallToClient({
            'method': method,
            'args': args,
            'kwargs': kwargs
        })



    # -------

    def get_track_names(self, track_type=revlens.TRACK_TYPE_MEDIA):
        return revlens.cmds.get_track_names(track_type=track_type)