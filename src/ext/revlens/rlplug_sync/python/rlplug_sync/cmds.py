
import logging
rlogger = logging.getLogger('rlp.{}'.format(__name__))

import rlplug_sync

def create_room(room_name):

    rlogger.info(room_name)
    return rlplug_sync.CLIENT.create_room(room_name)