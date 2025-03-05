//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/FrameRateProbe.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"


RLP_SETUP_LOGGER(revlens, CNTRL, FrameRateProbe)

CNTRL_FrameRateProbe::CNTRL_FrameRateProbe()
{
}


void
CNTRL_FrameRateProbe::syncUpdate(qlonglong currFrame)
{
    DS_NodePtr currNode = _controller->session()->
                                       getNodeByFrame(currFrame);
    if (currNode == nullptr)
    {
        return;
    }

    if (currNode->uuid() == _currMediaUuid)
    {
        return;
    }

    _currMediaUuid = currNode->uuid();

    // ensure framerate is set
    float vcurrFrameRate = _controller->getVideoManager()->
                                       getTargetFrameRate();
    float acurrFrameRate = _controller->getAudioManager()->
                                        getTargetFrameRate();

    if (currNode->hasProperty("video.frame_rate"))
    {
        float mediaFrameRate = currNode->getProperty<float>("video.frame_rate");

        if (vcurrFrameRate != mediaFrameRate) {

            RLP_LOG_INFO("updating video frame rate to" << mediaFrameRate << "fps");
            _controller->getVideoManager()->setTargetFrameRate(mediaFrameRate);
        }

        if (acurrFrameRate != mediaFrameRate)
        {
            RLP_LOG_INFO("updating audio frame rate to " << mediaFrameRate << "fps")
            _controller->getAudioManager()->setTargetFrameRate(mediaFrameRate);
        }
    }
}


void
CNTRL_FrameRateProbe::syncUpdateImmediate(qlonglong currFrame)
{
    syncUpdate(currFrame);
}