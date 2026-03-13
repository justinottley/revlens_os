

#include "RlpRevlens/AUDIO/OutputDevice.h"

#include "RlpRevlens/DS/PlaybackMode.h"

RLP_SETUP_LOGGER(revlens, AUDIO, OutputDevice)


AUDIO_OutputDevice::AUDIO_OutputDevice(DS_ControllerBase* controller):
    QIODevice(),
    _controller(controller),
    _bytesPerFrame(0),
    _scrubbing(false),
    _scrubAutoStop(false),
    _scrubFrame(-1),
    _currNodeInfo({nullptr, 0, 0, 0, 0, 0})
{
}


void
AUDIO_OutputDevice::setTargetFrameRate(float frameRate)
{
    RLP_LOG_DEBUG(frameRate)

    _frameRate = frameRate;

    int samplesPerFrame = int(rint(AUDIO_DATA_SAMPLE_RATE_HZ / frameRate));
    int bytesPerFrame = samplesPerFrame * AUDIO_DATA_CHANNEL_COUNT * AUDIO_DATA_BYTES_PER_SAMPLE;

    RLP_LOG_DEBUG(
        "Frame Rate:" << frameRate <<
        "audio samples per video frame:" << samplesPerFrame << 
        "Bytes per video frame:" << bytesPerFrame)

    _bytesPerFrame = bytesPerFrame;
}


void
AUDIO_OutputDevice::clearAll()
{
    // RLP_LOG_DEBUG("")

    _currNodeInfo = {nullptr, 0, 0, 0, 0, 0};
    updateToFrame(_currentFrameNum);
}


void
AUDIO_OutputDevice::updateToFrame(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum)

    DS_NodePtr node = _controller->session()->getNodeByFrame(frameNum);
    if (node == nullptr)
    {
        RLP_LOG_ERROR("No node at frame" << frameNum)
        return;
    }

    if ((_currNodeInfo.node == nullptr) || 
        (node != _currNodeInfo.node) ||
        ((frameNum - _currNodeInfo.frameStart) > _currNodeInfo.frameCount))
    {
        qlonglong nodeFrameCount = node->getProperty<qlonglong>("media.frame_count");
        float nodeFrameRate = node->getProperty<float>("video.frame_rate");

        DS_AudioFormat f(nodeFrameRate);

        _currNodeInfo = {
            node,
            node->getProperty<qlonglong>("session.frame"),
            nodeFrameCount,
            f.bytesPerFrame(),
            f.bytePositionForFrame(nodeFrameCount)
        };


        // RLP_LOG_DEBUG(
        //     "New node:" <<
        //     node.get() << 
        //     "at:" << _currNodeInfo.frameStart << 
        //     "length:" << _currNodeInfo.frameCount <<
        //     "max audio byte count:" << _currNodeInfo.maxAudioBytes)
    }

    _currentFrameNum = frameNum;

    // RLP_LOG_DEBUG("curr frame:" << _currentFrameNum << "curr node frame start:" << _currNodeInfo.frameStart)

    _currNodeInfo.bytePos = (_currentFrameNum - _currNodeInfo.frameStart) * _bytesPerFrame;
}


void
AUDIO_OutputDevice::setScrubbing(qlonglong scrubFrame, bool inScrub, bool scrubAutoStop)
{
    _scrubbing = inScrub;
    _scrubAutoStop = scrubAutoStop;

    if (inScrub)
    {
        _scrubFrame = scrubFrame;
    } else
    {
        _scrubFrame = -1;
    }
}


bool
AUDIO_OutputDevice::setNextNode()
{
    // RLP_LOG_DEBUG("")

    DS_PlaybackMode* pbm = _controller->playbackMode();
    qlonglong currEndFrame = _currNodeInfo.frameStart + _currNodeInfo.frameCount - 1;

    DS_PlaybackMode::FrameInfo nextFrameInfo = pbm->getNextFrameNum(
        currEndFrame, pbm->getDirection());

    qlonglong nextFrame = nextFrameInfo.first;

    updateToFrame(nextFrame);

    return true;
}


qint64
AUDIO_OutputDevice::readData(char* dest, qint64 len)
{
    // RLP_LOG_DEBUG(len)

    if (len == 0)
        return 0;

    if (_currNodeInfo.node == nullptr)
    {
        RLP_LOG_ERROR("NO CURRENT NODE")
        return 0;
    }


    // This might changed after data read comes back? so dont stash it, query it

    if (!_currNodeInfo.node->hasNodeProperty("audio.data"))
    {
        RLP_LOG_ERROR("No audio at current node")
        return 0;
    }

    QByteArray* audioData = _currNodeInfo.node->getProperty<QByteArray*>("audio.data");

    // RLP_LOG_DEBUG("Audio data size:" << audioData->size())

    if (_scrubbing)
    {
        if (_scrubFrame != -1)
        {
            len = _bytesPerFrame * 2;
            _scrubFrame = -1;

        } else
        {
            // played the frame already
            // RLP_LOG_DEBUG("frame played:" << _scrubFrame)
            if (_scrubAutoStop)
            {
                emit requestAudioStop(_scrubFrame);
            }
            return 0;
        }
    }


    if ((_currNodeInfo.bytePos + len) <= _currNodeInfo.maxAudioBytes)
    {
        memcpy(dest, (char *)audioData->data() + _currNodeInfo.bytePos, len);

        _currNodeInfo.bytePos += len;

    } else
    {
        // RLP_LOG_DEBUG(_currNodeInfo.maxAudioBytes << _currNodeInfo.bytePos << len)

        qint64 maxLen = _currNodeInfo.maxAudioBytes - _currNodeInfo.bytePos;

        memcpy(dest, (char *)audioData->data() + _currNodeInfo.bytePos, maxLen);

        qint64 remainderBytes = len - maxLen;

        // RLP_LOG_DEBUG("maxlen:" << maxLen << "Need remainder:" << remainderBytes)

        setNextNode();

        // RLP_LOG_DEBUG("max audio bytes for next node:" << _currNodeInfo.maxAudioBytes)

        while (true) // melancholy and the infinite sadness
        {
            // RLP_LOG_DEBUG("inside next node iteration")

            if (_currNodeInfo.bytePos != 0)
            {
                RLP_LOG_WARN("Expect 0 byte pos for next node, got" << _currNodeInfo.bytePos)
            }

            qint64 remBytes = remainderBytes;
            if (remainderBytes > _currNodeInfo.maxAudioBytes)
            {
                // RLP_LOG_WARN("Next node has less audio than requested remainder")
                remBytes = _currNodeInfo.maxAudioBytes;
            }

            audioData = _currNodeInfo.node->getProperty<QByteArray*>("audio.data");
            memcpy(dest, (char *)audioData->data() + _currNodeInfo.bytePos, remBytes);

            if (remBytes < remainderBytes)
            {
                setNextNode();
                remainderBytes = remainderBytes - remBytes;
            } else
            {
                break;
            }
        }

        // RLP_LOG_DEBUG("split node done")

        _currNodeInfo.bytePos += remainderBytes;
    }

    return len;
}


qint64
AUDIO_OutputDevice::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return len;
}
