//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_VIDEO_SYNCCLOCK_H
#define REVLENS_VIDEO_SYNCCLOCK_H

#include "RlpRevlens/VIDEO/Global.h"

#include "RlpCore/LOG/Logging.h"
#include "RlpRevlens/DS/NanoTimer.h"

#include "RlpRevlens/VIDEO/SyncTarget.h"

/*

class REVLENS_VIDEO_API VIDEO_SyncTarget : public QObject {
    Q_OBJECT
    
public:

    VIDEO_SyncTarget() {}
    ~VIDEO_SyncTarget() {}
    
    virtual void displayNextFrame() {}
};

*/

class REVLENS_VIDEO_API VIDEO_AVSyncClock : public QObject {
    Q_OBJECT
    
public:

    RLP_DEFINE_LOGGER
    
    
    VIDEO_AVSyncClock(VIDEO_SyncTarget* target,
                      float frameRate,
                      QMutex* mutex,
                      QWaitCondition* wait);
    
    
    
    void setTargetFrameRate(float frameRate);
    float getTargetFrameRate() { return _frameRate; }
    
    void stop();
    
    void syncWake();
    void syncWait();
    
public slots:
    
    void updateVideo();
    void resume();
    void run();
    
    
private:
    
    void setRunning();
    void stopRunning();
    bool isRunning();
    
private:
    
    
    VIDEO_SyncTarget* _syncTarget;
    
    QMutex* _runLock;
    QWaitCondition* _run;
    
    QWaitCondition* _vsync;
    QMutex* _vmutex;
    
    QMutex _slock;
    
    bool _running;
    
    bool _resetDrift;
    
private:
    
    DS_NanoTimer _timer;
    
    float _frameRate;
    
    timespec _vsyncTime;
    timespec _vsyncNextTime;
    
    timespec _sleepInterval;
    
    
    uint64_t _rate;
    int64_t _rateDiff;
    
    
};

#endif

