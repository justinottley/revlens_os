

import traceback


def startup():

    import logging

    from rlp.core import rlp_logging
    import rlp.core.util

    import revlens.plugin

    LOG = logging.getLogger('rlp.rlplug_openexr')
    LOG.debug('startup()')

    import rlplug_openexr
    try:
        import RlpExtrevlensRLEXRMEDIAmodule
        RlpExtrevlensRLEXRMEDIAmodule.init_module()
        rlp.core.util.loft_to_module('rlplug_openexr', RlpExtrevlensRLEXRMEDIAmodule)

    except:
        LOG.warning(traceback.format_exc())
        print(traceback.format_exc())


    revlens.plugin.register(
        revlens.plugin.PLUGIN_MEDIA,
        rlplug_openexr.RLEXRMEDIA_PLUGIN_OBJ)

    LOG.info('startup complete')



try:
    startup()

except:
    print(traceback.format_exc())