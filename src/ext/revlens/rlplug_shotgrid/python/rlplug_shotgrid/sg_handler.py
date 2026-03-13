
import os
import json
import logging

import shotgun_api3

import rlp.util as RlpUtil
import rlp.util.config

LOG = logging.getLogger('rlp.{}'.format(__name__))


class ShotgunHandler:

    _INSTANCE = {}

    _DEFAULT_NAME = None

    _PROJECT_CACHE = {
        'name': {},
        'id': {}
    }

    RETRY_MAX = 3

    def __init__(self, url, user, key):

        self.LOG = logging.getLogger('rlp.{}.{}'.format(
            __name__, self.__class__.__name__))

        self.username = self._get_username()

        self.LOG.debug('Creating ShotGrid API instance: {}'.format(url))

        self.url = url
        self.user = user
        self.sg = shotgun_api3.Shotgun(url, user, key)

        self.LOG.debug('ShotGrid OK: {}'.format(self.url))


    @classmethod
    def init(cls, name, url, user, key):
        if name not in cls._INSTANCE:

            sgh = cls(url, user, key)
            cls._INSTANCE[name] = sgh
            cls._DEFAULT_NAME = name

        return cls._INSTANCE[name]


    @classmethod
    def instance(cls, name=None):
        if name:
            return cls._INSTANCE[name]
        
        elif cls._DEFAULT_NAME:
            return cls._INSTANCE[cls._DEFAULT_NAME]

        print('WARNING: ShotGrid Not Initialized')
        return None


    def find(self, *args, **kwargs):
        for _ in range(self.RETRY_MAX):
            try:
                return self.sg.find(*args, **kwargs)

            except Exception as e:
                self.LOG.error('Error querying ShotGrid: {}'.format(e))


    def _get_username(self):
        try:
            return RlpUtil.APPGLOBALS.globals()['username']

        except:
            return os.getenv('USER', os.getenv('USERNAME'))



def _get_site_config_path():

    if os.getenv('REVLENS_SHOTGRID_CONFIG_PATH'):
        return os.getenv('REVLENS_SHOTGRID_CONFIG_PATH')


    site_config_dir = os.path.join(
        rlp.util.config.CONFIG_DIR,
        'shotgrid'
    )

    if not os.path.isdir(site_config_dir):
        os.makedirs(site_config_dir)

    site_config_path = os.path.join(
        site_config_dir,
        'shotgrid_site.json'
    )

    return site_config_path


def write_site_config(site_config):

    config_path = _get_site_config_path()
    with open(config_path, 'w') as wfh:
        wfh.write(json.dumps(site_config, indent=2))

        LOG.info('Wrote {}'.format(config_path))


def read_site_config():

    config_path = _get_site_config_path()
    if os.path.isfile(config_path):
        LOG.debug('Reading ShotGrid config: {}'.format(config_path))
        with open(config_path) as fh:
            site_config = json.load(fh)
            return site_config
        
    else:
        LOG.error('ShotGrid config file not found: {}'.format(config_path))


def init_sg_revlens(sg_config_type=None, site_config=None, force=False, update=True):

    if not force and ShotgunHandler._INSTANCE.get(sg_config_type) != None:
        LOG.warning('ShotGrid API already initialized for {}, skipping'.format(sg_config_type))
        return

    if site_config is None:
        site_config = read_site_config()

    if not site_config or 'configs' not in site_config:
        LOG.error('Error: No ShotGrid site config, aborting')
        return

    if not sg_config_type:
        if 'default_config' in site_config:
            sg_config_type = site_config['default_config']

        else:
            sg_config_type = 'prod_onprem'

    sg_config = site_config['configs'][sg_config_type]

    ShotgunHandler.init(
        sg_config_type,
        sg_config['sg_url'],
        sg_config['api_user'],
        sg_config['api_key']
    )

    if update:
        site_config['default_config'] = sg_config_type
        write_site_config(site_config)


    # sg_remote_file_locator = ShotgunRemoteFileLocator()
    # revlens.media.LOCATORS.append(sg_remote_file_locator)
