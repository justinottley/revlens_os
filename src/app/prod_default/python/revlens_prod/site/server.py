

import rlp.core as RlpCore

# Seeded from C++
EDBC = None
CNTRL = None



def startup():

    global EDBC
    import RlpEdbCNTRLmodule
    EDBC = RlpEdbCNTRLmodule.EDBC

    print('startup: EDBC OK')

    global CNTRL
    import RlpProdSITEmodule
    CNTRL = RlpProdSITEmodule.CNTRL

    print('startup: CNTRL OK')

    RlpCore._init_plugins()