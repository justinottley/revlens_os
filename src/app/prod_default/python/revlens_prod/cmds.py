
import base64
import pprint
import logging

import revlens
from .client_ws import SiteWsClient

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

def bookendSelectedShots():

    from revlens_prod.panel.playlist import PlaylistPanel

    playlist = PlaylistPanel.instance()
    selMedia = playlist.selectedMedia()
    minFrame = -1
    maxFrame = -1
    lastLength = -1

    for mediaEntry in selMedia:

        mf = mediaEntry['frame']

        if (minFrame == -1 or mf < minFrame):
            minFrame = mf

        if (maxFrame == -1 or mf > maxFrame):
            maxFrame = mf
            lastLength = mediaEntry['length']

    outFrame = maxFrame + lastLength - 1
    print('{} - {}, {}'.format(minFrame, maxFrame, lastLength))

    revlens.CNTRL.setInFrame(minFrame, False, True)
    revlens.CNTRL.setOutFrame(outFrame, True, True)
