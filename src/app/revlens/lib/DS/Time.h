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

    DS_Time(float videoFrameRate);

    void setSecs(double secs);
    void setMicroSecs(qint64 microSecs);
    void setVideoFrame(qlonglong videoFrame);
    void setVideoFrameRate(float frameRate);
    void setVideoFrameOffset(qlonglong videoFrameOffset);

    double secs() { return _secs; }
    qint64 microSecs() { return _microSecs; }

    qlonglong videoFrame() { return _videoFrame; }

    // NOTE: media frame starts at 0, playlist frame starts at 1
    //
    qlonglong mediaVideoFrame() { return std::max(0, int(_videoFrame - _videoFrameOffset)); }
    qlonglong videoFrameOffset() { return _videoFrameOffset; }

    DS_TimePtr copy();


protected:
    
    float _videoFrameRate;
    
    double _secs;
    qint64 _microSecs;
    qlonglong _videoFrame;
    
    qlonglong _videoFrameOffset;
};


Q_DECLARE_METATYPE(DS_TimePtr);

#endif

