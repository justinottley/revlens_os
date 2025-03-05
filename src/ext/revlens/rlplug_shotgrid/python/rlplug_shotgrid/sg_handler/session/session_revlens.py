#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import pprint

from revlens.media.builder import SessionBuilder
# from revlens.media.translation_map import TRANSLATION_MAP_REVLENS_TMP_FILECACHE

from ..dailies import RevlensReviewClient

class RevlensSessionMixin:

    def build_revlens_session_from_playlist(self, playlist_id, download=False, uri=False):
        '''
        Build a revlens session from a shotgun playlist
        '''

        rl_media_list, playlist_name = self.build_media_list_from_playlist(playlist_id)

        session_builder = SessionBuilder()

        media_track = session_builder.build_media_track(playlist_name, 0)
        ann_track = session_builder.build_annotation_track('Draw', 1)

        frame = 1
        for rl_media_entry in rl_media_list:

            pprint.pprint(rl_media_entry)
            media_md = rl_media_entry['media.metadata']

            assert(rl_media_entry['media.frame_count'])
            
            media_length = rl_media_entry['media.frame_count']
            # media_md['last_frame'] - media_md['first_frame'] + 1

            print('{f} : {l} {mp}'.format(
                f=frame, l=media_length, mp=rl_media_entry['media.video']
            ))
            self._add_media(media_track, frame, rl_media_entry)
            frame = frame + media_length

        # NOTE for track_list:
        # order of the tracks is important - seems
        #
        session_data = {
            'track_data': {
                ann_track['uuid']: ann_track,
                media_track['uuid']: media_track
            },
            'track_list': [
                media_track['uuid'], 
                ann_track['uuid']
            ],
            'version': 1
        }

        return (playlist_name, session_data)


    def create_revlens_sync_session_from_playlist(self, playlist_id, download=False, uri=True):
        '''
        Create a sync tracked revlens session from a shotgun playlist
        '''

        playlist_id = int(playlist_id)
        playlist_name, session_data = self.build_revlens_session_playlist(playlist_id, uri=uri)

        ws_ip = '127.0.0.1'
        ws_port = 7401

        ws_url = 'ws://{rs_ip}:{rs_port}/ws'.format(
            rs_ip=ws_ip,
            rs_port=ws_port)

        tc = RevlensReviewClient(ws_url)
        tc.start(playlist_name, session_data)
