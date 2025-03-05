
import json
import traceback
import requests

class EdbHttpError(Exception): pass


class EdbHttpClient(object):
    '''
    Client for web-based server

    http://192.168.1.174:8005/rlpsite/call
    '''
    def __init__(self, site_name, url, username, auth_key, certfile_path):
        self.__site_name = site_name
        self.__url = url
        self.__username = username
        self.__auth_key = auth_key
        self.__certfile_path = certfile_path

        self.__client_type = 'rpc.http'
    

    @property
    def site_name(self):
        return self.__site_name

    @property
    def url(self):
        return self.__url

    @property
    def username(self):
        return self.__username

    @property
    def auth_key(self):
        return self.__auth_key

    @property
    def certfile_path(self):
        return self.__certfile_path


    def client_type(self):
        return self.__client_type


    def _get_info(self):
        return {
            'endpoint_url': self.url,
            'username': self.username,
            'auth_key': self.auth_key,
            'certfile_path': self.certfile_path
        }


    def _handle_result(self, result):
        
        parsed_result = None
        try:
            parsed_result = json.loads(result.text)

        except:
            print(traceback.format_exc())
            print(result.text)
            raise EdbHttpError('Server side error')

        if parsed_result:
            if parsed_result['status'] == 'OK':
                return parsed_result['result']

            elif parsed_result['status'] == 'ERR':
                raise EdbHttpError(parsed_result['err_msg'])


    def get_entity_types(self):

        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'get_entity_types',
                'args': json.dumps([])
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)


    def create(self, e_type, e_name, fields, uuid=""):

        args = [e_type, e_name, fields, uuid]
        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'create',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)


    def update(self, e_type, e_uuid, fields):
        args = [e_type, e_uuid, fields]
        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'update',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)


    def delete_entity(self, e_type, e_uuid):
        args = [e_type, e_uuid]
        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'delete_entity',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)


    #
    # Entity Types
    #

    def create_entity_type(self, etype_name, etype_parent=None):

        args = [etype_name, etype_parent]
        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'create_entity_type',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)


    def create_entity_type_field(self,
        e_type,
        field_name,
        display_name,
        field_type,
        val_type):

        args = [e_type, field_name, display_name, field_type, val_type]
        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'create_entity_type_field',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)


    def delete_entity_type_field(self,
        e_type,
        field_name):

        args = [e_type, field_name]

        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'delete_entity_type_field',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)


    def get_entity_type_fields(self, e_type):
        args = [e_type]
        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'get_entity_type_fields',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        result = self._handle_result(result)

        # deserialize entity type field metadata
        #
        for field_data in result.values():
            if 'metadata' in field_data:
                field_data['metadata'] = json.loads(field_data['metadata'])

        return result


    #
    # Search
    #


    def find(self, e_type, filters, return_fields, limit=-1, order_by=None):
        
        order_by = order_by or []
        args = [e_type, filters, return_fields, limit, order_by]
        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': 'find',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)


    def refresh_cache(self):
        args = []
        result = requests.post(
            self.__url,
            params={
                'username': self.username,
                'auth_key': self.auth_key,
                'method': '_refresh_cache',
                'args': json.dumps(args)
            },
            verify=self.__certfile_path
        )

        return self._handle_result(result)

