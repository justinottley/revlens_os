

import pprint
import logging

import revlens
from .client_ws import SiteWsClient

import revlens_prod
import rlplug_annotate.cmds


LOG = logging.getLogger('rlp.revlens_prod')

def register_media(media_uuid):

    LOG.debug('register_media - Creating EDB Media')

    node = revlens.CNTRL.session().getNodeByUuid(media_uuid)
    props = node.getProperties()

    if props.get('media.video.locator') == 'RlpSiteLocator':
        LOG.warning('register_media: skipping, already located via SiteLocator')
        return


    media_name = 'm_{}'.format(media_uuid)
    media_title = props.get('media.name')
    frame_count = props.get('media.frame_count')    
    thumbnail_data = props.get('media.thumbnail.data')

    LOG.debug('register_media: {} {} {} {}'.format(
        media_uuid, media_name, media_title, frame_count
    ))

    media_payload = {
        'title': media_title,
        'frame_count': frame_count,
        'rfs_uuid': media_uuid
    }
    if thumbnail_data:
        media_payload['thumb_frame_data'] = thumbnail_data


    edbc = SiteWsClient.instance()

    def _media_created(media_entity_uuid):
        LOG.debug('Media register complete: {}'.format(media_entity_uuid))

    edbc.call(_media_created, 'edbc.create', 'Media',
        'm_{}'.format(media_uuid),
        media_payload,
        media_uuid
    ).run()


def register_thumbnail(media_uuid):

    LOG.debug('register thumbnail: {}'.format(media_uuid))

    node = revlens.CNTRL.session().getNodeByUuid(media_uuid)
    props = node.getProperties()

    # pprint.pprint(props)
    thumbnail_data = props.get('media.thumbnail.data')
    if not thumbnail_data:
        thumbnail_path = props.get('media.thumbnail.path')

        import base64 # TODO FIXME: late import for iOS
        thumbnail_data = base64.b64encode(open(thumbnail_path, 'rb').read()).decode('utf-8')


    edbc = SiteWsClient.instance()

    def _media_updated(result):
        LOG.debug('Media update complete, result: {}'.format(result))


    edbc.call(_media_updated, 'edbc.update',
        'Media', media_uuid,
        {
            'thumb_frame_data': thumbnail_data
        }
    ).run()


def copy_to_fs(source_path, dest_path, file_uuid):
    LOG.debug('copy_to_fs: skipping')


def bookend_selected_shots():

    from revlens_prod.panel.session.main import SessionPanel

    playlist = SessionPanel.instance()
    selMedia = playlist.selectedMedia()
    minFrame = -1
    maxFrame = -1
    lastLength = -1

    selFrameMap = {}
    for mediaEntry in selMedia:

        mf = mediaEntry['frame']

        selFrameMap[mf] = True
        for mfr in range (mf, mediaEntry['length'] + mf):
            selFrameMap[mfr] = True


        if (minFrame == -1 or mf < minFrame):
            minFrame = mf

        if (maxFrame == -1 or mf > maxFrame):
            maxFrame = mf
            lastLength = mediaEntry['length']

    outFrame = maxFrame + lastLength - 1

    plTrack = playlist.selectedTrack

    deselNodes = []
    for frame in range(minFrame, outFrame):
        if frame not in selFrameMap:
            # print('Not in selection: {}'.format(frame))
            node = plTrack.getNodeByFrame(frame)
            nodeUuid = node.graph().uuid().toString()
            if nodeUuid not in deselNodes:
                deselNodes.append(nodeUuid)
    
    for nodeUuidStr in deselNodes:
        plTrack.setNodeEnabled(nodeUuidStr, False)

    revlens.CNTRL.setInFrame(minFrame, False, True)
    revlens.CNTRL.setOutFrame(outFrame, True, True)


def toggle_bookends():

    BOOKEND_STATE_KEY = 'bookend.state'
    session = revlens.CNTRL.session()

    if not session.hasProperty(BOOKEND_STATE_KEY):
        session.setProperty(BOOKEND_STATE_KEY, False)

    bookendsSet = session.getProperty(BOOKEND_STATE_KEY)

    # print('Current Bookend state: {}'.format(bookendsSet))

    if bookendsSet:
        revlens.cmds.clear_in_out_frames()
        revlens.CNTRL.session().resetNodeEnabled()

    else:
        bookend_selected_shots()

    session.setProperty(BOOKEND_STATE_KEY, not bookendsSet)


def _get_current_node_idx_info():

    session = revlens.CNTRL.session()
    node = session.getNodeByFrame(revlens.CNTRL.currentFrameNum())
    node_props = node.getProperties()
    track = node_props['session.track']
    node_uuid = node_props['graph.uuid'].toString()

    node_frame_list = track.getNodeFrameList()

    idx = 0
    for node_info in node_frame_list:
        if node_info['node_uuid'] == node_uuid:
            break
            
        idx += 1

    return (idx, node_frame_list)


def goto_next_shot():

    idx, node_frame_list = _get_current_node_idx_info()
    idx += 1

    if idx < len(node_frame_list):
        next_frame = node_frame_list[idx]['frame']
        revlens.CNTRL.gotoFrame(next_frame, True, True, True)


def _goto_shot_in_playlist(offset):

    from revlens_prod.panel.session.main import SessionPanel

    playlist = SessionPanel.instance()
    selMedia = playlist.selectedRows()

    if len(selMedia) == 1:
        rowIdx = selMedia[0]
        playlist.selectMediaByIdx(rowIdx + offset)


def goto_next_shot_in_playlist():
    return _goto_shot_in_playlist(1)


def goto_previous_shot_in_playlist():
    return _goto_shot_in_playlist(-1)


def goto_previous_shot():

    idx, node_frame_list = _get_current_node_idx_info()
    idx -= 1
    
    if idx >= 0:

        next_frame = node_frame_list[idx]['frame']
        revlens.CNTRL.gotoFrame(next_frame, True, True, True)


def clearAnnotationsOnCurrentFrame():

    rlplug_annotate.cmds.clearAnnotationOnCurrentFrame()
    revlens_prod.CNTRL_STREAMINGCONTROLLER_OBJ.clearAnnotations()