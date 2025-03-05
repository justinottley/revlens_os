

import rlp.core.util
import revlens.plugin


import rlplug_qpsd
import RlpExtrevlensRLQPSD_MEDIAmodule
RlpExtrevlensRLQPSD_MEDIAmodule.init_module()
rlp.core.util.loft_to_module('rlplug_qpsd', RlpExtrevlensRLQPSD_MEDIAmodule)


revlens.plugin.register(
    revlens.plugin.PLUGIN_MEDIA,
    rlplug_qpsd.RLQPSD_MEDIA_PLUGIN_OBJ)

print('startup complete')
