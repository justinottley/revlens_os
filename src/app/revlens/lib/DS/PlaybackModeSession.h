
#ifndef REVLENS_DS_SESSION_PLAYBACKMODE_H
#define REVLENS_DS_SESSION_PLAYBACKMODE_H

#include "RlpRevlens/DS/Global.h"

#include "RlpRevlens/DS/PlaybackMode.h"


class REVLENS_DS_API DS_SessionPlaybackMode : public DS_PlaybackMode {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    DS_SessionPlaybackMode();

    DS_PlaybackMode::FrameInfo getNextFrameNum(qlonglong frameNum, DirectionMode directionMode = FORWARD, IncMode incMode = INC);

};

#endif
