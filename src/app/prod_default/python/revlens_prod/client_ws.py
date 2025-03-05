import logging
import traceback

import rlp.util as RlpUtil

import rlp.core.util

from rlp.core.net.websocket import RlpClient
from rlp.core.net.websocket.rlpqt import RlpQtWsClient


class SiteWsClient(RlpQtWsClient):

    _SITE_MAP = {}

    SCHEME = 'ws'

    def __init__(self, *args, **kwargs):
        RlpQtWsClient.__init__(self, encrypted=True, method_prefix='edbc.', *args, **kwargs)

        self.__site_name = None
        self.__username = None
        self.__auth_key = None
        self.__first_name = None
        self.__last_name = None
        self.__user_id = None

        self._login_info = None


    @property
    def site_name(self):
        return self.__site_name

    @property
    def username(self):
        return self.__username

    @property
    def auth_key(self):
        return self.__auth_key

    @property
    def first_name(self):
        return self.__first_name

    @property
    def last_name(self):
        return self.__last_name

    @property
    def user_id(self):
        return self.__user_id


    @property
    def user_info(self):
        return {
            'first_name': self.first_name,
            'last_name': self.last_name,
            'uuid': self.__user_id
        }


    @property
    def info(self):
        return {
            'site_name': self.site_name,
            'auth_key': self.auth_key,
            'username': self.username,
            'user_id': self.user_id,
            'first_name': self.first_name,
            'last_name': self.last_name
        }

    @classmethod
    def instance(cls):
        print('SiteWsClient.instance() DEPRECATED, use RlpClient.instance() instead')
        return RlpClient.instance()


    @classmethod
    def init(cls, site_hostinfo, force=False):
        '''
        site_hostinfo: HOSTNAME:PORT
        '''
        if force or site_hostinfo not in cls._SITE_MAP:
            site_url = site_hostinfo # 'ws://{}'.format(site_hostinfo)
            site_client = cls(site_url)

            cls._SITE_MAP[site_hostinfo] = site_client

        return cls._SITE_MAP[site_hostinfo]


    def on_connected(self):
        RlpQtWsClient.on_connected(self)

        if self._login_info:

            login_args = self._login_info[0]
            self.call(self._login_done, 'site.login', *login_args).run()


    def on_disconnected(self):
        RlpQtWsClient.on_disconnected(self)

        if self._login_info:
            func = rlp.core.util.import_function(self._login_info[1])
            func({'result': False, 'status': 'disconnected'})


    def _login_done(self, login_result):

        # self.__class__._INSTANCE = self
        RlpClient._INSTANCE = self

        if login_result.get('result'):

            login_result['connect_state'] = 'OK'

            self.__auth_key = login_result['auth_key']
            self.__username = login_result['username']
            self.__site_name = login_result['site_name']
            self.__first_name = login_result['first_name']
            self.__last_name = login_result['last_name']
            self.__user_id = login_result['user_id']

            self.LOG.info('{} Login OK - site_name: {} ident: {}'.format(
                self, self.site_name, self.ident
            ))

            # save client ident into appglobals
            RlpUtil.APPGLOBALS.update({'client.ident': self.ident})


        else:
            self.LOG.info('Login Failed')


        if self._login_info:
            login_callback_funcname = self._login_info[1]

            self.LOG.debug('Login callback: {}'.format(login_callback_funcname))
            if login_callback_funcname:
                func = rlp.core.util.import_function(login_callback_funcname)
                func(login_result)

            else:
                self.LOG.warning('No login callback')


    def login(self, username, password, callback=None):

        self.LOG.info('Login - callback: {}'.format(callback))

        args  = [username, password]
        self._login_info = (args, callback)

        if not self.connected:
            return self.connect()

        else:
            self.on_connected()
