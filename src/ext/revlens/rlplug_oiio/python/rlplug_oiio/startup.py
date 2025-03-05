


def startup():
    print('startup()')


    import rlp.core.util
    import revlens.plugin

    import rlplug_oiio

    import RlpExtrevlensRLOIIOmodule
    RlpExtrevlensRLOIIOmodule.init_module()
    rlp.core.util.loft_to_module('rlplug_oiio', RlpExtrevlensRLOIIOmodule)

    revlens.plugin.register(
        revlens.plugin.PLUGIN_MEDIA,
        rlplug_oiio.RLOIIO_PLUGIN_OBJ
    )


startup()