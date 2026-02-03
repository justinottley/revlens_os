
import logging

import revlens


class MediaLocator(object):

    # TODO FIXME: Determine programatically via plugins?
    #
    CONTAINER_FORMATS = ['.mov', '.avi', '.mp4', '.ogv', '.m4v', '.mkv', '.mj2']
    FRAME_FORMATS = ['.jpg', '.png', '.tif', '.tiff', '.exr', '.dpx', '.cin', '.tif', '.tiff', '.psd']
    AUDIO_FORMATS = ['.wav', '.aac', '.mp3', '.m4a', '.aiff']

    def __init__(self):
        self.LOG = logging.getLogger('rlp.{ns}.{cls}'.format(
            ns=self.__class__.__module__, cls=self.__class__.__name__
        ))

    @property
    def media_factory(self):
        return revlens.CNTRL.getMediaManager().getMediaFactory()

