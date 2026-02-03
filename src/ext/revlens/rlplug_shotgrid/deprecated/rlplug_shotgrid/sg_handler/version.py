#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import os
import base64
import tempfile

from rlp import QtGui
from revlens.media.builder import MediaBuilder

from ..objmodel.version import SGVersion

class VersionMixin:

    def version_make_media(self, ver_info, download_thumbnail=True):

        media_builder = MediaBuilder()
        media_builder.metadata.update(SGVersion.get_media_metadata(ver_info))
        
        if ver_info['image'] and download_thumbnail:

            self._display_progress_message('Downloading thumbnail for {s}..'.format(s=ver_info['code']))
            
            tempfilename = os.path.join(tempfile.gettempdir(), 'sg_thumbnail_{}.jpg'.format(ver_info['code']))

            if not os.path.isfile(tempfilename):
                temp_fd, tempfilename = tempfile.mkstemp(prefix='tmp_sg_thumbnail_', suffix='.jpg')
                os.close(temp_fd)

                self.logger.debug('Downloading thumbnail: {s} at url {u} to {t}'.format(
                    s=ver_info['code'], u=ver_info['image'], t=tempfilename
                ))

                self.sg.download_attachment({'url': ver_info['image']}, tempfilename)
                
                '''
                response = requests.get(ver_info['image'])
                if response.status_code == 200:

                    with open(tempfilename, 'wb') as wfh:
                        wfh.write(response.content)

                    result = True
                '''

                self.logger.debug('resizing thumbnail..')

                # resize down
                #
                resized_thumb = QtGui.QImage(tempfilename).scaledToWidth(200)
                resized_thumb.save(tempfilename)

                self.logger.debug('Thumbnail resized')
        
            thumb_data = b''
            with open(tempfilename, 'rb') as bfh:
                thumb_data = bfh.read()
            thumb_data = base64.b64encode(thumb_data)
            ver_info['thumb_data'] = thumb_data.decode()


        if ver_info.get('sg_path_to_movie'):
            media_builder.add_video_component('movie', ver_info['sg_path_to_movie'], name=ver_info['code'])
            media_builder.add_audio_component('movie', ver_info['sg_path_to_movie'], name=ver_info['code'])


        if ver_info.get('sg_path_to_frames'):
            media_builder.add_video_component('frames', ver_info['sg_path_to_frames'], name=ver_info['code'])


        if ver_info.get('sg_uploaded_movie'):
            media_builder.add_video_component_info('movie', 'sg_upload_url', ver_info['sg_uploaded_movie']['url'])
            media_builder.add_video_component_info('movie', 'sg_attachment_id', ver_info['sg_uploaded_movie']['id'])

            media_builder.add_audio_component_info('movie', 'sg_upload_url', ver_info['sg_uploaded_movie']['url'])
            media_builder.add_audio_component_info('movie', 'sg_attachment_id', ver_info['sg_uploaded_movie']['id'])


        if ver_info.get('thumb_data'):
            media_builder.add_thumbnail_component(ver_info['thumb_data'])
        
        # HANDLES
        #
        if media_builder.metadata.get('department') in ['Animation', 'TechAnim', 'Layout']:
            media_builder.payload['media.frame_in'] = 0
            media_builder.payload['media.frame_out'] = 0

        return media_builder