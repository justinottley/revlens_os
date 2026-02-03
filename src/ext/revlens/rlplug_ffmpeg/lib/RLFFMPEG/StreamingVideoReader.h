

#ifndef EXTREVLENS_RLFFMPEG_STREAMING_VIDEO_READER_H
#define EXTREVLENS_RLFFMPEG_STREAMING_VIDEO_READER_H

#include "RlpExtrevlens/RLFFMPEG/VideoReader3.h"

class RLFFMPEG_StreamingVideoReader : public RLFFMPEG_VideoReader {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLFFMPEG_StreamingVideoReader(QVariantMap* properties);

    void resetAll();
    void setFlushMode(bool flushModeVal);
    RLFFMPEG_PacketReadResult seekToFrame(qlonglong frameNum);
    // void startCriticalReadSection() { }
    // void endCriticalReadSection() { }
};

#endif
