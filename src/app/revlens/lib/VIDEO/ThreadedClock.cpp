//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

// Qt's QTimer seems to have only millisecond accuracy (even with 
// Qt::PreciseTimer it seems) which is not accurate enough to maintain a 
// stable precise enough framerate. The framerate tends to drift too wildly
// when I tried using QTimer (with Qt::PreciseTimer) 
// Hence revlens has a custom nanosecond accurate timer and sync clock.


#include "RlpRevlens/VIDEO/ThreadedClock.h"
#include "RlpRevlens/DS/TimeConstants.h"


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Windows nanosleep implementation from
// https://github.com/coreutils/gnulib/blob/master/lib/nanosleep.c
//

enum { BILLION = 1000 * 1000 * 1000 };

/* The Windows API function Sleep() has a resolution of about 15 ms and takes
   at least 5 ms to execute.  We use this function for longer time periods.
   Additionally, we use busy-looping over short time periods, to get a
   resolution of about 0.01 ms.  In order to measure such short timespans,
   we use the QueryPerformanceCounter() function.  */

int
win_nanosleep (const struct timespec *requested_delay,
           struct timespec *remaining_delay)
{
  static bool initialized;
  /* Number of performance counter increments per nanosecond,
     or zero if it could not be determined.  */
  static double ticks_per_nanosecond;

  if (requested_delay->tv_nsec < 0 || BILLION <= requested_delay->tv_nsec)
    {
      errno = EINVAL;
      return -1;
    }

  /* For requested delays of one second or more, 15ms resolution is
     sufficient.  */
  if (requested_delay->tv_sec == 0)
    {
      if (!initialized)
        {
          /* Initialize ticks_per_nanosecond.  */
          LARGE_INTEGER ticks_per_second;

          if (QueryPerformanceFrequency (&ticks_per_second))
            ticks_per_nanosecond =
              (double) ticks_per_second.QuadPart / 1000000000.0;

          initialized = true;
        }
      if (ticks_per_nanosecond)
        {
          /* QueryPerformanceFrequency worked.  We can use
             QueryPerformanceCounter.  Use a combination of Sleep and
             busy-looping.  */
          /* Number of milliseconds to pass to the Sleep function.
             Since Sleep can take up to 8 ms less or 8 ms more than requested
             (or maybe more if the system is loaded), we subtract 10 ms.  */
          int sleep_millis = (int) requested_delay->tv_nsec / 1000000 - 10;
          /* Determine how many ticks to delay.  */
          LONGLONG wait_ticks = requested_delay->tv_nsec * ticks_per_nanosecond;
          /* Start.  */
          LARGE_INTEGER counter_before;
          if (QueryPerformanceCounter (&counter_before))
            {
              /* Wait until the performance counter has reached this value.
                 We don't need to worry about overflow, because the performance
                 counter is reset at reboot, and with a frequency of 3.6E6
                 ticks per second 63 bits suffice for over 80000 years.  */
              LONGLONG wait_until = counter_before.QuadPart + wait_ticks;
              /* Use Sleep for the longest part.  */
              if (sleep_millis > 0)
                Sleep (sleep_millis);
              /* Busy-loop for the rest.  */
              for (;;)
                {
                  LARGE_INTEGER counter_after;
                  if (!QueryPerformanceCounter (&counter_after))
                    /* QueryPerformanceCounter failed, but succeeded earlier.
                       Should not happen.  */
                    break;
                  if (counter_after.QuadPart >= wait_until)
                    /* The requested time has elapsed.  */
                    break;
                }
              goto done;
            }
        }
    }
  /* Implementation for long delays and as fallback.  */
  Sleep (requested_delay->tv_sec * 1000 + requested_delay->tv_nsec / 1000000);

 done:
  /* Sleep is not interruptible.  So there is no remaining delay.  */
  if (remaining_delay != NULL)
    {
      remaining_delay->tv_sec = 0;
      remaining_delay->tv_nsec = 0;
    }
  return 0;
}
#endif


RLP_SETUP_LOGGER(revlens, VIDEO, ThreadedClockTimer)
RLP_SETUP_LOGGER(revlens, VIDEO, ThreadedClock)

VIDEO_ThreadedClockTimer::VIDEO_ThreadedClockTimer(
    VIDEO_SyncTarget* target, 
    float frameRate):
        VIDEO_ClockBase(target, frameRate)
{
    _runLock = new QMutex();
    _run = new QWaitCondition();

    // _vsync = new QWaitCondition();
    // _vsync->wakeAll();
    
    _vmutex = new QMutex();
}


void
VIDEO_ThreadedClockTimer::setTargetFrameRate(float frameRate)
{
    QMutexLocker lock(_vmutex);
    
    VIDEO_ClockBase::setTargetFrameRate(frameRate);
}


void
VIDEO_ThreadedClockTimer::updateVideo()
{
    // RLP_LOG_DEBUG("")


    QMetaObject::invokeMethod(_syncTarget,
                              "displayNextFrame",
                              Qt::BlockingQueuedConnection);


    // RLP_LOG_DEBUG( << "VIDEO_ThreadedClockTimer::updateVideo() " <<  std::endl;

    _timer.getTime(&_vsyncNextTime);


    // RLP_LOG_DEBUG(_vsyncTime.tv_sec);
    // RLP_LOG_DEBUG(_vsyncTime.tv_nsec);


    // RLP_LOG_DEBUG("TICK DIFF: " << (_vsyncNextTime.tv_nsec - _vsyncTime.tv_nsec));

    uint64_t currNanoTime = (_vsyncTime.tv_sec * TIME_SEC_SCALE) + _vsyncTime.tv_nsec;
    uint64_t newNanoTime = (_vsyncNextTime.tv_sec * TIME_SEC_SCALE) + _vsyncNextTime.tv_nsec;
    
    uint64_t nanodiff = newNanoTime - currNanoTime;
  
    int64_t drift = nanodiff - (_sleepInterval.tv_sec + _rate);
    
    _vsyncTime.tv_sec = _vsyncNextTime.tv_sec;
    _vsyncTime.tv_nsec = _vsyncNextTime.tv_nsec;
  

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
    
    // RLP_LOG_DEBUG("nanodiff: " << nanodiff << "drift:" << drift << "rate:" << _rate << "_rateDiff:" << _rateDiff)
    // RLP_LOG_DEBUG(_sleepInterval.tv_sec << _sleepInterval.tv_nsec);
    // RLP_LOG_DEBUG("rate: " << _sleepInterval.tv_sec + _rate);
    // RLP_LOG_DEBUG("drift: " << drift);
    // RLP_LOG_DEBUG("_rateDiff: " << _rateDiff);
    
    _sleepInterval.tv_sec = 0;
    _sleepInterval.tv_nsec = _rateDiff; // _rate;
    // _sleepInterval.tv_nsec = 41701417;
    
    
    
    #ifdef _WIN32
    win_nanosleep(&_sleepInterval, NULL);

    #else
    nanosleep(&_sleepInterval, NULL);

    #endif
}


void
VIDEO_ThreadedClockTimer::stop()
{
    stopRunning();
}


void
VIDEO_ThreadedClockTimer::resume()
{
    RLP_LOG_DEBUG("");
    
    setRunning();
    
    _resetDrift = true;
    
    _run->wakeAll();
}


void
VIDEO_ThreadedClockTimer::setRunning()
{
    QMutexLocker lock(_vmutex);
    
    _running = true;
}


void
VIDEO_ThreadedClockTimer::stopRunning()
{
    QMutexLocker lock(_vmutex);
    
    _running = false;
}


bool
VIDEO_ThreadedClockTimer::isRunning()
{
    QMutexLocker lock(_vmutex);
    
    bool result = _running;
    return result;
}


void
VIDEO_ThreadedClockTimer::run()
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


// -----
// -----

VIDEO_ThreadedClock::VIDEO_ThreadedClock(
    VIDEO_SyncTarget* target,
    float frameRate):
        VIDEO_ClockBase(target, frameRate),
        _timer(new VIDEO_ThreadedClockTimer(target, frameRate)),
        _thread()
{
    connect(&_thread, SIGNAL(started()), _timer, SLOT(run()));
    _timer->moveToThread(&_thread);

    _thread.start(QThread::TimeCriticalPriority);
}