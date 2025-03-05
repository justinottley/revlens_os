//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_CNTRL_FRAMEPROBE_H
#define EXTREVLENS_RLANN_CNTRL_FRAMEPROBE_H

#include "RlpExtrevlens/RLANN_CNTRL/Global.h"

#include "RlpRevlens/DS/SyncProbe.h"


class RLANN_CNTRL_DrawController;

class EXTREVLENS_RLANN_CNTRL_API RLANN_CNTRL_FrameProbe : public DS_SyncProbe {
    
public:
    RLP_DEFINE_LOGGER
    
    RLANN_CNTRL_FrameProbe(RLANN_CNTRL_DrawController* plugin);
    
    void syncUpdate(qlonglong currFrame);
    void syncUpdateImmediate(qlonglong currFrame) { syncUpdate(currFrame); }
    
    
private:
    
    RLANN_CNTRL_DrawController* _plugin;
};

#endif