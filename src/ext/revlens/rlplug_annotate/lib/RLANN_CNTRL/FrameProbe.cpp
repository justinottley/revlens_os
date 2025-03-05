//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpExtrevlens/RLANN_CNTRL/FrameProbe.h"
#include "RlpExtrevlens/RLANN_CNTRL/DrawController.h"

RLP_SETUP_EXT_LOGGER(RLANN_CNTRL, FrameProbe)

RLANN_CNTRL_FrameProbe::RLANN_CNTRL_FrameProbe(RLANN_CNTRL_DrawController* plugin):
    _plugin(plugin)
{
}


void
RLANN_CNTRL_FrameProbe::syncUpdate(qlonglong frameNum)
{
    if ((_plugin->getMainController()->getPlaybackState() != CNTRL_MainController::PAUSED) ||
        (_plugin->getMainController()->getVideoManager()->isScrubbing()))
    {
        return;
    }

    // RLP_LOG_DEBUG(frameNum);

    _plugin->setupFrame(frameNum);
}