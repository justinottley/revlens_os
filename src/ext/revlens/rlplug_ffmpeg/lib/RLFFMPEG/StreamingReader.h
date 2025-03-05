

#ifndef EXTREVLENS_RLFFMPEG_STREAMING_READER_H
#define EXTREVLENS_RLFFMPEG_STREAMING_READER_H

#include "RlpExtrevlens/RLFFMPEG/VideoReader3.h"

class RLFFMPEG_StreamingReader : public RLFFMPEG_VideoReader {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLFFMPEG_StreamingReader(QVariantMap* properties);

    void init();

    void readVideo(DS_TimePtr timeInfo,
                   DS_BufferPtr destBuffer,
                   bool postUpdate = true);
};

#endif
