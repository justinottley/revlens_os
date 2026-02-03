
//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_VIDEO_TIMERCLOCK_H
#define REVLENS_VIDEO_TIMERCLOCK_H

#include "RlpRevlens/VIDEO/Global.h"
#include "RlpRevlens/VIDEO/ClockBase.h"


// NOTE: uncomment this to use timer, dont wanna accidentally redefine thi
// if using the threaded clock
//
// TODO FIXME: implementation specific drift threshold
//
// #ifdef DRIFT_THRESHOLD
// #undef DRIFT_THRESHOLD
// #endif
// 
// #define TIMER_DRIFT_THRESHOLD 1000000


/*
 * A frame rate clock implemented via QTimer. This is *not ideal* as the
 * precicion of QTimer is not great, but is a solve for purely single
 * threaded operation.
 */
class REVLENS_VIDEO_API VIDEO_TimerClock : public VIDEO_ClockBase {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    VIDEO_TimerClock(VIDEO_SyncTarget* target,
                     float frameRate);    


public slots:
        
    void updateVideo();
    void resume();
    void stop();

private:
    QTimer* _qtimer;
    // int _interval; // interval in milliseconds
    uint64_t _interval;
};

#endif

