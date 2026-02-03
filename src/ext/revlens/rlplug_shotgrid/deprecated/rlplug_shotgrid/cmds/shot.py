#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import pprint
import logging

import revlens
import revlens.cmds

import rlplug_qtbuiltin.cmds

from ..sg_handler import ShotgunHandler

rlogger = logging.getLogger('rlp.{}'.format(__name__))

def _refresh_all():

    revlens.CNTRL.getVideoManager().cache().expireAll()
    revlens.cmds.goto_frame(revlens.cmds.get_current_frame_num(), True, True)


def _get_sequence(mdata):
    sgh = ShotgunHandler.instance()

    project_id = mdata.get('project.sg_id')
    shot_name = mdata.get('entity.name')
    
    seq = sgh.get_shot_sequence(project_id, name=shot_name)

    assert(seq)

    sg_seq_obj = sgh.make_sequence(project_id, sg_id=seq['id'])

    return sg_seq_obj


def _verify_next_shot(track_info, track_frame_map, node_uuid, next_shot):

    rlogger.debug('_verify_next_shot(): {u} next to load: {n}'.format(
        u=node_uuid, n=next_shot
    ))

    frame_positions = sorted(track_frame_map)

    rlogger.debug(frame_positions)
    rlogger.debug(track_frame_map)

    media_idx = 0

    # TODO FIXME: this is complicated.. track media iterator?
    for frame_entry in frame_positions:
        if str(track_frame_map[frame_entry]) == str(node_uuid):
            break

        media_idx += 1

    next_media_idx = media_idx + 1

    rlogger.debug('next media idx: {}'.format(next_media_idx))
    if (next_media_idx < len(frame_positions)):

        # TODO FIXME: if there is any media, just stop now
        # ideally we should verify that the adjacent media is the next shot,
        # but that would mean moving everything to the right to fit the next shot
        #
        # next_frame = frame_positions[next_media_idx]
        # next_node_uuid = track_frame_map[next_frame]
        # 
        # rlogger.debug('next frame start: {}'.format(next_frame))
        # rlogger.debug('next node uuid: {}'.format(next_node_uuid))
        

        return False

    return True



def _verify_prev_shot(track_info, track_frame_map, node_uuid, prev_shot):

    rlogger.debug('_verify_prev_shot(): {u} prev to load: {n}'.format(
        u=node_uuid, n=prev_shot
    ))

    frame_positions = sorted(track_frame_map)

    rlogger.debug(frame_positions)
    rlogger.debug(track_frame_map)

    media_idx = 0

    # TODO FIXME: this is complicated.. track media iterator?
    for frame_entry in frame_positions:
        if str(track_frame_map[frame_entry]) == str(node_uuid):
            break

        media_idx += 1

    prev_media_idx = media_idx - 1

    rlogger.debug('prev media idx: {}'.format(prev_media_idx))
    if (prev_media_idx >= 0):

        # TODO FIXME: if there is any media, just stop now
        # ideally we should verify that the adjacent media is the next shot,
        # but that would mean moving everything to the right to fit the next shot
        #
        # next_frame = frame_positions[next_media_idx]
        # next_node_uuid = track_frame_map[next_frame]
        # 
        # rlogger.debug('next frame start: {}'.format(next_frame))
        # rlogger.debug('next node uuid: {}'.format(next_node_uuid))
        

        return False

    return True


def load_next_shot(node_in, department=None):
    '''
    context loading function - get the next shot based on the one input,
    and insert it after the input shot on the same track
    '''

    sgh = ShotgunHandler.instance()
    props = node_in.getProperties()
    mdata = props.get('media.metadata', {})

    if 'shot' not in mdata:

        rlogger.error('loadNextShot(): shot not found in metadata')
        return

    
    track = props['session.track']
    track_info = track.getNodeInfo()
    track_index = track.index()

    # order by frame number - values are supposed to be unique so the dict
    # can be inverted
    track_frame_map = dict((v,k) for k,v in track_info.items())

    shot_name = mdata.get('entity.name')
    department = department or mdata.get('department')

    sg_seq_obj = _get_sequence(mdata)

    next_shot_obj = sg_seq_obj.next_shot(name=shot_name)

    verify_result = _verify_next_shot(
        track_info,
        track_frame_map,
        props['graph.uuid'],
        next_shot_obj.name
    )

    if not verify_result:
        rlplug_qtbuiltin.cmds.display_message(
            'Cannot load next shot - media present',
            duration=2500,
            colour='red')

        return


    best_version = next_shot_obj.get_best_version(department)

    media_builder = sgh.version_make_media(best_version)
    media_in = media_builder.payload # revlens.cmds.identify_media(media_builder.payload)

    rlogger.info('')
    rlogger.info(pprint.pformat(media_in))
    rlogger.info('')

    at_frame = props['session.frame']
    next_frame = at_frame + props['media.frame_count']

    revlens.cmds.insert_media_at_frame(
        media_in,
        track_index,
        next_frame,
        identify=True
    )

    _refresh_all()


def load_previous_shot(node_in, from_frame, department=None, emit_sync_action=True):

    rlogger.info('loadPreviousShot()')

    sgh = ShotgunHandler.instance()
    props = node_in.getProperties()
    mdata = props.get('media.metadata', {})
    
    
    shot_name = mdata.get('entity.name')
    department = department or mdata.get('department')

    track = props['session.track']
    track_info = track.getNodeInfo()
    # track_uuid = track.uuid().toString()

    # order by frame number - values are supposed to be unique so the dict
    # can be inverted
    track_frame_map = dict((v,k) for k,v in track_info.items())

    sg_seq_obj = _get_sequence(mdata)
    
    prev_shot_obj = sg_seq_obj.prev_shot(name=shot_name)

    verify_result = _verify_prev_shot(
        track_info,
        track_frame_map,
        props['graph.uuid'],
        prev_shot_obj.name
    )

    if not verify_result:
        rlplug_qtbuiltin.cmds.display_message(
            'Cannot load prev shot - media present',
            duration=2500,
            colour='red')

        return

    best_version = prev_shot_obj.get_best_version(department)

    media_builder = sgh.version_make_media(best_version)

    # Run identify now to get frame information
    #
    # media_in = revlens.media.resolve_components(media_builder.payload)

    # Need to create the whole node to do any retiming for handles
    #
    mmgr = revlens.CNTRL.getMediaManager()
    media_in = mmgr.identifyMedia(media_builder.payload)
    new_node = mmgr.getMediaFactory().createMedia(media_in)

    node_props = new_node.getProperties()

    rlogger.info(pprint.pformat(media_in))
    rlogger.info('')
    rlogger.info(node_props)
    rlogger.info('')

    media_frame_count = node_props['media.frame_count']

    rlogger.info('Shifting track media by {} frames'.format(media_frame_count))

    session = revlens.CNTRL.session()
    curr_session_frame = props['session.frame']

    for frame_num in reversed(sorted(track_frame_map)):
        node_uuid = track_frame_map[frame_num]

        node = session.getNodeByUuid(node_uuid)
        assert(node.isValid())
        rlogger.info('{f} {n}'.format(f=frame_num, n=node))

        track.moveNodeToFrame(node, frame_num + media_frame_count)

    track.insertNode(new_node, curr_session_frame)


    '''
    revlens.cmds.insert_media_at_frame(
        media_in,
        track_index,
        curr_session_frame
    )
    '''

    if emit_sync_action:

        rlogger.debug('EMIT SYNC ACTION {}'.format(department))

        revlens.CNTRL.emitSyncAction(
            'load_previous_shot',
            {
                'from_frame': from_frame,
                'department': department,
                'track_uuid': track.uuid().toString(),
                'track_info': track.toJson()
            }
        )

    _refresh_all()


def load_previous_shot_by_current_frame():

    rlogger.info('load_previous_shot_by_current_frame()')

    from_frame = revlens.cmds.get_current_frame_num()
    n = revlens.CNTRL.session().getNodeByFrame(from_frame)
    if n.isValid():
        load_previous_shot(n, from_frame, department='Animation')

def load_next_shot_by_current_frame():

    rlogger.info('load_next_shot_by_current_frame()')

    n = revlens.CNTRL.session().getNodeByFrame(revlens.cmds.get_current_frame_num())
    if n.isValid():
        load_next_shot(n, department='Animation')

def load_surrounding_shots_by_current_frame():

    rlogger.info('load_surrounding_shots_by_current_frame()')

    from_frame = revlens.cmds.get_current_frame_num()
    n = revlens.CNTRL.session().getNodeByFrame(from_frame)
    if n.isValid():
        load_next_shot(n, department='Animation')
        load_previous_shot(n, from_frame, department='Animation')


def _on_load_previous_shot(track_uuid, from_frame, department=None):
    '''
    Sync handler for loading previous shot
    '''

    rlogger.info('_on_load_previous_shot(): {} {} {}'.format(
        track_uuid, from_frame, department
    ))

    track = revlens.CNTRL.session().getTrackByUuid(track_uuid)
    if track.isValid():
        node = track.getNodeByFrame(from_frame)
        if node.isValid():

            rlogger.debug('_on_load_previous_shot(): got node from track {} pos {}'.format(track_uuid, from_frame))
            load_previous_shot(node, from_frame, department=department, emit_sync_action=False)

        else:
            rlogger.error('_on_load_previous_shot(): node not found: - track {} pos {}'.format(track_uuid, from_frame))

    else:
        rlogger.error('_on_load_previous_shot(): track not found: {}'.format(track_uuid))