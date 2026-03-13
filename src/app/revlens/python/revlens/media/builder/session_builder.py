
import os
import uuid
import json
import pprint
import logging

class SessionBuilder(object):
    '''
    A standalone revlens session builder utility class that returns data objects
    without storing any state
    '''

    def __init__(self):
        self.logger = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))


    def _get_uuid(self):
        return '{{{u}}}'.format(u=str(uuid.uuid4()))

    def sanitize_media(self, media_in):
        
        media_out = media_in.copy()
        if 'graph' in media_out:
            del media_out['graph']

        media_out['properties'] = {}

        for key, value in media_in['properties'].items():
            if key.startswith('media.') and 'resolved_component' not in key:
                media_out['properties'][key] = value

        return media_out

    def sanitize_track(self, track_in):
        track_out = track_in.copy()
        track_out['media'] = {}

        track_media = track_in.get('media') or {}
        for media_uuid, media_data in track_media.items():
            track_out['media'][media_uuid] = self.sanitize_media(media_data)

        return track_out

    def sanitize_session(self, session_in):

        self.logger.debug('sanitize_session()') # : {}'.format(pprint.pformat(session_in)))

        session_out = session_in.copy()
        track_data_out = {}
        
        for track_uuid, track_data in session_in['track_data'].items():
            if track_data['track_type'] == 'Default':
                track_data_out[track_uuid] = self.sanitize_track(track_data)

            else:
                track_data_out[track_uuid] = track_data

        session_out['track_data'] = track_data_out
        return session_out


    def build_track(self, track_type, track_name, idx, owner='unknown'):
        track_uuid = self._get_uuid()
        return {
            'idx': idx,
            'name': track_name,
            'owner': owner,
            'track_type': track_type,
            'uuid': track_uuid
        }

    def build_annotation_track(self, track_name, idx):
        track = self.build_track('Annotation', track_name, idx)
        track.update({
            'annotation': {},
        })

        return track

    def build_media_track(self, track_name, idx, owner='unknown'):
        track = self.build_track('Default', track_name, idx, owner=owner)
        track.update({
            'media': {},
            'media_count': 0
        })

        return track

    def add_media(self, track, frame, media_info, uuid=None):
        media_uuid = uuid or self._get_uuid()
        if 'media' not in track:
            track['media'] = {}

        if 'media_count' not in track:
            track['media_count'] = 0

        track['media'][media_uuid] = {
            'frame': frame,
            'properties': media_info
        }
        track['media_count'] += 1



class SessionIO(object):
    '''
    pure python Session data object for compiling a session
    '''

    def __init__(self, builder=None):
        self.logger = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))
        self.builder = builder or SessionBuilder()
        self.data = {
            'version': 1,
            'session_uuid': self.builder._get_uuid(),
            'track_list': [],
            'track_data': {}
        }

        
        self.file_path = None

    def _get_next_track_idx(self):
        return len(self.data['track_list'])


    def add_track(self, track_name, track_idx=None, track_type='Default', owner='unknown'):
        
        if track_idx is None:
            track_idx = self._get_next_track_idx()
        
        if track_type == 'Default':
            track_data = self.builder.build_media_track(track_name, track_idx, owner=owner)

        track_uuid = track_data['uuid']
        self.data['track_list'].append(track_uuid)
        self.data['track_data'][track_uuid] = track_data

    def raw_add_track(self, track_data):

        self.logger.info('add_track_raw(): {}'.format(track_data))

        track_uuid = track_data['track_uuid']
        self.data['track_list'].append(track_uuid)
        self.data['track_data'][track_uuid] = track_data


    def raw_add_media_to_track(self, raw_media_info):

        frame = raw_media_info['frame']
        media_uuid = raw_media_info['media_uuid']
        track_uuid = raw_media_info['track_uuid']
        media_in = raw_media_info['media_in']

        if track_uuid in self.data['track_data']:
            track_data = self.data['track_data'][track_uuid]

            self.builder.add_media(track_data, frame, media_in, uuid=media_uuid)


    def raw_update_track(self, track_uuid, track_data):
        if track_uuid in self.data['track_data']:
            s_track_data = self.builder.sanitize_track(track_data)
            self.data['track_data'][track_uuid] = s_track_data


    def raw_delete_track(self, track_uuid):
        if track_uuid in self.data['track_data']:
            del self.data['track_data'][track_uuid]

        if track_uuid in self.data['track_list']:
            self.data['track_list'].remove(track_uuid)

    def raw_remove_media(self, track_uuid, media_uuid=None, frame=None):

        self.logger.debug('raw_remove_media() {} {} {}'.format(track_uuid, media_uuid, frame))

        if track_uuid in self.data['track_data']:
            track_data = self.data['track_data'][track_uuid]

            if 'media' in track_data:
                if frame:

                    # get the media at this frame
                    found_uuid = None
                    for media_uuid, media_data in track_data['media'].items():
                        if media_data['frame'] == frame:
                            found_uuid = media_uuid
                    if found_uuid:

                        self.logger.info('deleting media at frame {} - {}'.format(frame, found_uuid))
                        del track_data['media'][found_uuid]
                        if 'media_count' in track_data:
                            track_data['media_count'] -= 1

                elif media_uuid and media_uuid in track_data['media']:

                    self.logger.info('removing media {} from track {}'.format(media_uuid, track_uuid))

                    del track_data['media'][media_uuid]
                    if 'media_count' in track_data:
                        track_data['media_count'] -= 1

                else:
                    self.logger.warning('media not found: uuid: {}'.format(media_uuid))
            else:
                self.logger.error('raw_remove_media(): "media" key not found in track_data')


        else:            
            self.logger.error('raw_remove_media(): track not found: {}'.format(track_uuid))

    def raw_update_media_property(self, track_uuid, media_uuid, prop_key, prop_val):
        
        self.logger.info('updating media property {}'.format(prop_key))

        if track_uuid in self.data['track_data']:
            track_data = self.data['track_data'][track_uuid]

            if 'media' in track_data and media_uuid in track_data['media']:
                props = track_data['media'][media_uuid]['properties']
                props[prop_key] = prop_val

                track_data['media'][media_uuid]['properties'] = props

                self.logger.info('{} = {} added'.format(prop_key, prop_val))

        else:
            self.logger.error('raw_update_media_property(): track {} not found'.format(track_uuid))

        
    def flush(self, write_annotations=True):
        raise NotImplementedError


class SingleFileSessionIO(SessionIO):
    '''
    A writer to serialize a session into a single file
    '''

    def read(self, file_path):
        with open(file_path) as fh:
            self.data = json.load(fh)

    def write(self, file_path):
        self.logger.info('flush(): writing to {}'.format(file_path))

        data = self.builder.sanitize_session(self.data)

        with open(file_path, 'w') as wfh:
            wfh.write(json.dumps(data, indent=2))


class SplitFileSessionIO(SessionIO):
    '''
    A writer to serialize a session inside a directory structure, with
    annotations saved to separate files
    '''


    def _read_annotation_data(self, track_data_dir, track_data):
        
        self.logger.info('Loading track data from {}'.format(track_data_dir))

        for ann_entry in os.listdir(track_data_dir):
            ann_path = os.path.join(track_data_dir, ann_entry)

            self.logger.info('Loading annotation data at {}'.format(ann_path))

            with open(ann_path) as fh:
                ann_info = json.load(fh)
                ann_uuid = ann_info['uuid']
                if 'annotation' not in track_data or not track_data.get('annotation'):
                    track_data['annotation'] = {}

                track_data['annotation'][ann_uuid] = ann_info


    def update_track(self, track_uuid, track_data):
        
        track_data = self.builder.sanitize_track(track_data)
        self.data['track_data'][track_uuid] = track_data

        self.flush(write_annotations=False)

        
    def read(self, file_path, dir_root=None):
        if dir_root is None:
            dir_root = os.path.dirname(file_path)

        else:
            file_path = os.path.join(dir_root, file_path)

        if not file_path.endswith('.rls'):
            file_path += '.rls'


        self.logger.info('reading {}'.format(file_path))

        self.data = {}
        with open(file_path) as fh:
            self.data = json.load(fh)

        for track_uuid, track_data in self.data['track_data'].items():
            
            track_dir = os.path.join(dir_root, 'track', track_uuid, 'annotation')
            if os.path.isdir(track_dir):
                self._read_annotation_data(track_dir, track_data)
                '''
                ann_dir = os.path.join(track_dir, 'annotation')
                for ann_entry in os.listdir(ann_dir):
                    ann_path = os.path.join(ann_dir, ann_entry)

                    self.logger.info('reading annotation: {}'.format(ann_path))

                    self._read_annotation_data(ann_path, track_data)
                '''

            # if track_data.get('annotation.data'):
            #    self._read_annotation_data(track_data['annotation.data'], track_data)

        self.file_path = file_path


    def write_annotation_track(self, dir_root, track_data):

        track_uuid = track_data['uuid']
        ann_dir = os.path.join(dir_root, 'track', track_uuid, 'annotation')
        if not os.path.isdir(ann_dir):

            self.logger.debug('creating {}'.format(ann_dir))
            os.makedirs(ann_dir)

        for ann_uuid, ann_info in track_data['annotation'].items():
            ann_file_path = os.path.join(ann_dir, '{}.json'.format(ann_uuid))
            ann_info['data.path'] = ann_file_path
            with open(ann_file_path, 'w') as wfh:
                wfh.write(json.dumps(ann_info, indent=2))


        track_data['annotation.data'] = ann_dir
        track_data['annotation'] = {}


    def write(self, file_path, dir_root=None, write_annotations=True):

        if dir_root is None:
            dir_root = os.path.dirname(file_path)

        else:
            file_path = os.path.join(dir_root, file_path)

        if not file_path.endswith('.rls'):
            file_path += '.rls'

        self.logger.info('flush(): writing to {}'.format(file_path))

        if not os.path.isdir(dir_root):
            self.logger.debug('flush(): creating {}'.format(dir_root))
            os.makedirs(dir_root)

        sdata = self.builder.sanitize_session(self.data)

        if write_annotations:

            self.clean_annotations(dir_root)

            track_data = sdata['track_data']
            for track_uuid, track_data in track_data.items():
                if track_data['track_type'] == 'Annotation':
                    self.write_annotation_track(dir_root, track_data)


        with open(file_path, 'w') as wfh:
            wfh.write(json.dumps(sdata, indent=2))

        self.logger.info('Wrote {}'.format(file_path))

        self.file_path = file_path


    def clean_annotations(self, dir_root):

        track_dir_root = os.path.join(dir_root, 'track')
        track_uuid_list = os.listdir(track_dir_root)
        for track_uuid in track_uuid_list:
            track_dir = os.path.join(track_dir_root, track_uuid)
            ann_track_dir = os.path.join(track_dir, 'annotation')
            if os.path.isdir(ann_track_dir):

                self.logger.debug('clean_annotations(): cleaning {}'.format(ann_track_dir))
                for root, dirs, files in os.walk(ann_track_dir, topdown=False):
                    for file_entry in files:
                        file_fullpath = os.path.join(root, file_entry)

                        self.logger.debug('clean_annotations(): removing {}'.format(file_fullpath))
                        os.remove(file_fullpath)

            
            self.logger.debug('clean_annotations(): removing  {}'.format(ann_track_dir))
            os.rmdir(ann_track_dir)

            self.logger.debug('clean_annotations(): removing {}'.format(track_dir))
            os.rmdir(track_dir)



    def flush(self, write_annotations=False):
        if self.file_path:
            self.write(self.file_path, write_annotations=write_annotations)


