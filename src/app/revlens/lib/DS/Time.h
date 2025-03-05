//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_TIME_H
#define REVLENS_DS_TIME_H

#include <memory>
#include <math.h>

#include <iostream>

#include "RlpRevlens/DS/Global.h"
#include "RlpCore/LOG/Logging.h"

static const int MICROSEC_SCALE = 1000000;

class DS_Time;
typedef std::shared_ptr<DS_Time> DS_TimePtr;

class REVLENS_DS_API DS_Time {

public:
    RLP_DEFINE_LOGGER
    
    
public:

    DS_Time(float videoFrameRate):
        _videoFrameRate(videoFrameRate),
        _secs(0),
        _microSecs(0),
        _videoFrame(0),
        _videoFrameOffset(0)
    {
    }


    void setSecs(double secs)
    {
        _secs = secs;
        _microSecs = round(_secs * MICROSEC_SCALE);
        _videoFrame = _secs * _videoFrameRate + 1;
    }


    void setMicroSecs(qint64 microSecs)
    {
        _microSecs = microSecs;
        _secs = (double)_microSecs / (double)MICROSEC_SCALE;
        _videoFrame = _secs * _videoFrameRate + 1;
    }


    void setVideoFrame(qlonglong videoFrame)
    {

        if (videoFrame < 1) {
            videoFrame = 1;
        }

        _videoFrame = videoFrame;

        if (_videoFrameRate > 0) {

            _secs = (double)(videoFrame - 1) / _videoFrameRate;
            _microSecs = round(_secs * MICROSEC_SCALE);

        } else {

            RLP_LOG_DEBUG(videoFrame << " cannot set secs - video frame rate is 0");
        }
    }


    void setVideoFrameRate(float frameRate)
    {
        _videoFrameRate = frameRate;
        setSecs(_secs);
        
        // LOG4CPLUS_DEBUG(flogger, "DS_Time::setVideoFrameRate(): " << frameRate << " -> " << _secs << " at " << _videoFrame << std::endl;

        // Requires reset of values..
        // _secs = 0;
        // _microSecs = 0;
        // _videoFrame = 0;
    }


    void setVideoFrameOffset(qlonglong videoFrameOffset)
    {
        _videoFrameOffset = videoFrameOffset;
    }

    double secs() { return _secs; }
    qint64 microSecs() { return _microSecs; }

    qlonglong videoFrame() { return _videoFrame; }

    // NOTE: media frame starts at 0, playlist frame starts at 1
    //
    qlonglong mediaVideoFrame() { return std::max(0, int(_videoFrame - _videoFrameOffset)); }
    qlonglong videoFrameOffset() { return _videoFrameOffset; }

    DS_TimePtr copy()
    { 
        DS_TimePtr result(new DS_Time(_videoFrameRate));
        result->setVideoFrame(_videoFrame);

        return result;
    }


protected:
    
    float _videoFrameRate;
    
    double _secs;
    qint64 _microSecs;
    qlonglong _videoFrame;
    
    qlonglong _videoFrameOffset;
};


Q_DECLARE_METATYPE(DS_TimePtr);

#endif

