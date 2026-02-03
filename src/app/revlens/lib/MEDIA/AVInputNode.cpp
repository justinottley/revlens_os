//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include "RlpCore/UTIL/AppGlobals.h"

#include "RlpRevlens/DS/AudioBuffer.h"
#include "RlpRevlens/MEDIA/AVInputNode.h"

RLP_SETUP_LOGGER(revlens, MEDIA, AVInputNode)

MEDIA_AVInputNode::MEDIA_AVInputNode(QVariantMap* properties, DS_NodePtr videoIn, DS_NodePtr audioIn) :
    DS_Node("MEDIA_AVInput", properties)
{
    _inputs.clear();

    _inputs.push_back(videoIn);
    _inputs.push_back(audioIn);

    QVariantMap* ag = UTIL_AppGlobals::instance()->globalsPtr();

    QVariantMap audioInfo;
    audioInfo.insert("audio.output.bitdepth", ag->value("audio.output.bitdepth"));
    audioInfo.insert("audio.output.channels", ag->value("audio.output.channels"));
    audioInfo.insert("audio.samplesPerBuffer", ag->value("audio.output.samplesPerBuffer"));

    _emptyAudioBuffer = DS_AudioBufferPtr(new DS_AudioBuffer(this, audioInfo));
    _emptyAudioBuffer->reallocate();

    std::memset(_emptyAudioBuffer->data(), 0, _emptyAudioBuffer->getDataBytes());
}


void
MEDIA_AVInputNode::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destFrame, bool postUpdate)
{
    _inputs[VIDEO_IN]->readVideo(timeInfo, destFrame, postUpdate);
}


void
MEDIA_AVInputNode::readAudio(DS_TimePtr timeInfo, DS_BufferPtr destBuffer)
{
    if (_inputs[AUDIO_IN] != nullptr)
    {
        _inputs[AUDIO_IN]->readAudio(timeInfo, destBuffer);
    } else
    {
        _emptyAudioBuffer->copyTo(destBuffer);
    }
}


void
MEDIA_AVInputNode::scheduleReadTask(DS_TimePtr timeInfo,
                                    DS_BufferPtr destFrame,
                                    DS_Node::NodeDataType dataType,
                                    bool optional)
{
    if (dataType == DS_Node::DATA_VIDEO)
    {
        _inputs[VIDEO_IN]->scheduleReadTask(
            timeInfo, destFrame, dataType, optional);
    } else if (dataType == DS_Node::DATA_AUDIO)
    {
        _inputs[AUDIO_IN]->scheduleReadTask(
            timeInfo, destFrame, dataType, optional);
    } else
    {
        std::cout << "MEDIA_AVInputNode::scheduleReadTask(): "
                  << "unable to schedule, unknown Node data type!"
                  << std::endl;
    }
}
    