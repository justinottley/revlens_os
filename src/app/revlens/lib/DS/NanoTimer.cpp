//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/NanoTimer.h"

RLP_SETUP_LOGGER(revlens, DS, NanoTimer)

DS_NanoTimer::DS_NanoTimer()
{
    #ifdef __MACH__
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &_cclock);
    #endif
    
    #ifdef _WIN32
    if(!QueryPerformanceFrequency(&_frequency)) {
        RLP_LOG_ERROR("QueryPerformanceFrequency() failed")
        // LOG4CPLUS(flogger, "QueryPerformanceFrequency() failed!" );
    }
    
    QueryPerformanceCounter(&_lastTime);
    // RLP_LOG_DEBUG("Frequency: " << _frequency.QuadPart)

    #endif

}


void
DS_NanoTimer::getElapsed(struct timespec* t)
{  
    #ifdef __MACH__
        
        mach_timespec_t mts;
        clock_get_time(_cclock, &mts);
        
        t->tv_sec = mts.tv_sec;
        t->tv_nsec = mts.tv_nsec;
        
    #endif
    
    #ifdef __linux__
        clock_gettime(CLOCK_MONOTONIC, t);
    #endif
    
    #ifdef SCAFFOLD_WASM
        clock_gettime(CLOCK_MONOTONIC, t);
    #endif

    #ifdef _WIN32
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        
        
        LARGE_INTEGER elapsed;
        elapsed.QuadPart = li.QuadPart - _lastTime.QuadPart;
        elapsed.QuadPart *= 1000000000;
        elapsed.QuadPart /= _frequency.QuadPart;

        // RLP_LOG_DEBUG("elapsed: " << elapsed.QuadPart)

        _lastTime = li;

        // LOG4CPLUS(flogger,  "gettime(): " << li.QuadPart ); // " " << (double(li.QuadPart) / _PCFreq) );
        t->tv_sec = 0;
        t->tv_nsec = elapsed.QuadPart; // li.QuadPart; // * 1000;

    #endif
    
}


void
DS_NanoTimer::getTime(struct timespec* t)
{
    #ifdef __MACH__
        
        mach_timespec_t mts;
        clock_get_time(_cclock, &mts);
        
        t->tv_sec = mts.tv_sec;
        t->tv_nsec = mts.tv_nsec;
        
    #endif
    
    #ifdef __linux__
        clock_gettime(CLOCK_MONOTONIC, t);
    #endif
    
    #ifdef SCAFFOLD_WASM
        clock_gettime(CLOCK_MONOTONIC, t);
    #endif
    
    #ifdef _WIN32
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);

        li.QuadPart *= 1000000000;
        li.QuadPart /= _frequency.QuadPart;
        
        t->tv_sec = 0;
        t->tv_nsec = li.QuadPart;

    #endif
}