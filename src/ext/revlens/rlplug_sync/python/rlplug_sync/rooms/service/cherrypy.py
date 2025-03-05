'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import os
import json
import pprint
import logging
import traceback

import cherrypy

from rlp_core.rlp_logging import basic_config
from rlp_core.remoting.websocket import JsonWebSocket, InvalidMethodError


from ..repo.repo_filesys import FilesysRoomsRepository


class RoomServerError(Exception): pass
class InvalidRoomError(RoomServerError): pass
class InvalidClientError(RoomServerError): pass
class NoActionError(RoomServerError): pass
class InvalidActionError(RoomServerError): pass

class RoomServerWebSocket(JsonWebSocket):

    EDBC = None

    def __init__(self, *args, **kwargs):
        JsonWebSocket.__init__(self, *args, **kwargs)
        self.app_name = 'rooms'

        # This server side websocket object is a handle for a single
        # corresponding client connection, so it is okay to store
        # the room name
        #
        self.room_name = None

    def repo(self, site_name):
        return FilesysRoomsRepository.instance(site_name, self.EDBC)

    def broadcast(self, method, args=None, kwargs=None, session_kwargs=None):
        if kwargs is None:
            kwargs = {}
            
        if session_kwargs is None:
            session_kwargs = {}
            
        session_kwargs['broadcast'] = True

        #
        # self.logger.debug('broadcast: {m}\nARGS:\n  {a}\nKWARGS:\n  {k}\nSESSION:\n  {s}\n'.format(
        #     m=method, a=args, k=pprint.pformat(kwargs), s=pprint.pformat(session_kwargs)
        # ))
        #

        cherrypy.engine.publish('websocket-broadcast',
            self.build_message(
                method,
                args,
                kwargs,
                session_kwargs),
            room_name=session_kwargs.get('room_name'))


    def _handle_delete_room(self, session_kwargs, *args, **kwargs):
        result = self.repo(session_kwargs.get('site_name')).delete_room(kwargs.get('room_name'))
        result_str = self.build_return('ok', result, {'from_method': 'delete_room'})
        self.send(result_str, False)


    def _handle_rename_room(self, session_kwargs, *args, **kwargs):

        repo = self.repo(session_kwargs.get('site_name'))

        result = repo.rename_room(
            kwargs.get('room_name'),
            kwargs.get('new_room_name')
        )

        if result:
            
            self.logger.info('Updating Room Entity')
            update_result = self.EDBC.update('Room', kwargs['room_uuid'], {'name': kwargs['new_room_name']})

            self.logger.debug(update_result)

        result_str = self.build_return('ok', result, {'from_method': 'rename_room'})

        self.send(result_str, False)


    def _handle_create_room(self, session_kwargs, *args, **kwargs):

        room_name = kwargs.get('room_name')
        session_data = kwargs.get('session_data')

        if not room_name:
            self.logger.error('No room_name, cannot create! got {}'.format(kwargs))
            return


        self.logger.info('{pa}: creating room "{n}"'.format(
            pa=self.peer_address, n=room_name))


        #
        # Create EDBC entity
        #
        edbc_result = self.EDBC.create('Room', room_name, {})
        self.logger.info(edbc_result)

        room_uuid = None
        if edbc_result['status'] == 'OK':
            room_uuid = edbc_result['result']

        result = False
        try:
            result = self.repo(session_kwargs.get('site_name')).create_room(
                room_name, room_uuid, session=session_data)
        
        except:
            self.logger.error(traceback.format_exc())


        bkwargs = {
            'from_method': 'create_room',
            'action_kwargs': {
                'result': result,
                'room_name': room_name,
                'room_uuid': room_uuid
            }
        }

        # Need this to flag to lower levels to broadcast to everyody in every room
        #
        session_kwargs['room_name'] = None

        self.broadcast('create_room', args=[], kwargs=bkwargs, session_kwargs=session_kwargs)


    def _handle_update_room_session(self, session_kwargs, action, *args, **kwargs):

        self.logger.debug('update_room_session(): {sk} "{a}" {ag} {k}'.format(
            sk=session_kwargs, a=action, ag=args, k=kwargs
        ))
        
        try:

            update_kwargs = kwargs.copy()
            # update_kwargs['session_data'] = kwargs.get('session_data')
            update_kwargs['action'] = action

            #
            # TODO FIXME: !!!! FIX THIS!!! INTERFACE SPEC!!!
            #
            update_info = kwargs
            if kwargs.get('update_info'):
                update_info = kwargs['update_info']

            result = self.repo(session_kwargs.get('site_name')).update_room(
                session_kwargs.get('room_name'),
                action,
                update_info
            )

            if action == 'message':
                
                self.logger.info('Got message, attempting DB push')

                # record message in db
                msg_result = self.EDBC.create('Message',
                    None,
                    {
                        'text': update_info['text'],
                        'room_link': {'uuid':update_info['room_uuid']},
                        'username': update_info['username'],
                        'user_link': {'uuid':update_info['user_info']['uuid']},
                        'created_at': update_info['create_time']
                    }
                )

                self.logger.info(msg_result)


            if (action != 'mediaUpdate'):
                self.broadcast('update_room_session', [result], update_kwargs, session_kwargs=session_kwargs)

        except:
            self.logger.error(traceback.format_exc())


    def _handle_join_room(self, session_kwargs, *args, **kwargs):
        
        self.logger.info('_handle_join_room() {a} {k} {s}'.format(
            a=args, k=kwargs, s=session_kwargs
        ))

        room_name = kwargs.get('room_name')
        client_username = session_kwargs.get('client.user')

        result = self.repo(session_kwargs.get('site_name')).add_client(room_name, client_username)
        if result:
            self.room_name = room_name
            session_kwargs['room_name'] = room_name
            self.logger.info('joined room: {}'.format(room_name))

        self.broadcast('join_room', [result], session_kwargs)


    def _handle_load_room(self, session_kwargs, *args, **kwargs):

        try:
            room_name = kwargs.get('room_name')
            sync = kwargs.get('sync')

            session_data = self.repo(session_kwargs.get('site_name')).get_room_session_data(room_name)
            
            if sync:
                self._handle_join_room(session_kwargs, *args, **kwargs)

            self.send(self.build_return('ok', session_data, {
                    'from_method': 'load_room',
                    'sync': sync
                }),
                False)
        except:
            self.logger.error(traceback.format_exc())


    def _handle_leave_room(self, session_kwargs, *args, **kwargs):

        room_name = kwargs.get('room_name')

        if room_name:
            self.logger.info('Leave room: {}'.format(room_name))

            result = self.repo(session_kwargs.get('site_name')).remove_client(
                room_name,
                session_kwargs.get('client.user')
            )

            self.broadcast('leave_room', [result], session_kwargs)
            self.room_name = None



    def _handle_clear_room(self, session_kwargs):
        # self.rooms[room_name].clear()
        # self.broadcast('clear_room', [room_name])
        pass


    def _handle_get_rooms(self, session_kwargs, *args, **kwargs):

        self.logger.debug('{pa}: _handle_get_rooms()'.format(pa=self.peer_address))

        result = self.repo(session_kwargs.get('site_name')).get_rooms()
        result_str = self.build_return('ok', result, {'from_method': 'get_rooms'})

        self.send(result_str, False)


    def _handle_get_clients(self, session_kwargs, *args, **kwargs):
        
        self.logger.debug('{pa}: _handle_get_clients()'.format(pa=self.peer_address))

        result = self.repo(session_kwargs.get('site_name')).get_clients(kwargs.get('room_name'))
        result_str = self.build_return('ok', result, {'from_method': 'get_clients'})
    
        self.send(result_str, False)


    def _handle_client_cmd(self, session_kwargs, action, action_kwargs=None):

        if not action:
            raise NoActionError('action not supplied')

        if not action_kwargs:
            action_kwargs = {}

        if action in ['addTrack',
                      'deleteTrack',
                      'insert_media_at_frame',
                      'load_media_list',
                      'load_previous_shot',
                      'mediaRemoved',
                      'mediaUpdate',
                      'holdFrame',
                      'releaseFrame',
                      'clearAnnotations',
                      'message']:
            self._handle_update_room_session(session_kwargs, action, **action_kwargs)

        else:
            broadcast_kwargs = {
                'from_method': action,
                'action_kwargs': action_kwargs
            }

            self.broadcast('client_cmd', [], broadcast_kwargs, session_kwargs)


    def _handle_pass(self, session_kwargs, *args, **kwargs):
        pass


    def handle_message(self, session_kwargs, method, *args, **kwargs):

        #
        # self.logger.debug('handle_message(): {m}\nARGS:\n{a}\nKWARGS:\n{k}\nSESSION:\n{s}\n'.format(
        #   m=method, a=pprint.pformat(args), k=pprint.pformat(kwargs), s=pprint.pformat(session_kwargs)))
        #

        handler_name = '_handle_{m}'.format(m=method)
        if hasattr(self, handler_name):
            getattr(self, handler_name)(session_kwargs, *args, **kwargs)

        else:
            raise InvalidMethodError(method)

