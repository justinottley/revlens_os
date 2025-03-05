
import os
import json
import pprint
import logging
import traceback


import revlens

class MediaLocator(object):

    CONTAINER_FORMATS = ['.mov', '.avi', '.mp4', '.ogv', '.m4v', '.mkv', '.mj2']
    FRAME_FORMATS = ['.jpg', '.png', '.tif', '.tiff', '.exr', '.dpx', '.cin', '.tif', '.tiff', '.psd']
    AUDIO_FORMATS = ['.wav', '.aac', '.mp3', '.m4a', '.aiff']

    def __init__(self):
        self.LOG = logging.getLogger('rlp.{ns}.{cls}'.format(
            ns=self.__class__.__module__, cls=self.__class__.__name__
        ))

    @property
    def media_factory(self):
        return revlens.CNTRL.getMediaManager().getMediaFactory()


    def _audio_locator(self, media_input, result):
        for media_ext in self.AUDIO_FORMATS:
            if media_input and media_input.lower().endswith(media_ext):

                self.LOG.debug('_audio_locator(): found')

                result['media.audio.components'].append('audio_file')
                result['media.audio.component.audio_file.path'] = media_input
                result['media.audio.component.audio_file.name'] = os.path.basename(media_input)


    def _video_filesequence_locator(self, media_input, result):

        for media_ext in self.FRAME_FORMATS:
            if media_input and media_input.lower().endswith(media_ext):

                self.LOG.debug('_video_filesequence_locator(): found')

                result['media.video.components'].append('frames')
                result['media.video.component.frames.path'] = media_input
                result['media.video.component.frames.name'] = os.path.basename(media_input)


    def _container_media_locator(self, media_input, result):

        self.LOG.debug('_container: checking {} formats: {}'.format(
            media_input, self.CONTAINER_FORMATS))

        for media_ext in self.CONTAINER_FORMATS:
            if media_input and media_input.lower().endswith(media_ext):
                
                self.LOG.debug('_container_media_locator(): found')

                for component_type in ['video', 'audio']:
                    result['media.{}.components'.format(component_type)].append('movie')
                    result['media.{}.component.movie.path'.format(component_type)] = media_input
                    result['media.{}.component.movie.name'.format(component_type)] = os.path.basename(media_input)


    def _resolve_component(self, media_dict, component_type, search_path):

        self.LOG.debug('_resolve_component(): {ct} {sp}'.format(ct=component_type, sp=search_path))

        locator_key = 'media.{}.locator'.format(component_type)
        if media_dict.get(locator_key):
            self.LOG.debug('{}: already resolved with locator "{}" - skipping'.format(
                component_type, media_dict[locator_key]
            ))
            
            return False

        for search_item_entry in search_path:

            component_path_key = 'media.{}.component.{}.path'.format(
                component_type,
                search_item_entry)

            component_name_key = 'media.{}.component.{}.name'.format(
                component_type,
                search_item_entry)
    
            component_path_result = media_dict.get(component_path_key)
            component_name_result = media_dict.get(component_name_key)

            if component_path_result:
                if '{f}' in component_path_result or \
                    '%04d' in component_path_result or \
                    '#' in component_path_result or \
                    (os.path.isfile(component_path_result) and \
                     os.stat(component_path_result).st_size > 0):

                    self.LOG.debug('_resolve_component(): {} got {}'.format(
                        component_path_key, component_path_result
                    ))

                    media_dict['media.{}'.format(component_type)] = component_path_result
                    media_dict['media.name'] = component_name_result # TODO FIXME: overwritten
                    media_dict['media.{}.resolved_component'.format(component_type)] = search_item_entry

                return True

            else:
                self.LOG.error('_resolve_component(): no result from key {}'.format(component_name_key))

        return False


    def resolve_components(self, media_input):
        
        return_status = False

        component_result_map = {
            'video': False,
            'audio': False
        }
        for component_type in component_result_map.keys():
                
            component_pref_key = '{}.component_search_path'.format(component_type)

            _raw_val = self.media_factory.getMediaPref(component_pref_key)

            component_type_search_path = ['frames', 'movie', 'audio_file']

            resolve_result = self._resolve_component(media_input, component_type, component_type_search_path)
            component_result_map[component_type] = resolve_result
        
        # return true if we have at least one component
        for component_type, result in component_result_map.items():
            if result:
                return_status = True
                break

        return (return_status, media_input)


    def _locate(self, media_input, result):
        
        self.LOG.debug('_locate(): {}'.format(media_input))

        for locator_func in [
            self._container_media_locator,
            self._video_filesequence_locator,
            self._audio_locator]:

            self.LOG.debug('Trying locator {}'.format(locator_func))
            locator_func(media_input, result)

        self.LOG.debug('_locate(): result: {}'.format(pprint.pformat(result)))

        if result['media.video.components'] or result['media.audio.components']:
            return result


    def locate(self, media_input, resolve_components=True):
        '''
        Locate and then identify actual media source to use based on component search path preference
        '''

        media_input_cls_name =  media_input.__class__.__name__

        self.LOG.info('locate(): {} {} resolve_components:{}'.format(
            '', media_input_cls_name, resolve_components))
        # self.LOG.info('--->\n{}'.format(pprint.pformat(media_input)))

        path_input = media_input
        media_result = {
            'media.metadata': {},
            'media.video.components': [],
            'media.audio.components': []
        }

        if media_input_cls_name in ['dict', 'OrderedDict']:
            if 'path' in media_input:
                path_input = media_input['path']

            elif 'media.video.components' in media_input or 'media.audio.components' in media_input:
                if resolve_components:
                    (return_status, media_result) = self.resolve_components(media_input)
                    if return_status:
                        return media_result


            else:

                print('ERROR: INVALID INPUT, "path" EXPECTED')
                pprint.pprint(media_input)
                # call this an error - we need a path to a file on disk at this point
                return None

        # self.LOG.debug('locate(): path_input: {}'.format(path_input))

        path_input_cls_name = path_input.__class__.__name__
        if path_input_cls_name == 'str':
            result = self._locate(path_input, result=media_result)

            if result:

                if 'graph.uuid' in media_input:
                    graph_uuid = media_input['graph.uuid'].toString()
                    if graph_uuid[0] != '{': # TODO FIXME FIX THIS!!! switch graph uuid format
                        graph_uuid = '{' + media_input['graph.uuid'].toString() + '}'

                    result['graph.uuid'] = graph_uuid

                if resolve_components:
                    (return_status, media_result) = self.resolve_components(result)

                    self.LOG.debug('GOT RESULT: {}\n{}'.format(return_status, pprint.pformat(media_result)))

                    if return_status:
                        return media_result
                
                else:
                    return result

            else:
                self.LOG.error('no result from _locate(), skipping resolve_components()')
        else:
            self.LOG.error('invalid input type for path_input, cannot locate')


class RemoteFileLocator(MediaLocator):
    '''
    Attempt to download a file from a service if not found on locally accessible filesystem
    '''

    def __init__(self, download=True, source_name='Filesys'):
        MediaLocator.__init__(self)

        self.download = download
        self.source_name = source_name


    def _progress_callback(self, source_label, bytes_read=None, bytes_total=None, completed_percent=None, progress_data=None):
        
        percent_str = ''
        if completed_percent:
            percent_str += '- {}%'.format(completed_percent)

        msg = 'Downloading from {}: {} {}'.format(
            self.source_name, source_label, percent_str
        )
        
        if revlens.CNTRL:
            
            import rlplug_qtbuiltin.cmds
            from PythonQt import QtCore
            rlplug_qtbuiltin.cmds.display_message(msg)
            QtCore.QCoreApplication.processEvents()


    def get_local_file_path(self, file_path, *args, **kwargs):

        import tempfile

        use_file_path = file_path
        while use_file_path[0] == '/':
            use_file_path = use_file_path[1:]

        
        local_path = os.path.join(
            tempfile.gettempdir(),
            'revlens',
            'file_cache',
            use_file_path
        )

        return local_path


    def download_file(self, file_path, media_dict=None, component_type=None):
        '''
        Download file to local file cache, with progress
        '''

        raise NotImplementedError()


    def can_download(self, src_path, media_dict, component_type, search_path):

        raise NotImplementedError()



    def _resolve_component(self, media_dict, component_type, search_path):
        MediaLocator._resolve_component(self, media_dict, component_type, search_path)


        locator_key = 'media.{}.locator'.format(component_type)
        if media_dict.get(locator_key):
            self.LOG.debug('{}: already resolved with locator "{}" - skipping'.format(
                component_type, media_dict[locator_key]
            ))
            
            return False

        media_key = 'media.{}'.format(component_type)

        media_value = media_dict.get(media_key)

        resolve_result = False

        if media_value:

            src_path = str(media_value) # str(Path(os.path.abspath(media_value)))
            src_path = src_path.replace('\\', '/')
            local_path = self.get_local_file_path(src_path, media_dict, component_type)

            if os.path.isfile(src_path):

                self.LOG.info('src path found: {}'.format(src_path))
                media_dict[media_key] = src_path


            elif local_path is not None and os.path.isfile(local_path):
                self.LOG.info('local cached path found: {}'.format(local_path))

                media_dict[media_key] = local_path
                resolve_result = True

            elif os.path.isfile(src_path):
                self.LOG.info('source path found: {}'.format(src_path))
                resolve_result = True

            elif self.download and self.can_download(src_path, media_dict, component_type, search_path):

                self.LOG.debug('file not found, attempting download: {}'.format(src_path))

                local_path = self.download_file(src_path, media_dict=media_dict, component_type=component_type)
                if local_path:
                    media_dict[media_key] = local_path
                    resolve_result = True

        else:
            self.LOG.debug('NO MEDIA VALUE FOR {}'.format(media_key))

        if resolve_result:
            media_dict[locator_key] = self.__class__.__name__

        return resolve_result


