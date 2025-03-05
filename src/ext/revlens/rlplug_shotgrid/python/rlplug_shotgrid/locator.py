#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import os
import time
import pprint
import tempfile

from revlens.media.locator import RemoteFileLocator, MediaLocator

from .sg_handler import ShotgunHandler

from .objmodel.version import SGVersion

class ShotgunRemoteFileLocator(RemoteFileLocator):

    def __init__(self, download=True):
        RemoteFileLocator.__init__(self, download=download, source_name='Shotgun')
        
        #self.media_key_name = 'media.video.component.movie.name'
        self.media_key_url = 'media.video.component.movie.sg_upload_url'
        self.media_key_attachment_id = 'media.video.component.movie.sg_attachment_id'

        self.sg = ShotgunHandler.instance().sg

    def media_key_name(self, component_type, resolved_component):
        return 'media.{}.component.{}.name'.format(component_type, resolved_component)

    def can_download(self, src_path, media_dict, component_type, search_path):

        resolved_key = 'media.{}.resolved_component'.format(component_type)

        return media_dict and media_dict.get(self.media_key_url) and media_dict.get(resolved_key) in ['movie', 'frames']


    def download_file(self, file_path, media_dict=None, component_type=None):

        remote_url = media_dict[self.media_key_url]
        local_path = self.get_local_file_path(file_path)

        parent_dir = os.path.dirname(local_path)
        if not os.path.isdir(parent_dir):
            self.LOG.debug('creating {}'.format(parent_dir))
            os.makedirs(parent_dir)

        self.LOG.debug('downloading to {}'.format(local_path))

        '''
        self.http_transport.download(
            local_path,
            remote_url,
            resume=False,
            remote_url=True,
            progress_callback=self._progress_callback
        )
        '''

        # self.LOG.debug(pprint.pformat(media_dict))
        resolved_component = media_dict['media.{}.resolved_component'.format(component_type)]
        media_key = self.media_key_name(component_type, resolved_component)
        progress_str = media_dict[media_key]
        if media_dict.get('progress.media_count') and media_dict.get('progress.media_total'):
            progress_str += ' - {}/{} ..'.format(
                media_dict['progress.media_count'],
                media_dict['progress.media_total']
            )

        self._progress_callback(progress_str)

        sg_attachment_id = int(media_dict[self.media_key_attachment_id])
        self.sg.download_attachment(
            {'type': 'Attachment', 'id': sg_attachment_id},
            local_path
        )

        return local_path


class ComponentLocator(MediaLocator):
    '''
    Insert metadata into media by parsing information out of file path
    '''
    def __init__(self, *args, **kwargs):
        MediaLocator.__init__(self, *args, **kwargs)


    def _sanitize(self, path_in):

        result = path_in.replace('{f}', '#')
        result = result.replace('####', '#')

        return result

    