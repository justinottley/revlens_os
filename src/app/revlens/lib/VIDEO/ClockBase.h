//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_VIDEO_SYNCCLOCK_BASE_H
#define REVLENS_VIDEO_SYNCCLOCK_BASE_H


#include "RlpRevlens/VIDEO/Global.h"

#include "RlpRevlens/DS/NanoTimer.h"
#include "RlpRevlens/DS/TimeConstants.h"

#include "RlpRevlens/VIDEO/SyncTarget.h"



/*
 * Purpose of this class is regulate the timing of painting a frame.
 * The interval of the paint is determined by the frame rate, and is
 * measured / regulated based on how long it takes to paint the frame.
 * This class measures how long it takes to paint a frame and calculates
 * how long to wait till the next repaint is required.
 * 
 * Every update interval, emit a callback to a "Sync Target".
 * The "Sync target" is meant to handle whatever is necessary to
 * update on the frame repaint interval time.
 *
 */
class REVLENS_VIDEO_API VIDEO_ClockBase : public QObject {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER
    
    VIDEO_ClockBase(VIDEO_SyncTarget* target,
                    float frameRate);

public slots:
    
    virtual void setTargetFrameRate(float frameRate);
    virtual float getTargetFrameRate() { return _frameRate; }
    
    virtual void updateVideo() {}
    virtual void resume() {}
    virtual void stop() {}


protected:
    
    VIDEO_SyncTarget* _syncTarget;
    
    bool _running;
    
    bool _resetDrift;


protected:
    
    DS_NanoTimer _timer;
    
    float _frameRate;
    
    timespec _vsyncTime;
    timespec _vsyncNextTime;

    timespec _sleepInterval;
    
    uint64_t _rate;
    int64_t _rateDiff;
    
};

#endif

