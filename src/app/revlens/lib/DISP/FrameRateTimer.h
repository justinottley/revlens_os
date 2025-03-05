
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DISP_FRAMERATE_TIMER_H
#define REVLENS_DISP_FRAMERATE_TIMER_H

#include <time.h>

#include "RlpRevlens/DS/NanoTimer.h"
#include "RlpRevlens/DISP/Global.h"

class REVLENS_DISP_API DISP_FrameRateTimer {

public:
    DISP_FrameRateTimer();

    void time();
    float fpsAverage() { return _fpsAvg; }

private:
    DS_NanoTimer _timer;

    timespec _paintTime;
    
    long* _ticks;
    long* _tickdiff;
    
    int _tickIndex;
    long _ticksum;
    
    float _fpsAvg;
};


#endif