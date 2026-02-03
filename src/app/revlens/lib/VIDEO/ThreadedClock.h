//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_VIDEO_THREADED_CLOCK_H
#define REVLENS_VIDEO_THREADED_CLOCK_H

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/VIDEO/Global.h"
#include "RlpRevlens/VIDEO/ClockBase.h"


class REVLENS_VIDEO_API VIDEO_ThreadedClockTimer : public VIDEO_ClockBase {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER
    
    VIDEO_ThreadedClockTimer(
        VIDEO_SyncTarget* target,
        float frameRate
    );
    
    void setTargetFrameRate(float frameRate);
    
    void stop();
    
public slots:
    
    void updateVideo();
    void resume();
    void run();
    
    
private:
    
    void setRunning();
    void stopRunning();
    bool isRunning();
    
private:
    
    QMutex* _runLock;
    QWaitCondition* _run;
    
    QMutex* _vmutex;
};


class REVLENS_VIDEO_API VIDEO_ThreadedClock : public VIDEO_ClockBase {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER
    
    VIDEO_ThreadedClock(
        VIDEO_SyncTarget* target,
        float frameRate
    );

public slots:
    
    void setTargetFrameRate(float frameRate) { _timer->setTargetFrameRate(frameRate); }
    float getTargetFrameRate() { return _timer->getTargetFrameRate(); }

    void updateVideo() { _timer->updateVideo(); }
    void resume() { _timer->resume(); }
    void stop() { _timer->stop(); }


private:

    VIDEO_ThreadedClockTimer* _timer;
    QThread _thread;
    

};

#endif

