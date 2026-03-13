

import rlp.core.util
import revlens.plugin


import rlplug_qpdf

import RlpExtrevlensRLQPDF_MEDIAmodule
RlpExtrevlensRLQPDF_MEDIAmodule.init_module()
rlp.core.util.loft_to_module('rlplug_qpdf', RlpExtrevlensRLQPDF_MEDIAmodule)


revlens.plugin.register(
    revlens.plugin.PLUGIN_MEDIA,
    rlplug_qpdf.RLQPDF_MEDIA_PLUGIN_OBJ)

print('startup complete')
