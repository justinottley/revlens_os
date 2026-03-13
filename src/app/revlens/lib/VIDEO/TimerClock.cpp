

#include "RlpRevlens/VIDEO/TimerClock.h"


RLP_SETUP_LOGGER(revlens, VIDEO, TimerClock)

VIDEO_TimerClock::VIDEO_TimerClock(
    VIDEO_SyncTarget* target,
    float frameRate):
        VIDEO_ClockBase(target, frameRate)
{
    _qtimer = new QTimer();
    _qtimer->setTimerType(Qt::PreciseTimer);

    connect(
        _qtimer,
        &QTimer::timeout,
        this,
        &VIDEO_TimerClock::updateVideo
    );

    _interval = int(rint(double(_rate) / double(MILLISEC_SCALE)));

    RLP_LOG_DEBUG("")
}


void
VIDEO_TimerClock::updateVideo()
{
    // RLP_LOG_DEBUG("")

    // return;
    _syncTarget->displayNextFrame();

    // QMetaObject::invokeMethod(_syncTarget, "displayNextFrame", Qt::AutoConnection);
    
    _timer.getElapsed(&_vsyncNextTime);

    // uint64_t currNanoTime = (_vsyncTime.tv_sec * TIME_SEC_SCALE) + _vsyncTime.tv_nsec;
    // uint64_t newNanoTime = (_vsyncNextTime.tv_sec * TIME_SEC_SCALE) + _vsyncNextTime.tv_nsec;
    
    // uint64_t nanodiff = newNanoTime - currNanoTime;
    
    uint64_t elapsed = _vsyncNextTime.tv_nsec;

    // int64_t drift = nanodiff - (_sleepInterval.tv_sec + _rate);
    int64_t drift = elapsed - _rate;
    
    // RLP_LOG_DEBUG("elapsed: " << elapsed << " rate: " << _rate << " drift: " << drift)

    // _vsyncTime.tv_sec = _vsyncNextTime.tv_sec;
    // _vsyncTime.tv_nsec = _vsyncNextTime.tv_nsec;
    
    if (_resetDrift) {
        
        // RLP_LOG_DEBUG("RESETTING DRIFT");
        
        drift = 0;
        _resetDrift = false;
    }
    
    
    if (std::abs(drift) > DRIFT_THRESHOLD) {

        // RLP_LOG_DEBUG("DRIFT: " << drift << " T: " << DRIFT_THRESHOLD)
        
        _rateDiff = _rateDiff - drift;
        // _rateDiff = _rateDiff + drift;
    }
    
    if (_rateDiff < 0) {
        _rateDiff = _rate;
    }
    
    
    int milliInterval = int(rint(double(_rateDiff) / double(MILLISEC_SCALE)));

    // RLP_LOG_DEBUG("millisec interval: " << milliInterval << " _rateDiff: " << _rateDiff << " drift: " << drift << " elapsed: " << elapsed << " rate: " << _rate)

    /*
    if (std::abs(_qtimer->interval() - milliInterval) > 2) {

        // RLP_LOG_DEBUG("SETTING INTERVAL: " << milliInterval)

        _interval = milliInterval;
        _qtimer->setInterval(_interval);
    }
    */

}


void
VIDEO_TimerClock::resume()
{
    RLP_LOG_DEBUG(_frameRate << "interval:" << _interval)

    _running = true;
    _resetDrift = true;
    
    _qtimer->setInterval(_interval);
    _qtimer->start();
}


void
VIDEO_TimerClock::stop()
{
    RLP_LOG_DEBUG("")

    _running = false;
    _qtimer->stop();
}

