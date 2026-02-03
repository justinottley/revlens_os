
import traceback

import rlp.core as RlpCore

# Seeded from C++
EDBC = None
CNTRL = None



def _startup():

    global EDBC
    import RlpEdbCNTRLmodule
    EDBC = RlpEdbCNTRLmodule.EDBC

    print('site_server startup: EDBC OK')

    global CNTRL
    import RlpProdSITEmodule
    CNTRL = RlpProdSITEmodule.CNTRL

    # for some API compatibility (plugins)
    import revlens
    revlens.CNTRL = CNTRL

    print('site_server startup: CNTRL OK')


    RlpCore._init_plugins()


def startup():
    try:
        _startup()

    except:
        print('site_server startup: error running startup')
        print(traceback.format_exc())