#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import os
import base64
import tempfile
import traceback

import revlens
from revlens.media.builder import MediaBuilder

from ..objmodel.version import SGVersion

class PlaylistMixin:

    def build_media_list_from_playlist(self, playlist_id, download_thumbnails=True):
        '''
        @param playlist_id: shotgun playlist id
        '''

        self._display_progress_message('Getting Playlist {pid}..'.format(pid=playlist_id))

        playlist_result = self.sg.find(
            'Playlist',
            [['id', 'is', playlist_id]],
            ['code', 'versions'])

        if len(playlist_result) != 1:
            raise Exception('invalid result: expected 1 result, got {n}'.format(n=len(playlist_result)))

        playlist_name = playlist_result[0]['code']
        ver_id_list = []
        for ver_result in playlist_result[0]['versions']:
            ver_id_list.append(ver_result['id'])


        rl_media_list = []

        self._display_progress_message('Getting Version info for {n} versions..'.format(n=len(ver_id_list)))

        ver_result = self.sg.find(
            'Version',
            [['id', 'in', ver_id_list]],
            SGVersion.VERSION_FIELDS
        )

        shot_id_map = {}
        ver_id_result_map = {}
        for ver_entry in ver_result:

            
            if ver_entry['entity']['type'] == 'Shot':

                shot_entity = ver_entry['entity']
                self._display_progress_message('Getting Shot info for {s}..'.format(s=shot_entity['name']))

                if shot_entity['id'] not in shot_id_map:
                    shot_info = self.sg.find(
                        'Shot', [['id', 'is', shot_entity['id']]],
                        [
                            'sg_cut_order', 'sg_cut_in', 'sg_cut_out', 'sg_head_in', 'sg_head_out'
                        ]
                    )
                    if len(shot_info) == 1:

                        self.logger.debug('Caching shot info for {s}'.format(s=shot_entity['name']))
                        shot_id_map[shot_entity['id']] = shot_info[0]

                    else:
                        self.logger.error('error looking up shot info from {si}'.format(si=shot_entity))


            ver_id_result_map[ver_entry['id']] = ver_entry
        
        #
        # it seems the result may not come back in the order we want,
        # ensure the order is preserved
        #

        for ver_id in ver_id_list:

            ver_info = ver_id_result_map[ver_id]
            entity_id = ver_info['entity']['id']

            ver_entity_info = {}
            for ikey, ival in ver_info['entity'].items():
                ver_entity_info['entity.{k}'.format(k=ikey)] = ival

            media_builder = self.version_make_media(ver_info)

            media_builder.metadata.update(ver_entity_info)

            try:
                
                if entity_id in shot_id_map:

                    shot = shot_id_map[entity_id]
                    
                    try:
                        media_builder.metadata.update({
                            'cut_in': shot['sg_cut_in'],
                            'cut_out': shot['sg_cut_out'],
                            'head_in': shot['sg_head_in'],
                            'head_out': shot['sg_head_out']
                        })
                    except:
                        self.logger.error(traceback.format_exc())
                        
                rl_media_list.append(media_builder.payload)

            except:
                msg = 'ERROR: could not load {c}'.format(c=ver_info['code'])
                self._display_progress_message(msg)
                self.logger.error(traceback.format_exc())
                
        
        return (rl_media_list, playlist_name)


        