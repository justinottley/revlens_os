//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/FrameProbe.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"

RLP_SETUP_LOGGER(revlens, CNTRL, FrameProbe)

CNTRL_FrameProbe::CNTRL_FrameProbe()
{
}


void
CNTRL_FrameProbe::syncUpdate(qlonglong currFrame)
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

    QUuid newUuid = currNode->uuid();
    if (newUuid != _currMediaUuid)
    {
        _currMediaUuid = newUuid;

        QVariantMap info;
        info.insert("media_uuid", newUuid);

        _controller->emitNoticeEvent("clip_changed", info);
    }


    // ensure framerate is set
    float vcurrFrameRate = _controller->getVideoManager()->
                                       getTargetFrameRate();
    float acurrFrameRate = _controller->getAudioManager()->
                                        getTargetFrameRate();

    if (currNode->hasProperty("video.frame_rate"))
    {
        float mediaFrameRate = currNode->getProperty<float>("video.frame_rate");

        if (vcurrFrameRate != mediaFrameRate)
        {

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
CNTRL_FrameProbe::syncUpdateImmediate(qlonglong currFrame)
{
    syncUpdate(currFrame);
}


void
CNTRL_FrameProbe::prerollStart(qlonglong currFrame)
{
    // RLP_LOG_DEBUG(currFrame)

    _controller->getAudioManager()->stop(currFrame);
}


void
CNTRL_FrameProbe::prerollEnd()
{
    _controller->getAudioManager()->resume();
}