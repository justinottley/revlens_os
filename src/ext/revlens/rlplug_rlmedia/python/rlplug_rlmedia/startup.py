'''
Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
'''
import traceback

def startup():
    
    import logging
    import traceback

    import rlp.core.util

    LOG = logging.getLogger('rlp.rlplug_rlmedia')

    LOG.info('startup()')

    import revlens.plugin
    import revlens.media.locator

    import rlplug_rlmedia

    import RlpExtrevlensRLMEDIAmodule
    RlpExtrevlensRLMEDIAmodule.init_module()
    rlp.core.util.loft_to_module('rlplug_rlmedia', RlpExtrevlensRLMEDIAmodule)


    revlens.plugin.register(
        revlens.plugin.PLUGIN_MEDIA,
        rlplug_rlmedia.RLMEDIA_PLUGIN_OBJ)

    revlens.media.locator.MediaLocator.CONTAINER_FORMATS.append('.rlm')

try:
    startup()


except:
    print(traceback.format_exc())
