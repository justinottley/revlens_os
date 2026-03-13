
#include "RlpExtrevlens/RLFFMPEG/StreamingVideoReader.h"

RLP_SETUP_LOGGER(ext, RLFFMPEG, StreamingVideoReader)

RLFFMPEG_StreamingVideoReader::RLFFMPEG_StreamingVideoReader(QVariantMap* properties):
    RLFFMPEG_VideoReader(properties)
{
    RLP_LOG_DEBUG("")
}


void
RLFFMPEG_StreamingVideoReader::resetAll()
{
    RLP_LOG_DEBUG("Doing nothing")
}


void
RLFFMPEG_StreamingVideoReader::setFlushMode(bool flushModeVal)
{
    RLP_LOG_WARN(flushModeVal << "Restting stream")

    closeMedia();
     if (!openMedia())
     {
        RLP_LOG_ERROR("Could not reset stream")
        graph()->emitNoticeEvent("stream_broken");
     }
}


RLFFMPEG_PacketReadResult
RLFFMPEG_StreamingVideoReader::seekToFrame(qlonglong frameNum)
{
    if (frameNum != 0)
    {
        RLP_LOG_WARN(frameNum)
    }
    

    return readNextPacket();
}
