
import os

class MediaBuilder(object):
    '''
    Utility class for building a usable media payload
    '''

    def __init__(self):
        self.payload = {
            'media.video.components': [],
            'media.audio.components': [],
            'media.metadata': {}
        }

    @property
    def metadata(self):
        return self.payload['media.metadata']

    def _add_component(self, track_type, component_type_name, file_path, name=None):
        '''
        @param track_type: "video", "audio"
        @param component_type_name: "movie", "frames"
        '''

        if not name:
            name = os.path.basename(file_path)

        key_base = 'media.{}.component.{}'.format(
            track_type, component_type_name)

        key_name = '{}.name'.format(key_base)
        key_path = '{}.path'.format(key_base)

        self.payload[key_name] = name
        self.payload[key_path] = file_path
        self.payload['media.{}.components'.format(track_type)].append(component_type_name)


    def _add_component_info(self, track_type, component_type_name, info_key, value):
        media_key = 'media.{}.component.{}.{}'.format(
            track_type, component_type_name, info_key
        )
        self.payload[media_key] = value

    def add_video_component(self, component_type_name, file_path, name=None):
        self._add_component('video', component_type_name, file_path, name=name)

    def add_video_component_info(self, component_type_name, info_key, value):
        self._add_component_info('video', component_type_name, info_key, value)

    def add_audio_component(self, component_type_name, file_path, name=None):
        self._add_component('audio', component_type_name, file_path, name=name)

    def add_audio_component_info(self, component_type_name, info_key, value):
        self._add_component_info('audio', component_type_name, info_key, value)

    

    def add_thumbnail_component(self, thumb_data):
        self.payload['media.thumbnail.data'] = thumb_data