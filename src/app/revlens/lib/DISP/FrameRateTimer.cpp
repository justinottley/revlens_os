#include "RlpRevlens/DS/TimeConstants.h"

#include "RlpRevlens/DISP/FrameRateTimer.h"

DISP_FrameRateTimer::DISP_FrameRateTimer()
{
    _timer.getTime(&_paintTime);
    
    _ticks = (long*)malloc(50 * sizeof(long));
    _tickdiff = (long*)malloc(50 * sizeof(long));
    
    for (int x=0; x!= 50; ++x) {
        _ticks[x] = 0L;
        _tickdiff[x] = 0L;
    }
    
    _ticksum = 0;
    _ticks[0] = _paintTime.tv_nsec;
    _tickIndex = 1;
    
}


void
DISP_FrameRateTimer::time()
{
    _timer.getTime(&_paintTime);
    
    long currNanoTime = (_paintTime.tv_sec * TIME_SEC_SCALE) + _paintTime.tv_nsec;
    
    // _ticksum-= _ticks[_tickIndex];
    _ticks[_tickIndex] = currNanoTime;
    
    int ti = _tickIndex;
    if (_tickIndex == 0) {
        ti = 50;
    }
    
    long newdiff = currNanoTime - _ticks[ti - 1];
    
    
    // RLP_LOG_DEBUG(newdiff);
    
    _ticksum -= _tickdiff[ti - 1];
    _tickdiff[ti - 1] = newdiff;
    _ticksum += newdiff;
    
    _tickIndex++;
    if (_tickIndex == 50) {
        _tickIndex = 0;
        float avg = (float)_ticksum / 50.0;
        
        // #ifdef _WIN32
        // avg = 1.0 / (avg / (double(TIME_SEC_SCALE) * _timer.clock_frequency_usecs));
        // #else
        avg = 1.0 / (avg / double(TIME_SEC_SCALE));
        // #endif
        // RLP_LOG_DEBUG("frame rate: " << avg);
        
        _fpsAvg = avg;
    }
    
}