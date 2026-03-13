//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/Time.h"

RLP_SETUP_LOGGER(revlens, DS, Time)

DS_Time::DS_Time(float videoFrameRate):
    _videoFrameRate(videoFrameRate),
    _secs(0),
    _microSecs(0),
    _videoFrame(0),
    _videoFrameOffset(0)
{
}


void
DS_Time::setSecs(double secs)
{
    _secs = secs;
    _microSecs = round(_secs * MICROSEC_SCALE);
    _videoFrame = _secs * _videoFrameRate + 1;
}


void
DS_Time::setMicroSecs(qint64 microSecs)
{
    _microSecs = microSecs;
    _secs = (double)_microSecs / (double)MICROSEC_SCALE;
    _videoFrame = _secs * _videoFrameRate + 1;
}


void
DS_Time::setVideoFrame(qlonglong videoFrame)
{
    if (videoFrame < 1)
    {
        videoFrame = 1;
    }

    _videoFrame = videoFrame;

    if (_videoFrameRate > 0)
    {
        _secs = (double)(videoFrame - 1) / _videoFrameRate;
        _microSecs = round(_secs * MICROSEC_SCALE);

    } else
    {
        // RLP_LOG_DEBUG(videoFrame << " cannot set secs - video frame rate is 0");
    }
}


void
DS_Time::setVideoFrameRate(float frameRate)
{
    _videoFrameRate = frameRate;
    setSecs(_secs);
}


void
DS_Time::setVideoFrameOffset(qlonglong videoFrameOffset)
{
    _videoFrameOffset = videoFrameOffset;
}


DS_TimePtr
DS_Time::copy()
{
    DS_TimePtr result(new DS_Time(_videoFrameRate));
    result->setVideoFrame(_videoFrame);

    return result;
}
