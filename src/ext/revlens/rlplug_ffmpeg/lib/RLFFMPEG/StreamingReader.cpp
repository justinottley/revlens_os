
#include "RlpExtrevlens/RLFFMPEG/StreamingReader.h"

RLFFMPEG_StreamingReader::RLFFMPEG_StreamingReader(QVariantMap* properties):
    RLFFMPEG_VideoReader(properties)
{
}


void
RLFFMPEG_StreamingReader::init()
{
    openMedia();
}


void
RLFFMPEG_StreamingReader::readVideo(DS_TimePtr timeInfo, DS_BufferPtr destBuffer, bool postUpdate)
{
    if ((destBuffer == nullptr) ||
        (!destBuffer->isValid()))
    {
        RLP_LOG_ERROR("invalid dest buffer, abort read")
        return;
    }

    if (_lastReadResult.av_frame != NULL)
    {
        av_frame_unref(_lastReadResult.av_frame);
        av_frame_free(&_lastReadResult.av_frame);
    }

    RLFFMPEG_PacketReadResult p_ret = readNextPacket();

    _lastReadResult = p_ret;

    // convert to destination format
    //
    uint8_t* destBufferData = (uint8_t*)destBuffer->data();
    sws_scale(
        _swsCtx,
        (const uint8_t **)(p_ret.av_frame->data),
        p_ret.av_frame->linesize,
        0,
        _streamDecCtx->height,
        &destBufferData,
        _rgbLineSize
    );

    destBuffer->getBufferMetadata()->insert("video.readResult", p_ret.return_status);

    // Set channel order to BGRA
    //
    dynamic_cast<DS_FrameBuffer*>(destBuffer.get())->setChannelOrder(
        DS_FrameBuffer::CHANNEL_ORDER_BGRA);


    if (postUpdate)
    {

        // RLP_LOG_DEBUG("POST UPDATE "  << timeInfo->mediaVideoFrame() << destBuffer->getOwner())

        QApplication::postEvent(destBuffer->getOwner(),
                                new MEDIA_ReadEvent(timeInfo,
                                                    getProperty<int>("index"),
                                                    destBuffer,
                                                    MEDIA_FrameReadEventType));
    }

}