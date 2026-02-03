
#include "RlpRevlens/VIDEO/ClockBase.h"


RLP_SETUP_LOGGER(revlens, VIDEO, ClockBase)

VIDEO_ClockBase::VIDEO_ClockBase(
    VIDEO_SyncTarget* target,
    float frameRate):
        _syncTarget(target),
        _frameRate(frameRate),
        _timer(),
        _running(false)
{    
    _vsyncTime.tv_sec = 0;
    _vsyncTime.tv_nsec= 0L;

    setTargetFrameRate(frameRate);

    _timer.getTime(&_vsyncTime);
}


void
VIDEO_ClockBase::setTargetFrameRate(float frameRate)
{
    _frameRate = frameRate;
    
    // _rate = (1.0 / frameRate) * NANOSEC_SCALE;
    // _rateDiff = _rate;
    
    double rawRate = (1.0 / frameRate);
    int rateSecs = (int)rawRate;
    double fractionalSecs = rawRate - rateSecs;
    // uint64_t secRate = TIME_SEC_SCALE - _rate;
    
    _rate = fractionalSecs * TIME_SEC_SCALE;
    _rateDiff = _rate;
    
    RLP_LOG_DEBUG("rate secs: " << rateSecs  << " fractional secs: " << fractionalSecs << " final rate in microsecs: " << _rate);
    
    // _sleepInterval.tv_sec = 0;
    _sleepInterval.tv_sec = rateSecs;
    _sleepInterval.tv_nsec = _rate; // (1.0 / frameRate) * TIME_SEC_SCALE;
}
    