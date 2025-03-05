

#include "RlpExtrevlens/RLOIIO/FrameBuffer.h"

RLP_SETUP_EXT_LOGGER(RLOIIO, FrameBuffer)

QImage
RLOIIO_FrameBuffer::toQImage()
{
    RLP_LOG_DEBUG(_width << _height)

    if (_channelCount == 3)
    {
        QImage result = QImage((uchar*)_data, _width, _height, QImage::Format_RGB888);
        return result.copy();
    } else if (_channelCount == 4 )
    {
        QImage result = QImage((uchar*)_data, _width, _height, QImage::Format_RGBA8888);
        return result.copy();
    }

    QImage result;
    return result;
}