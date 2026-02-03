//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_PLAYBACKMODE_H
#define REVLENS_DS_PLAYBACKMODE_H

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Session.h"


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
        LOOP,
        SWING
    };

    enum IncMode {
        INC,
        DEC
    };

    typedef std::pair<qlonglong, DirectionMode> FrameInfo;

    DS_PlaybackMode();

    DirectionMode getDirection() { return _directionMode; }

    void setDirection(DirectionMode direction)
    {
        _directionMode = direction;
    }

    LoopMode getLoopMode() { return _loopMode; }

    void setLoopMode(LoopMode mode);

    virtual FrameInfo getNextFrameNum(qlonglong frameNum, DirectionMode directionMode = FORWARD, IncMode incMode = INC);


public slots:

    qlonglong getFrameCount();

    qlonglong getNextFrameNum(qlonglong frameNum, int incMode)
    {
        return getNextFrameNum(frameNum, FORWARD, (IncMode)incMode).first;
    }

    qlonglong getFrameIncrement() { return _frameInc; }

    void setFrameRange(qlonglong startFrame, qlonglong endFrame);
    void setInFrame(qlonglong inFrame);
    void setOutFrame(qlonglong outFrame);

    void setSession(DS_SessionPtr session) { _session = session; }

    qlonglong getInFrame() { return _startFrame; }
    qlonglong getOutFrame() { return _endFrame; }

    qlonglong clampFrame(qlonglong inputFrame);

public:

    const int INVALID_FRAME = INT_MIN;

protected:

    qlonglong _startFrame;
    qlonglong _endFrame;
    qlonglong _frameInc;

    DirectionMode _directionMode;
    LoopMode _loopMode;

    DS_SessionPtr _session;
};

#endif

