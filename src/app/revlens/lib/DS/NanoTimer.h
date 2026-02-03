//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_NANOTIMER_H
#define REVLENS_DS_NANOTIMER_H

#include <iostream>

#include "RlpCore/LOG/Logging.h"
#include "RlpRevlens/DS/Global.h"

#ifdef __POSIX__
#include <time.h>
#include <sys/time.h>
#endif

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <time.h>
#include <windows.h>
#endif

class REVLENS_DS_API DS_NanoTimer {
    
public:
    RLP_DEFINE_LOGGER

    DS_NanoTimer();
    
    ~DS_NanoTimer() {
        
    }
    
    void getElapsed(struct timespec* t);
    void getTime(struct timespec* t);
    

private:
    
    #ifdef __MACH__
    clock_serv_t _cclock;
    #endif
    
    #ifdef _WIN32
    LARGE_INTEGER _frequency;
    LARGE_INTEGER _lastTime;
    #endif
    
    #ifdef __linux__
    timespec _lastTime;
    #endif

};

#endif

