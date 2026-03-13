
#ifndef EXTREVLENS_RLANN_GUI_PYMODULE_H
#define EXTREVLENS_RLANN_GUI_PYMODULE_H

#include "RlpCore/LOG/Logging.h"

#include "RlpExtrevlens/RLANN_GUI/Global.h"

class EXTREVLENS_RLANN_GUI_API RLANN_GUI_PyModule {

public:
    RLP_DEFINE_LOGGER

    RLANN_GUI_PyModule();

    static void bootstrap();

};


#endif
