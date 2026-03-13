
import uuid
import json
import time
import pprint
import logging

import rlp_util

from rlp_core.remoting.websocket.rlpqt import RlpQtWsClient


class EdbWsClient(RlpQtWsClient):
    '''
    NOTE: basically deprecated. Using base class directly
    '''

    def _refresh_cache(self):
        method_name = '{}refreshCache'.format(self.method_prefix)
        return self._rpc(method_name)


    def get_entity_types(self):
        method_name = '{}getEntityTypes'.format(self.method_prefix)
        return self._rpc(method_name)

    def create(self, e_type, name, fields, uuid=""):
        args = [e_type, name, fields, uuid]
        method_name = '{}create'.format(self.method_prefix)
        return self._rpc(method_name, args)

    def update(self, e_type, e_uuid, fields):
        args = [e_type, e_uuid, fields]
        return self._rpc('update', args)

    def delete_entity(self, e_type, e_uuid):
        args = [e_type, e_uuid]
        method_name = '{}deleteEntity'.format(self.method_prefix)
        return self._rpc(method_name, args)

    def find(self, entity_name, filters, return_fields, limit=-1, order_by=None):
        order_by = order_by or []
        args = [entity_name, filters, return_fields, limit, order_by]
        method_name = '{}find'.format(self.method_prefix)
        return self._rpc(method_name, args)

    def create_entity_type(self, etype_name, parent=None):
        args = [etype_name, parent]
        method_name = '{}createEntityType'.format(self.method_prefix)
        return self._rpc(method_name, args)

    def create_entity_type_field(self, entity_name, field_name, display_name, field_type, val_type, metadata=None):
        
        if not metadata:
            metadata = {}

        args = [entity_name, field_name, display_name, field_type, val_type, metadata]
        method_name = '{}createEntityTypeField'.format(self.method_prefix)
        return self._rpc(method_name, args)

    def delete_entity_type_field(self, entity_name, field_name):
        args = [entity_name, field_name]
        method_name = '{}deleteEntityTypeField'.format(self.method_prefix)
        return self._rpc(method_name, args)


    def get_entity_type_fields(self, e_type, parents=True):
        args = [e_type, parents]
        method_name = '{}getEntityTypeFields'.format(self.method_prefix)
        return self._rpc(method_name, args)