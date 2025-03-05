
import os
import json
import logging

import revlens

ONE_MILLION = 1000000

class SettingsController:

    _INSTANCE = None

    SETTING_LOOKAHEAD_FILE = 'vlookahead.json'
    SETTINGS_SYNC = 'sync.json'

    def __init__(self):

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.state_root_dir = revlens._get_state_dir()

        self.settings_lookahead = {}


    @classmethod
    def instance(cls):
        if cls._INSTANCE is None:
            cls._INSTANCE = cls()

        return cls._INSTANCE

    @property
    def cache_lookahead(self):
        return revlens.CNTRL.getVideoManager().cache()
        
    
    def load_state(self):

        self.LOG.info('loading state')

        self.load_state_lookahead()



    def get_settings(self, settings_file):

        file_path = os.path.join(self.state_root_dir, settings_file)
        if os.path.isfile(file_path):
            with open(file_path) as fh:
                return json.load(fh)


    def load_state_lookahead(self):
        vl_cache_state_path = os.path.join(self.state_root_dir, self.SETTING_LOOKAHEAD_FILE)
        if os.path.isfile(vl_cache_state_path):
            try:
                with open(vl_cache_state_path) as fh:
                    self.settings_lookahead = json.load(fh)
                    

                    # Load
                    backward_frame_count = self.settings_lookahead.get('backward_frame_count')
                    if backward_frame_count:

                        self.LOG.info('loading setting: video lookahead backward frame count: {}'.format(backward_frame_count))
                        self.cache_lookahead.setMaxBackwardFrameCount(backward_frame_count)


                    cache_size = self.settings_lookahead.get('cache_size')
                    if cache_size:

                        self.LOG.info('loading setting: video lookahead cache size: {}'.format(cache_size))
                        self.cache_lookahead.setMaxSize(cache_size * ONE_MILLION)


            except:
                self.LOG.error('could not load lookahead saved state')
                self.LOG.error(traceback.format_exc())


