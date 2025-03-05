


def startup():

    import os
    import revlens
    import revlens.plugin

    import rlplug_ocio
    
    if os.getenv('RLP_NO_OCIO'):
        print('Skipping OCIO startup, RLP_NO_OCIO set')
        return


    import RlpExtrevlensRLOCIOmodule
    RlpExtrevlensRLOCIOmodule.init_module()

    rlplug_ocio.RLOCIO_PLUGIN_OBJ = RlpExtrevlensRLOCIOmodule.RLOCIO_PLUGIN_OBJ

    revlens.CNTRL.toolCreated.connect(rlplug_ocio.RLOCIO_PLUGIN_OBJ.onToolCreated)
    revlens.CNTRL.toolCreated.connect(rlplug_ocio.init_toolbar)

    revlens.plugin.register(
        revlens.plugin.PLUGIN_EVENT,
        rlplug_ocio.RLOCIO_PLUGIN_OBJ
    )



startup()