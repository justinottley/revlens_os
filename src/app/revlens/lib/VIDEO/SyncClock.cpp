//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

// Qt's QTimer seems to have only millisecond accuracy (even with 
// Qt::PreciseTimer it seems) which is not accurate enough to maintain a 
// stable precise enough framerate. The framerate tends to drift too wildly
// when I tried using QTimer (with Qt::PreciseTimer) 
// Hence revlens has a custom nanosecond accurate timer and sync clock.



#include "RlpRevlens/VIDEO/SyncClock.h"

#include "RlpRevlens/DS/TimeConstants.h"


RLP_SETUP_LOGGER(revlens, VIDEO, AVSyncClock)

VIDEO_AVSyncClock::VIDEO_AVSyncClock(VIDEO_SyncTarget* target, 
                                     float frameRate,
                                     QMutex* mutex,
                                     QWaitCondition* wc) :
    _timer(),
    _syncTarget(target),
    _runLock(mutex),
    _run(wc),
    _running(true)
{
    _vsync = new QWaitCondition();
    _vsync->wakeAll();
    
    _vmutex = new QMutex();
    
    _vsyncTime.tv_sec = 0;
    _vsyncTime.tv_nsec= 0L;
    
    setTargetFrameRate(frameRate);
    
    _timer.getTime(&_vsyncTime);
}


void
VIDEO_AVSyncClock::syncWait()
{
    QMutexLocker lock(_vmutex);
    
    _vsync->wait(_vmutex);
}


void
VIDEO_AVSyncClock::syncWake()
{
    _vsync->wakeAll();
}


void
VIDEO_AVSyncClock::setTargetFrameRate(float frameRate)
{
    QMutexLocker lock(_vmutex);
    
    _frameRate = frameRate;
    
    // _rate = (1.0 / frameRate) * NANOSEC_SCALE;
    // _rateDiff = _rate;
    
    double rawRate = (1.0 / frameRate);
    int rateSecs = (int)rawRate;
    double fractionalSecs = rawRate - rateSecs;
    // uint64_t secRate = TIME_SEC_SCALE - _rate;
    
    _rate = fractionalSecs * TIME_SEC_SCALE;
    _rateDiff = _rate;
    
    RLP_LOG_DEBUG("rate secs: " << rateSecs  << " fractional secs: " << fractionalSecs << " final rate: " << _rate);
    
    // _sleepInterval.tv_sec = 0;
    _sleepInterval.tv_sec = rateSecs;
    _sleepInterval.tv_nsec = _rate; // (1.0 / frameRate) * TIME_SEC_SCALE;
}


void
VIDEO_AVSyncClock::updateVideo()
{
    #ifdef _WIN32   
    QMetaObject::invokeMethod(_syncTarget,
                              "displayNextFrame",
                              Qt::BlockingQueuedConnection);
    #else
    QMetaObject::invokeMethod(_syncTarget,
                              "displayNextFrame",
                              Qt::BlockingQueuedConnection);
    #endif
    
    // RLP_LOG_DEBUG( << "VIDEO_AVSyncClock::updateVideo() " <<  std::endl;
    
    _timer.getTime(&_vsyncNextTime);
    
    
    // RLP_LOG_DEBUG(_vsyncTime.tv_sec);
    // RLP_LOG_DEBUG(_vsyncTime.tv_nsec);
    
    
    // #ifdef _WIN32
    
    // On Windows actually microseconds, not nanoseconds
    //
    // uint64_t nanodiff = double(_vsyncNextTime.tv_nsec - _vsyncTime.tv_nsec) / _timer.clock_frequency_usecs;
    
    // RLP_LOG_DEBUG("TICK DIFF: " << (_vsyncNextTime.tv_nsec - _vsyncTime.tv_nsec));
    // #else
    
    uint64_t currNanoTime = (_vsyncTime.tv_sec * TIME_SEC_SCALE) + _vsyncTime.tv_nsec;
    uint64_t newNanoTime = (_vsyncNextTime.tv_sec * TIME_SEC_SCALE) + _vsyncNextTime.tv_nsec;
    
    uint64_t nanodiff = newNanoTime - currNanoTime;
    
    
    // #endif // _WIN32
    
    int64_t drift = nanodiff - (_sleepInterval.tv_sec + _rate);
    
    _vsyncTime.tv_sec = _vsyncNextTime.tv_sec;
    _vsyncTime.tv_nsec = _vsyncNextTime.tv_nsec;
    
    
    /*
    RLP_LOG_DEBUG("nanodiff: " << nanodiff);
    RLP_LOG_DEBUG("rate: " << _sleepInterval.tv_sec + _rate);
    RLP_LOG_DEBUG("drift: " << drift);
    RLP_LOG_DEBUG("_rateDiff: " << _rateDiff);
    */
    
    if (_resetDrift) {
        
        // RLP_LOG_DEBUG("RESETTING DRIFT");
        
        drift = 0;
        _resetDrift = false;
    }
    
    
    if (std::abs(drift) > DRIFT_THRESHOLD) {

        // std::cout << "DRIFT: " << drift << " T: " << DRIFT_THRESHOLD << std::endl;
        
        _rateDiff = _rateDiff - drift;
    }
    
    if (_rateDiff < 0) {
        _rateDiff = _rate;
    }
    
    
    _sleepInterval.tv_nsec = _rateDiff; // _rate;
    // _sleepInterval.tv_nsec = 41701417;
    
    
    
    #ifdef _WIN32
    
    QThread::currentThread()->usleep(_sleepInterval.tv_nsec);
    // QThread::currentThread()->usleep(27000);
    
    #else
    
    
    nanosleep(&_sleepInterval, NULL);
    
    #endif
}


void
VIDEO_AVSyncClock::stop()
{
    stopRunning();
}


void
VIDEO_AVSyncClock::resume()
{
    RLP_LOG_DEBUG("");
    
    setRunning();
    
    _resetDrift = true;
    
    _run->wakeAll();
}


void
VIDEO_AVSyncClock::setRunning()
{
    QMutexLocker lock(_vmutex);
    
    _running = true;
}


void
VIDEO_AVSyncClock::stopRunning()
{
    QMutexLocker lock(_vmutex);
    
    _running = false;
}


bool
VIDEO_AVSyncClock::isRunning()
{
    QMutexLocker lock(_vmutex);
    
    bool result = _running;
    return result;
}


void
VIDEO_AVSyncClock::run()
{
    
    RLP_LOG_DEBUG( "");
    
    while (true) {
        
        _runLock->lock();
        _run->wait(_runLock);
        _runLock->unlock();
        
        
        // TODO: start the sound here (after the thread wakes up?)
        // so that we hit a more accurate sync..?
        
        while (isRunning()) {
            updateVideo();
        }
    }
}

