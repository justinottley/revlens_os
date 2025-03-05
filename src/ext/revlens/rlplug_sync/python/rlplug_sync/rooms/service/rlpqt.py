
import uuid
import json
import pprint
import logging
import traceback

from ..repo.repo_filesys import FilesysRoomsRepository

from rlp import QtCore

import revlens_prod.site.server

class RoomServerError(Exception): pass
class InvalidMethodError(RoomServerError): pass
class InvalidRoomError(RoomServerError): pass
class InvalidClientError(RoomServerError): pass
class NoActionError(RoomServerError): pass
class InvalidActionError(RoomServerError): pass


class RlpQtRoomServerController(object):

    _INSTANCE = None

    def __init__(self):
        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

    @classmethod
    def instance(cls):
        if cls._INSTANCE is None:
            cls._INSTANCE = cls()

        return cls._INSTANCE

    @property
    def EDBC(self):
        return revlens_prod.site.server.EDBC

    def repo(self, site_name):
        return FilesysRoomsRepository.instance(site_name, self.EDBC)


    def build_message(self, method, args=None, kwargs=None, session=None):
        
        if args is None:
            args = {}
            
        if kwargs is None:
            kwargs = {}
            
        if session is None:
            session = {}
        
        # session['key'] = API_KEY
        # session['app_name'] = self.app_name

        msg_dict = {
            'method': method,
            'args': args,
            'kwargs': kwargs,
            'session': session
        }

        return msg_dict

        # msg_str = json.dumps(msg_dict)
        # return msg_str


    def build_return(self, return_status, result, session_kwargs=None, do_broadcast=None, broadcast_clients=None):
        
        session_kwargs = session_kwargs or {}
        # session_kwargs['app_name'] = self.app_name
        broadcast_clients = broadcast_clients or []

        msg_dict = {
            'status': return_status,
            'result': result,
            'session': session_kwargs
        }

        if do_broadcast: # contains broadcast group (uuid)
            self.LOG.debug('build_return: GOT BROADCAST!!!!!!!!!!!!!')
            msg_dict['broadcast'] = do_broadcast # broadcast group (uuid)
            msg_dict['broadcast.clients'] = broadcast_clients # set of clients to force deliver message
        
        # self.LOG.info('build_return(): {}'.format(pprint.pformat(msg_dict)))

        return msg_dict


    def broadcast(self, method, broadcast_group, args=None, kwargs=None, session_kwargs=None, broadcast_clients=None):

        msg_dict = self.build_message(method, args, kwargs, session_kwargs)
        return self.build_return('OK', msg_dict, do_broadcast=broadcast_group, broadcast_clients=broadcast_clients)



    def _handle_delete_room(self, session_kwargs, *args, **kwargs):
        result = self.repo(session_kwargs.get('site_name')).delete_room(kwargs.get('room_name'))
        result_dict = self.build_return('ok', result, {'from_method': 'delete_room'})
        # self.send(result_str, False)

        return result_dict


    def _handle_rename_room(self, session_kwargs, *args, **kwargs):

        repo = self.repo(session_kwargs.get('site_name'))

        result = repo.rename_room(
            kwargs.get('room_name'),
            kwargs.get('new_room_name')
        )

        if result:
            
            self.LOG.info('Updating Room Entity')
            update_result = self.EDBC.update(
                'Room',
                kwargs['room_uuid'],
                {'name': kwargs['new_room_name']}
            )

            self.LOG.debug(update_result)

        return self.build_return('ok', result, {'from_method': 'rename_room'})


    def _handle_create_room(self, session_kwargs, *args, **kwargs):

        room_name = kwargs.get('room_name')
        session_data = kwargs.get('session_data')

        if not room_name:
            self.LOG.error('No room_name, cannot create! got {}'.format(kwargs))
            return


        self.LOG.info('creating room "{}"'.format(room_name))


        #
        # Create EDBC entity
        #
        edbc_result = self.EDBC.create(
            'Room',
            room_name,
            {},
            ''
        )

        self.LOG.info(edbc_result)

        room_uuid = None
        if edbc_result:
            room_uuid = edbc_result

        result = False
        try:
            result = self.repo(session_kwargs.get('site_name')).create_room(
                room_name, room_uuid, session=session_data)
        
        except:
            self.LOG.error(traceback.format_exc())


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

        return self.broadcast('create_room', room_uuid, args=[], kwargs=bkwargs, session_kwargs=session_kwargs)


    def _handle_update_room_session(self, session_kwargs, action, *args, **kwargs):

        self.LOG.debug('update_room_session(): {sk} "{a}" {ag} {k}'.format(
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
            room_uuid = session_kwargs.get('room_uuid')
            if kwargs.get('update_info'):
                update_info = kwargs['update_info']

            if not room_uuid:
                room_uuid = update_info['room_uuid']


            result = self.repo(session_kwargs.get('site_name')).update_room(
                session_kwargs.get('room_name'),
                action,
                update_info
            )

            if action == 'message':
                
                self.LOG.info('Got message, attempting DB push')

                # record message in db
                msg_name = 'msg_{}'.format(str(uuid.uuid4()).replace('{', '').replace('}', ''))

                # msg_result = self.EDBC.create(
                #     'Message',
                #     msg_name,
                #     {
                #         'text': update_info['text'],
                #         'room_link': {'uuid':room_uuid},
                #         'username': update_info['username'],
                #         'user_link': {'uuid':update_info['user_info']['uuid']},
                #         'created_at': update_info['create_time']
                #     },
                #     ''
                # )

                # self.LOG.info(msg_result)


            if (action != 'mediaUpdate'):
                return self.broadcast('update_room_session', room_uuid, [result], update_kwargs, session_kwargs=session_kwargs)

        except:
            self.LOG.error(traceback.format_exc())


    def _handle_join_room(self, session_kwargs, *args, **kwargs):

        self.LOG.info('_handle_join_room() {a} {k} {s}'.format(
            a=args, k=kwargs, s=session_kwargs
        ))

        room_name = kwargs.get('room_name')
        room_uuid = kwargs['room_uuid']
        client_username = session_kwargs.get('client.user')
        client_ident = session_kwargs['client.ident']

        revlens_prod.site.server.CNTRL.wsServer().addClientToBroadcastGroup(
            client_ident,
            room_uuid
        )

        result = self.repo(session_kwargs.get('site_name')).add_client(room_name, client_username)
        if result:
            # self.room_name = room_name
            session_kwargs['room_name'] = room_name
            self.LOG.info('joined room: {}'.format(room_name))

        return self.broadcast('join_room', room_uuid, [result], session_kwargs)


    def _handle_load_room(self, session_kwargs, *args, **kwargs):

        print('_handle_load_room: {} {} {}'.format(session_kwargs, args, kwargs))
        try:
            room_name = kwargs.get('room_name')
            sync = kwargs.get('sync')

            session_data = self.repo(session_kwargs.get('site_name')).get_room_session_data(room_name)
            
            if sync:
                self._handle_join_room(session_kwargs, *args, **kwargs)

            return self.build_return('ok', session_data, {
                    'from_method': 'load_room',
                    'sync': sync
                })

        except:
            print(traceback.format_exc())


    def _handle_leave_room(self, session_kwargs, *args, **kwargs):

        room_name = session_kwargs.get('room_name')
        room_uuid = session_kwargs.get('room_uuid')
        client_ident = session_kwargs.get('client.ident')

        if room_uuid and client_ident:

            self.LOG.info('Leave room: {} - {}'.format(room_name, room_uuid))

            result = self.repo(session_kwargs.get('site_name')).remove_client(
                room_name,
                session_kwargs.get('client.user')
            )

            revlens_prod.site.server.CNTRL.wsServer().removeClientFromBroadcastGroup(
                client_ident,
                room_uuid
            )

            return self.broadcast('leave_room', room_uuid, [result], session_kwargs, broadcast_clients=[client_ident])



    def _handle_clear_room(self, session_kwargs):
        # self.rooms[room_name].clear()
        # self.broadcast('clear_room', [room_name])
        pass


    def _handle_get_rooms(self, session_kwargs, *args, **kwargs):

        self.LOG.debug('_handle_get_rooms()')

        result = self.repo(session_kwargs.get('site_name')).get_rooms()
        result_dict = self.build_return('ok', result, {'from_method': 'get_rooms'})

        # self.send(result_str, False)
        return result_dict


    def _handle_get_clients(self, session_kwargs, *args, **kwargs):
        
        self.LOG.debug('_handle_get_clients()')

        result = self.repo(session_kwargs.get('site_name')).get_clients(kwargs.get('room_name'))
        result_dict = self.build_return('ok', result, {'from_method': 'get_clients'})
    
        return result_dict


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

            return self._handle_update_room_session(session_kwargs, action, **action_kwargs)

        else:
            room_uuid = session_kwargs['room_uuid']
            broadcast_kwargs = {
                'from_method': action,
                'action_kwargs': action_kwargs
            }

            return self.broadcast('client_cmd', room_uuid, [], broadcast_kwargs, session_kwargs)


    def _handle_pass(self, session_kwargs, *args, **kwargs):
        pass


    def handle_message(self, session_kwargs, method, *args, **kwargs):

        # self.LOG.debug('')
        # self.LOG.debug('{} args:{} kwargs:{}'.format(method, args, kwargs))
        # self.LOG.debug(pprint.pformat(session_kwargs))
        # self.LOG.debug('')

        # print('handle_message')
        # print(args)

        aargs = args[0]
        akwargs = args[1]

        # print('handle_message(): {m}\nARGS:\n{a}\nKWARGS:\n{k}\nSESSION:\n{s}\n'.format(
        #     m=method, a=pprint.pformat(args), k=pprint.pformat(kwargs), s=pprint.pformat(session_kwargs)))

        handler_name = '_handle_{m}'.format(m=method)
        if hasattr(self, handler_name):
            return getattr(self, handler_name)(session_kwargs, *aargs, **akwargs)

        else:
            raise InvalidMethodError(method)




def handle_message(*args, **kwargs):
    return RlpQtRoomServerController.instance().handle_message(*args, **kwargs)