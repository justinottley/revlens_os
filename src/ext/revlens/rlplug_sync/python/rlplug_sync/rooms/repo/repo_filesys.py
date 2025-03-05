'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import os
import json
import time
import pprint
import logging
import tempfile
import traceback

from rlp import QtCore
import rlp.core.util


from revlens.media.builder.session_builder import SplitFileSessionIO

class FilesysRoomsRepository(object):

    _INSTANCES = {} # map of site name to repository

    def __init__(self, site_name, edbc):

        self.DIR_ROOT = '{}/rooms/{}'.format(
            os.getenv('RLP_EDB_ROOT', '/opt/rlp'),
            os.getenv('REVLENS_ROOMSERVER_PORT', '8003')
        )

        self.LOG = logging.getLogger('rlp.{ns}.{cls}'.format(
            ns=self.__class__.__module__, cls=self.__class__.__name__))

        self.site_name = site_name
        self.edbc = edbc

        # TODO FIXME: refactor / move this out of FilesysRepo impl to higher level interface class
        # store for clients per room
        #
        self.room_info = {}

        self._init_fs()
        self._init_room_startup()



    @classmethod
    def instance(cls, site_name, edbc):
        if cls._INSTANCES.get(site_name) is None:
            cls._INSTANCES[site_name] = FilesysRoomsRepository(site_name, edbc)

        return cls._INSTANCES[site_name]


    def _init_room_startup(self):

        
        self.LOG.info('_init_room_startup()')

        room_list = self.get_rooms()
        for room_entry in room_list:

            self.LOG.info('_init_room_startup(): {rn}'.format(rn=room_entry['room_name']))

            room_entry['clients'] = {}
            room_entry['room_media'] = []
            
            if 'room_uuid' not in room_entry:
                self.LOG.error('Invalid room data for room')
                self.LOG.error(pprint.pformat(room_entry))
                continue


            room_uuid = room_entry['room_uuid']

            media_result = self.edbc.find(
                'ReviewMedia',
                [['review_link', 'is', {'uuid': room_uuid}]],
                ['media_link'],
                -1,
                [],
                True
            )

            for entry in media_result:
                room_entry['room_media'].append(entry['media_link']['uuid'].toString())

            self.LOG.info('ReviewMedia RESULT:')
            self.LOG.info(pprint.pformat(media_result))

            self.room_info[room_entry['room_name']] = room_entry


    def _init_fs(self):

        self.LOG.info('_init_fs(): {} site_name:{}'.format(self.DIR_ROOT, self.site_name))

        if not os.path.isdir(self.dir_base):
            
            self.LOG.info('Creating {d}'.format(d=self.dir_base))
            os.makedirs(self.dir_base)
            
        for subdir in ['room', 'client']:
            room_subdir = os.path.join(self.dir_base, subdir)
            if not os.path.isdir(room_subdir):
                os.mkdir(room_subdir)

    @property
    def dir_base(self):
        return os.path.join(self.DIR_ROOT, self.site_name)


    @property
    def dir_rooms(self):
        return os.path.join(self.dir_base, 'room')

    '''
    @property
    def dir_clients(self):
        return os.path.join(self.dir_base, 'client')
    '''

    def _get_room_dir(self, room_name):
        room_dirname = rlp.core.util.slugify(room_name)
        return os.path.join(self.dir_rooms, room_dirname)

    def _get_room_filepath(self, room_name):
        room_filename = '{rn}.json'.format(rn=rlp.core.util.slugify(room_name))
        return os.path.join(self._get_room_dir(room_name), room_filename)


    def _get_room_metadata_filepath(self, room_name):
        room_metadata_path = os.path.join(self._get_room_dir(room_name), 'room.json')
        return room_metadata_path


    def _init_session(self, session_data):

        # self.LOG.debug('_init_session(): {d}'.format(d=session_data))
        
        '''
        session_data = session_data or {}
        if 'track_data' not in session_data:

            self.LOG.warning('"track_data" not found in session, initializing')
            session_data['track_data'] = {}

        if 'track_list' not in session_data:

            self.LOG.warning('"track_list" not found in session, initializing')
            session_data['track_list'] = []

        if 'version' not in session_data:

            self.LOG.warning('"version" not found in session_data, initializing')
            session_data['version'] = 1
        '''


        return session_data


    def _init_room(self, room_name, room_uuid=None, room_data=None, clean=False, session=None):
        
        room_dir = self._get_room_dir(room_name)

        if clean and os.path.isdir(room_dir):
            self.LOG.info('cleaning {}'.format(room_dir))
            for root, dirs, files in os.walk(room_dir, topdown=False):
                for file_entry in files:
                    file_fullpath = os.path.join(root, file_entry)
                    
                    self.LOG.debug('deleting file {}'.format(file_fullpath))
                    os.remove(file_fullpath)


                for dir_entry in dirs:
                    dir_full = os.path.join(root, dir_entry)
                    
                    self.LOG.debug('deleting directory {}'.format(dir_full))
                    os.rmdir(dir_full)


        if not os.path.isdir(room_dir):
            self.LOG.debug('creating {d}'.format(d=room_dir))
            os.makedirs(room_dir)

        # serialize room metadata
        #
        room_metadata_path = self._get_room_metadata_filepath(room_name)
        room_metadata = {}
        if room_data and room_data.get('metadata'):
            room_metadata = room_data['metadata']

        media_uuid_list = []
        if session:
            # grab media uuids from session
            
            for track_data in session.get('track_data', {}).values():
                media_uuid_list += track_data.get('media', {}).keys()


        room_status = 'open'

        room_s_data = {
            'room_name': room_name,
            'room_uuid': room_uuid,
            'room_status': room_status,
            'room_dirname': rlp.core.util.slugify(room_name),
            'room_media': media_uuid_list,
            'created_at': time.time(),
            'metadata': room_metadata
        }

        self.LOG.info(pprint.pformat(room_s_data))
        # NOTE: much of this metadata in process of being deprecated
        # for switchover to EDB
        #
        with open(room_metadata_path, 'w') as wfh:
            wfh.write(json.dumps(room_s_data))


        track_dir = os.path.join(room_dir, 'track')
        
        if not os.path.isdir(track_dir):
            os.mkdir(track_dir)

        self.room_info[room_name] = {
            'room_uuid': room_uuid,
            'room_status': room_status,
            'room_media': [],
            'clients': {}
        }


    def get_room_session_data(self, room_name):
        
        session_data = None

        if self.room_info[room_name].get('session_io') is None:
            self.room_info[room_name]['session_io'] = SplitFileSessionIO()
        
        session_io = self.room_info[room_name]['session_io']

        room_uuid = self.room_info[room_name]['room_uuid']
        room_status = self.room_info[room_name]['room_status']

        room_dir_root = self._get_room_dir(room_name)
        session_io.read(room_name, dir_root=room_dir_root)

        return_result = {
            'session': session_io.data,
            'room_name': room_name,
            'room_uuid': room_uuid,
            'room_status': room_status
        }
        self.LOG.debug('got room data: {rd}'.format(rd=pprint.pformat(return_result)))
        
        return return_result

        '''


        track_dir = os.path.join(self._get_room_dir(room_name), 'track')
        room_filepath = self._get_room_filepath(room_name)
        
        self.LOG.debug('loading {r}'.format(r=room_filepath))
        
        with open(room_filepath) as fh:
            session_data = json.load(fh)
            
        for track_uuid in os.listdir(track_dir):
            ann_dir = os.path.join(track_dir, track_uuid, 'annotation')
            for ann_filename in os.listdir(ann_dir):
                ann_uuid = ann_filename.replace('.json', '')
                ann_path = os.path.join(ann_dir, ann_filename)
                self.LOG.debug('reloading annotation data from {fp}'.format(fp=ann_path))
                with open(ann_path) as wfh:
                    try:
                        ann_data = json.load(wfh)
                        if track_uuid not in session_data['track_data']:

                            assert(track_uuid not in session_data['track_list'])

                            session_data['track_data'][track_uuid] = {
                                'annotation': {},
                                'name': 'UnknownDraw',
                                'track_type': 'Annotation',
                                'idx': len(session_data['track_list']),
                                'uuid': track_uuid
                            }

                            session_data['track_list'].append(track_uuid)


                        ann_track = session_data['track_data'][track_uuid]['annotation']
                        if ann_uuid not in ann_track:
                            ann_track[ann_uuid] = ann_data

                        else:
                            ann_track[ann_uuid]['data'] = ann_data['data']


                    except:
                        self.LOG.error(traceback.format_exc())
        '''




    def _flush_room(self, room_name, room_data, clean=False):
        
        self.LOG.info('_flush_room(): {}'.format(room_name))

        if not room_name:
            self.LOG.error('_flush_room(): no room name, aborting')
            return


        self._init_room(room_name, room_data, clean=clean)
        for track_uuid, track_data in room_data['track_data'].items():
            track_dir = os.path.join(self._get_room_dir(room_name), 'track', track_uuid)
            ann_dir = os.path.join(track_dir, 'annotation')

            for dir_entry in [track_dir, ann_dir]:
                if not os.path.isdir(dir_entry):
                    self.LOG.info('creating {d}'.format(d=dir_entry))
                    os.makedirs(dir_entry)

            
            if track_data['track_type'] == 'Annotation':

                # Offload the annotation data to a separate file
                #
                for ann_uuid, ann_data in track_data['annotation'].items():
                    ann_filename = '{u}.json'.format(u=ann_uuid)
                    ann_path = os.path.join(track_dir, 'annotation', ann_filename)
                    with open(ann_path, 'w') as wfh:
                        wfh.write(json.dumps(ann_data))

                        # remove the annotation data from the main session data
                        #
                        ann_data['data'] = ''


        with open(self._get_room_filepath(room_name), 'w') as wfh:

            wfh.write(json.dumps(room_data, indent=2))


    def _update_annotation(self, room_name, track_uuid, ann_uuid, ann_data):

        track_dir = os.path.join(self._get_room_dir(room_name), 'track', track_uuid)
        if not os.path.isdir(track_dir):
            os.mkdir(track_dir)

        ann_filename = '{au}.json'.format(au=ann_uuid)
        ann_dir = os.path.join(track_dir, 'annotation')

        if not os.path.isdir(ann_dir):
            os.makedirs(ann_dir)

        ann_path = os.path.join(track_dir, 'annotation', ann_filename)

        with open(ann_path, 'w') as wfh:
            wfh.write(json.dumps(ann_data))

        self.LOG.debug('Wrote {p}'.format(p=ann_path))


    #
    # Public API
    #

    def get_rooms(self):
        room_dirs = os.listdir(self.dir_rooms)

        room_list = []
        for room_direntry in room_dirs:

            self.LOG.debug('get_rooms(): {}'.format(room_direntry))
            
            room_md = os.path.join(self.dir_rooms, room_direntry, 'room.json')
            if not os.path.isfile(room_md):
                self.LOG.error('get_rooms(): room metadata file not found, skipping: {}'.format(room_md))
                continue

            
            self.LOG.debug('reading {}'.format(room_md))
            try:
                with open(room_md) as fh:

                    room_data = json.load(fh)
                    room_data['created_at_str'] = time.ctime(room_data['created_at'])
                    room_list.append(room_data)

                    # pull in data from in memory if available
                    #
                    # room_name = room_data['room_name']
                    # if room_name in self.room_info:
                    #     room_media = self.room_info[room_name]['room_media']

                    #     room_data['room_media'] = room_media

            except:
                self.LOG.error('Error reading room data for {}'.format(room_direntry))
                self.LOG.error(traceback.format_exc())

        return room_list


    def create_room(self, room_name, room_uuid, session=None):
        
        self.LOG.info('Creating room "{}" uuid: {}'.format(room_name, room_uuid))
        
        # room_obj = Room(room_name, session=session)

        room_dir = self._get_room_dir(room_name)
        self._init_room(room_name, room_uuid, clean=True, session=session)

        sess_io = SplitFileSessionIO()
        if session:
            sess_io.data = session

        sess_io.write(room_name, room_dir)
        
        self.room_info[room_name]['session_io'] = sess_io

        return True


    def update_room(self, room_name, action, update_info):
        
        self.LOG.info('update_room(): {} {} {}'.format(room_name, action, update_info))

        if action == 'addTrack':

            sess_io = self.room_info[room_name].get('session_io')
            if sess_io:
                sess_io.raw_add_track(update_info)
                sess_io.flush()

        elif action == 'deleteTrack':

            sess_io = self.room_info[room_name].get('session_io')
            if sess_io:
                track_uuid = update_info['track_uuid']
                sess_io.raw_delete_track(track_uuid)
                sess_io.flush()

        elif action == 'insert_media_at_frame':
            
            room_data = self.room_info[room_name]
            room_uuid = room_data['room_uuid']
            
            if media_uuid not in room_data['room_media']:
                self.LOG.info('Registering ReviewMedia entity')
                rcreate = self.edbc.create(
                    'ReviewMedia',
                    'rm_{}'.format(media_uuid),
                    {
                        'media_link': {'uuid': media_uuid},
                        'review_link': {'uuid': room_uuid}
                    },
                    ''
                )
                self.LOG.info(rcreate)

            sess_io = self.room_info[room_name].get('session_io')
            if sess_io:
                sess_io.raw_add_media_to_track(update_info)
                sess_io.flush()

        elif action in ['load_media_list', 'load_previous_shot']:

            room_data = self.room_info[room_name]
            room_uuid = room_data['room_uuid']
            sess_io = room_data['session_io']

            self.LOG.info(sess_io)
            self.LOG.info(pprint.pformat(update_info))

            if sess_io:

                track_uuid = update_info.get('track_uuid')
                track_info = update_info.get('track_info')
                # media_list = update_info.get('media_input_list', {}).get('media', {})
                media_list = track_info['media'].values()
                for media_track_info in media_list:
                    
                    media_info = media_track_info['properties']
                    media_uuid = media_info['graph.uuid']

                    if media_uuid not in room_data['room_media']:
                        
                        self.LOG.info('Creating ReviewMedia Entity review_link={} media_link={}'.format(
                            room_uuid, media_uuid
                        ))
                        # create ReviewMedia entity
                        #
                        self.edbc.create(
                            'ReviewMedia',
                            media_uuid,
                            {
                                'review_link': {'uuid':room_uuid},
                                'media_link': {'uuid':media_uuid}
                            },
                            ''
                        )

                        room_data['room_media'].append(media_uuid)

                sess_io.raw_update_track(track_uuid, track_info)
                sess_io.flush()

        elif action == 'mediaRemoved':

            self.LOG.debug('running mediaRemoved')

            sess_io = self.room_info[room_name]['session_io']
            if sess_io:
                track_uuid = update_info.get('track_uuid')
                media_uuid = update_info.get('media_uuid')
                frame = update_info.get('frame')
                sess_io.raw_remove_media(track_uuid, media_uuid=media_uuid, frame=frame)
                sess_io.flush()


        elif action == 'setSession':
            session_data = update_info['session_data']
            self._flush_room(room_name, session_data, clean=True)

            return session_data

        elif action == 'mediaUpdate':

            # self.LOG.info('got mediaUpdate')

            if update_info['media_type'] == 'annotation':
                ann_data = update_info['ann_data']
                track_uuid = update_info['track_uuid']
                ann_uuid = ann_data['uuid']

                self._update_annotation(room_name, track_uuid, ann_uuid, ann_data)

            elif update_info['media_type'] == 'media_property':
                
                self.LOG.info('got media_property')

                sess_io = self.room_info[room_name]['session_io']
                if sess_io:
                    track_uuid = update_info['track_uuid']
                    media_uuid = update_info['media_uuid']
                    prop_key = update_info['prop_key']
                    prop_val = update_info['prop_val']

                    sess_io.raw_update_media_property(track_uuid, media_uuid, prop_key, prop_val)
                    sess_io.flush()


        elif action in ['holdFrame', 'releaseFrame']:
            
            track_uuid = update_info['track_uuid']
            ann_uuid = update_info['ann_uuid']
            frame_list = update_info['frame_list']

            ann_filename = '{au}.json'.format(au=ann_uuid)
            ann_dir = os.path.join(self._get_room_dir(room_name), 'track', track_uuid, 'annotation')
            ann_path = os.path.join(self._get_room_dir(room_name), 'track', track_uuid, 'annotation', ann_filename)
            
            if os.path.isfile(ann_path):
                with open(ann_path, 'r+') as fh:
                    ann_data = json.load(fh)
                    ann_data['frame_list'] = frame_list
                    fh.seek(0)
                    fh.write(json.dumps(ann_data))
                    fh.truncate()

                    self.LOG.debug('wrote {f}'.format(f=ann_path))

            else:
                self.LOG.error('file not found: {f}'.format(f=ann_path))

        elif action == 'clearAnnotations':

            track_uuid = update_info['track_uuid']
            ann_uuid = update_info['ann_uuid']

            ann_filename = '{au}.json'.format(au=ann_uuid)
            ann_dir = os.path.join(self._get_room_dir(room_name), 'track', track_uuid, 'annotation')
            ann_path = os.path.join(self._get_room_dir(room_name), 'track', track_uuid, 'annotation', ann_filename)
            
            if os.path.isfile(ann_path):
                self.LOG.info('removing annotation data file: {f}'.format(f=ann_path))
                os.remove(ann_path)



    def delete_room(self, room_name):

        self.LOG.info('delete_room(): {rn}'.format(rn=room_name))

        room_dir = self._get_room_dir(room_name)

        self.LOG.debug('walking {rd}'.format(rd=room_dir))

        if not os.path.isdir(room_dir):
            self.LOG.warning('Room dir not found {d}, skipping'.format(d=room_dir))
            return False

        for root, dirs, files in os.walk(room_dir):
            for file_entry in files:
                file_path = os.path.join(root, file_entry)
                self.LOG.info('Deleting {f}'.format(f=file_path))
                os.remove(file_path)

        for root, dirs, files in os.walk(room_dir, topdown=False):

            self.LOG.debug('delete_room(): Deleting {r}'.format(r=root))
            os.rmdir(root)
            

        return True


    def rename_room(self, room_name, new_room_name):

        room_dir = self._get_room_dir(room_name)
        new_room_dir = self._get_room_dir(new_room_name)

        # update metadata
        room_metadata_path = os.path.join(room_dir, 'room.json')
        new_md = None
        with open(room_metadata_path) as fh:
            new_md = json.load(fh)
            new_md['room_name'] = new_room_name
            new_md['room_dirname'] = rlp.core.util.slugify(new_room_name)

        if new_md:
            with open(room_metadata_path, 'w') as wfh:
                wfh.write(json.dumps(new_md, indent=2))

                self.LOG.info('Wrote {}'.format(room_metadata_path))


        room_session_path = os.path.join(room_dir, '{}.rls'.format(room_name))
        new_room_session_path = os.path.join(room_dir, '{}.rls'.format(new_room_name))

        self.LOG.info('{} -> {}'.format(room_session_path, new_room_session_path))
        os.rename(room_session_path, new_room_session_path)

        self.LOG.info('{} -> {}'.format(room_dir, new_room_dir))
        os.rename(room_dir, new_room_dir)

        self.room_info[new_room_name] = self.room_info[room_name]

        self.LOG.info('Rename Done')

        return True


    def add_client(self, room_name, client_name, client_info=None):
        
        client_info = client_info or {}
        self.room_info[room_name]['clients'][client_name] = client_info
        self.room_info[room_name]['clients'][client_name].update(
            {
                'joined_at': time.time()
            })

        return True

    def remove_client(self, room_name, client_name):

        result = False
        if room_name in self.room_info and client_name in self.room_info[room_name]:
            del self.room_info[room_name][client_name]
            result = True

        return result


    def get_clients(self, room_name=None):

        if room_name and room_name in self.room_info:
            return list(self.room_info[room_name]['clients'].keys())

        else:
            client_list = []
            for room_name in self.room_info:
                client_list.extend(self.room_info[room_name]['clients'].keys())

            return client_list