'''
TODO FIXME: DEPRECATED / UNUSED
'''

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


