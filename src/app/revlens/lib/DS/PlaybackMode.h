//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_PLAYBACKMODE_H
#define REVLENS_DS_PLAYBACKMODE_H

#include <climits>

#include "RlpCore/LOG/Logging.h"
#include "RlpRevlens/DS/Global.h"

class REVLENS_DS_API DS_PlaybackMode : public QObject {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    enum DirectionMode {
        FORWARD,
        BACKWARD
    };
    
    enum LoopMode {
        NONE,
        LOOP
        // SWING // Not implemented yet 
    };
    
    DS_PlaybackMode();
    
    
    DirectionMode getDirection() { return _directionMode; }
    
    void setDirection(DirectionMode direction) {
        _directionMode = direction;
    }
    
    LoopMode loopMode() { return _loopMode; }

    void setLoopMode(LoopMode mode) {
        _loopMode = mode;
    }
    
    qlonglong getNextFrameNum(qlonglong frameNum, DirectionMode direction = FORWARD);


public slots:
    
    qlonglong getFrameCount();

    qlonglong getNextFrameNum(qlonglong frameNum, int direction) {
        return getNextFrameNum(frameNum, (DirectionMode)direction);
    }

    qlonglong getFrameIncrement() { return _frameInc; }
    
    void setFrameRange(qlonglong startFrame, qlonglong endFrame) {

        RLP_LOG_DEBUG(startFrame << " " << endFrame);

        _startFrame = startFrame;
        _endFrame = endFrame;
    }
    
    void setInFrame(qlonglong inFrame) {

        RLP_LOG_DEBUG(inFrame);

        _startFrame = inFrame;
    }
    
    void setOutFrame(qlonglong outFrame) {

        RLP_LOG_DEBUG(outFrame);
        
        _endFrame = outFrame;
    }
    
    qlonglong getInFrame() { return _startFrame; }
    qlonglong getOutFrame() { return _endFrame; }
    
    
    qlonglong clampFrame(qlonglong inputFrame);
    
public:
    
    const int INVALID_FRAME = INT_MIN;
    
private:
    
    
    qlonglong _startFrame;
    qlonglong _endFrame;
    qlonglong _frameInc;
    
    DirectionMode _directionMode;
    LoopMode _loopMode;
};

#endif

