//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_CNTRL_MEDIAPROBE_H
#define REVLENS_CNTRL_MEDIAPROBE_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpRevlens/DS/SyncProbe.h"


class REVLENS_CNTRL_API CNTRL_FrameRateProbe : public DS_SyncProbe {

public:
    RLP_DEFINE_LOGGER

    CNTRL_FrameRateProbe();

    void syncUpdate(qlonglong currFrame);
    void syncUpdateImmediate(qlonglong currFrame);

private:
    QUuid _currMediaUuid;
};

#endif

