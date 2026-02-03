

#include "RlpExtrevlens/RLOIIO/FrameBuffer.h"

RLP_SETUP_EXT_LOGGER(RLOIIO, FrameBuffer)

QImage
RLOIIO_FrameBuffer::toQImage()
{
    if (_bufferMetadata.contains("media.full_path"))
    {
        QString fullPath =  _bufferMetadata.value("media.full_path").toString();

        RLP_LOG_DEBUG(_width << _height << fullPath)

        OIIO::ImageBuf buf(fullPath.toStdString());

        QImage result(_width, _height, QImage::Format_RGB888);
        
        for (int i = 0; i < _height; ++i)
        {
            buf.get_pixels(OIIO::ROI(0, _width, i, i+1, 0, 1, 0, 3/*nchannels*/), OIIO::TypeDesc::UINT8, result.scanLine(i));
        }

        return result;
    }


    RLP_LOG_WARN("may result in skewing, provide full path in framebuffer instead")

    if (_channelCount == 3)
    {
        QImage result = QImage((uchar*)_data, _width, _height, QImage::Format_RGB888);
        return result.copy();
    } else if (_channelCount == 4 )
    {
        QImage result = QImage((uchar*)_data, _width, _height, QImage::Format_RGBA8888);
        return result.copy();
    }

    RLP_LOG_ERROR("Invalid channel count, returning empty image")

    QImage result;
    return result;
}