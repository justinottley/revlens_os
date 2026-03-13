//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_CNTRL_MEDIAPROBE_H
#define REVLENS_CNTRL_MEDIAPROBE_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpRevlens/DS/SyncProbe.h"


class REVLENS_CNTRL_API CNTRL_FrameProbe : public DS_SyncProbe {

public:
    RLP_DEFINE_LOGGER

    CNTRL_FrameProbe();

    void syncUpdate(qlonglong currFrame);
    void syncUpdateImmediate(qlonglong currFrame);
    void prerollStart(qlonglong frameNum);
    void prerollEnd();

private:
    QUuid _currMediaUuid;
};

#endif

