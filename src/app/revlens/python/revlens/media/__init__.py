'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.

{
    "media.video":
    "media.audio":
    "media.name":
}
'''

import os
import json
import pprint
import logging
import traceback

from rlp import QtCore
from rlp.Qt import QStr

import rlp.core as RlpCore
from rlp.core import rlp_logging

from rlp.core.util import catch_exception
import rlp.core.filesequence.cmds

rlogger = logging.getLogger('rlp.revlens.media')

try:
    import revlens
    import rlp.util

except:
    rlogger.warning(traceback.format_exc())

from .locator import MediaLocator, RemoteFileLocator

LOCATORS = [] # RemoteFileLocator()


def _init_locators():

    rlogger.info('_init_locators()')

    global LOCATORS
    # LOCATORS.append(RemoteFileLocator())
    LOCATORS.append(MediaLocator())


def sanitize_session(session_input, remove_paths=False):
    '''
    NOTE: although this is modifying the input in-place, it is also returned
    to pass back through the python binding to C++
    '''

    for track_info in list(session_input['track_data'].values()):
        track_media = track_info.get('media')
        if not track_media:
            continue

        for media_info in list(track_media.values()):
            
            for key, value in list(media_info['properties'].items()):

                if not key.startswith('media.'):
                    continue

                if remove_paths:
                    if key == 'path' or 'resolved_component' in key or 'locator' in key:
                        print('Removing {}'.format(media_info['properties'][key]))

                        del media_info['properties'][key]
                        continue

    return session_input


def resolve_components(media_input):
    '''
    Resolve media input dict containing potentially multiple components (frames, movie, etc)
    for a piece of media into a single one using configured component search path
    
    NOTE: although this is modifying the input in-place, it is also returned
    to pass back through the python binding to C++
    '''

    global LOCATORS

    result = None
    
    for locator in LOCATORS:
        rlogger.info('resolve_components(): trying locator {}'.format(locator))
        (return_status, result) = locator.resolve_components(media_input)

    return result



def locate(media_input, resolve_components=False):
    '''
    Run a series of locators to build a data dictionary from arbitrary media input
    media_input could be the full path to a media file
    '''

    global LOCATORS

    result = None

    if issubclass(media_input.__class__, dict) and 'path' in media_input:
        input_path = media_input['path']
        if input_path.endswith('.json'):
            with open(input_path) as fh:
                input_payload = json.load(fh)
                media_input.update(input_payload)
                del media_input['path']

    if issubclass(media_input.__class__, str) and media_input.endswith('.json'):
        with open(media_input) as fh:
            result = json.load(fh)
    
    else:

        rlogger.debug('Locators: {}'.format(LOCATORS))

        for locator in LOCATORS:
            
            rlogger.debug('Trying locator {l}'.format(l=locator))
            
            result = locator.locate(media_input, resolve_components=resolve_components)
            if result:

                rlogger.debug('Locator {} successful, returning:\n{}'.format(locator, pprint.pformat(result)))
                return result
                
            
    return result



@catch_exception
def load_media_list(media_input_list, track_idx=0, start_frame=1, clear=False, emit_sync_action=True, options=None):
    '''
    Insert a list of media items into a track back-to-back
    '''

    import revlens
    import rlplug_qtbuiltin.cmds

    if not options:
        options = {}

    # rlogger.info('load_media_list(): attempting to load media with {ml}'.format(
    #    ml=media_input_list))


    mmgr = revlens.CNTRL.getMediaManager()
    track = revlens.CNTRL.session().getTrackByIndex(track_idx)
    
    # short circuit for session
    #
    if len(media_input_list) == 1 and media_input_list[0].__class__.__name__ in ['basestring', 'unicode', 'str'] and media_input_list[0].endswith('.rls'):
        rlogger.info('Loading session file: {}'.format(media_input_list[0]))

        # NOTE: EARLY RETURN for session load
        return mmgr.loadSessionFile(media_input_list[0])



    if not track:

        rlogger.error('load_media_list(): invalid track {i}, cannot load media'.format(
            i=track_idx))
        return


    if clear:
        track.clear()

    track_uuid = track.uuid().toString()
    curr_track_frames = set(track.getFrames())

    next_frame = start_frame
    loaded_media_count = 0

    # if media_input_list and len(media_input_list) > 0:
    #    rlplug_qtbuiltin.cmds.display_message('Loading...', 0)

    return_result = []

    media_total = len(media_input_list)
    media_count = 1
    for media_in in media_input_list:
        
        # rlplug_qtbuiltin.cmds.display_message('Loading... {n}/{t}'.format(
        #    n=media_count, t=media_total), 0)

        if media_in.__class__.__name__ in ['str']:
            if options.get('expand_file_to_sequence'):
                media_in_dir, media_in_filename = os.path.split(media_in)
                seq_list = RlpCore.SEQ_Util().getSequenceNames(media_in_dir)
                for seq_entry in seq_list:
                    seq_prefix = seq_entry.split('.')[0]
                    if media_in_filename.startswith(seq_prefix):
                        media_in = os.path.join(media_in_dir, seq_entry)
                        print('Found sequence from filename: {}'.format(media_in))


            media_in = {'path': media_in}

        # Seed some data for progress
        #
        if media_in.__class__.__name__ in ['dict', 'OrderedDict']:

            media_in['progress.media_count'] = media_count
            media_in['progress.media_total'] = media_total

            if 'media.metadata' not in media_in:
                media_in['media.metadata'] = {}

            media_in['graph.frame'] = next_frame
            media_in['track.uuid'] = track_uuid


        rlogger.debug('Attempting to identify: {}'.format('')) # media_in))

        media_result_val = mmgr.identifyMedia(media_in)
        media_result = media_result_val

        rlogger.debug('Identify complete')
        # pprint.pprint(media_result_val)

        if media_in.__class__.__name__ in ['dict', 'OrderedDict']:

            # Copy generated metadata back to input list for sync
            #

            if 'media.metadata' in media_result:
                media_in['media.metadata'].update(media_result['media.metadata'])

            if 'media.frame_count' in media_result:
                media_in['media.frame_count'] = media_result['media.frame_count']

            media_in['graph.uuid'] = media_result['graph.uuid']


        if media_result:
            node = mmgr.getMediaFactory().createMedia(media_result_val, False)
            if node:

                rlogger.info('OK: {}'.format(node))

                properties = node.getProperties()
                last_start_frame = next_frame
                

                load_frame_set = set(range(
                    last_start_frame,
                    next_frame + properties['media.frame_count']
                ))

                if curr_track_frames.intersection(load_frame_set):
                    
                    err_msg = 'ERROR: track "{}" has media inside load frame range, aborting'.format(
                        track.name()
                    )

                    rlogger.error(err_msg)
                    # rlplug_qtbuiltin.cmds.display_message(err_msg, duration=2500, colour='red')

                    # TODO FIXME: HMM.. this is gonna leave any previous media loaded
                    # do this in two passes?
                    
                    return

                track.insertNode(node, next_frame)
                
                next_frame += properties['media.frame_count']
                loaded_media_count += 1

                return_result.append(media_result)

            else:
                err = 'ERROR: Invalid node input, cannot load'
                rlogger.error(err)
                rlplug_qtbuiltin.cmds.display_message(err, 2500, 'red')



        media_count += 1


    if emit_sync_action:

        room_uuid = rlp.util.APPGLOBALS.globals().get('room_uuid')
        info = {
            'media_input_list': {'media': media_input_list}, # need to wrap for correct serialization of the list..
            'track_idx': track_idx,
            'track_uuid': track.uuid().toString(),
            'track_info': json.loads(track.toJson(False).toString()),
            'room_uuid': room_uuid,
            'start_frame': start_frame,
            'clear': clear
        }
        revlens.CNTRL.emitSyncAction('load_media_list', info)


    # rlplug_qtbuiltin.cmds.display_message('Load Complete - {n} Items'.format(n=loaded_media_count))

    revlens.CNTRL.getVideoManager().cache().expireAll()
    revlens.CNTRL.gotoFrame(1, True, True, True)

    return return_result


def _sanitize_media_info(media_in):
    '''
    Remove all generated info from media info, preserve only media keys
    Useful for sync, where receiving device may need to regenerate / download
    media data
    '''

    #
    # TODO FIXME: DUPLICATE OF SESSION_IO - REFACTOR
    #

    result = {}
    for key, value in list(media_in.items()):
        if key.startswith('media.') and 'resolved_component' not in key:
            result[key] = value

    return result


def insert_media_at_frame(media_in, track_idx, frame_num, emit_sync_action=True, identify=False):

    mmgr = revlens.CNTRL.getMediaManager()
    if identify:
        media_in = mmgr.identifyMedia(media_in)

    node = mmgr.getMediaFactory().createMedia(media_in_val)
    if node: # .isValid():

        track = revlens.CNTRL.session().getTrackByIndex(track_idx)
        track.insertNode(node, frame_num)


        if emit_sync_action:
            
            node_props = node.getProperties()
            if media_in.__class__.__name__ in ['dict']:
                media_in['media.frame_count'] = node_props['media.frame_count']

            # rlogger.debug(pprint.pformat(node_props))
            room_uuid = rlp.util.APPGLOBALS.globals().get('room_uuid')

            info = {
                'media_in': _sanitize_media_info(media_in),
                'track_idx': track_idx,
                'track_uuid': track.uuid().toString(),
                'media_uuid': node.graph().uuid().toString(),
                'room_uuid': room_uuid,
                'frame': frame_num
            }
            revlens.CNTRL.emitSyncAction("insert_media_at_frame", info)


def append_to_composite(media_in, node_uuid):

    import revlens
    import revlens.cmds

    node_in = revlens.CNTRL.session().getNodeByUuid(node_uuid)

    mmgr = revlens.CNTRL.getMediaManager()

    node_in_props = node_in.getProperties()
    composite_node = node_in_props.get("graph.node.composite")

    media_in = locate(media_in, resolve_components=True)

    rlogger.debug('Attempting to identify: {}'.format('')) # media_in))

    media_result_val = mmgr.identifyMedia(media_in)
    media_result = media_result_val

    rlogger.debug('Identify complete')


    if media_in.__class__.__name__ in ['dict', 'OrderedDict']:
        
        for media_entry in [media_in, media_result]:
            if 'media.metadata' not in media_entry:
                media_entry['media.metadata'] = {}


        # Copy generated metadata back to input list for sync
        #
        media_in['media.metadata'].update(media_result['media.metadata'])
        media_in['media.frame_count'] = media_result['media.frame_count']


    if media_result:
        node = mmgr.createMedia(media_result_val, True)
        if node:
            
            rlogger.info('{}: Appending to {}'.format(node, composite_node))
            composite_node.addInput(node)

            curr_frame = revlens.cmds.get_current_frame_num()

            revlens.CNTRL.getVideoManager().cache().expireAll()
            revlens.CNTRL.gotoFrame(curr_frame, True, False, True)
            revlens.CNTRL.getVideoManager().update()


            



def startup_load_media():
    
    import revlens
    
    appGlobals = rlp.util.APPGLOBALS.globals()
    media_input_list = appGlobals['startup.media']
    
    rlogger.info('startup: loading media {ml}'.format(ml=media_input_list))
    
    gotoFrame = 0
    if appGlobals.get('startup.goto_frame'):
        gotoFrame = int(appGlobals['startup.goto_frame'])


    return load_media_list(media_input_list)
    
