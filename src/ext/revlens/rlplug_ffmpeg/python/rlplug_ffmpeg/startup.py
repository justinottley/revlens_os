'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''

import traceback

def startup():

    import logging

    from rlp.core import rlp_logging
    import rlp.core.util

    import rlplug_ffmpeg

    LOG = logging.getLogger('rlp.rlplug_ffmpeg')

    LOG.debug('startup()')

    try:
        import RlpExtrevlensRLFFMPEGmodule
        RlpExtrevlensRLFFMPEGmodule.init_module()
        rlp.core.util.loft_to_module('rlplug_ffmpeg', RlpExtrevlensRLFFMPEGmodule)

    except Exception as e:
        LOG.warning(e)

    import revlens.plugin
    
    revlens.plugin.register(
        revlens.plugin.PLUGIN_MEDIA,
        rlplug_ffmpeg.RLFFMPEG_PLUGIN_OBJ)


try:
    startup()

except:
    print(traceback.format_exc())
