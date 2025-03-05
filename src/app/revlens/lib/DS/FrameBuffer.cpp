//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/FrameBuffer.h"

RLP_SETUP_LOGGER(app, DS, FrameBuffer)

QHash<int, QList<DS_FrameBufferPtr>* > DS_FrameBuffer::BUF_RING;
QMutex* DS_FrameBuffer::BUF_RING_LOCK = new QMutex();

DS_FrameBuffer::DS_FrameBuffer(QObject* owner, QVariantMap bufferMetadata):
    DS_Buffer(owner, bufferMetadata),
    _width(0),
    _height(0),
    _pixelSize(0),
    _channelCount(0),
    _channelOrder(0),
    _colourSpace(0)
{
    // Set some default values for metadata
    //
    setChannelOrder(CHANNEL_ORDER_RGB);
    setColourSpace(COLSPACE_NONLINEAR);
    setGLTextureFormat(GL_RGBA);
    setGLTextureInternalFormat(GL_RGBA);
    setGLTextureType(GL_UNSIGNED_BYTE);
}


void
DS_FrameBuffer::reallocate()
{
    int width = _bufferMetadata.value("width").toInt();
    int height = _bufferMetadata.value("height").toInt();
    int channelCount = _bufferMetadata.value("channelCount").toInt();
    int pixelSize = _bufferMetadata.value("pixelSize").toInt();

    _channelOrder = _bufferMetadata.value("channelOrder").toInt();
    _colourSpace = _bufferMetadata.value("colourSpace").toInt();
    
    _glTextureFormat = _bufferMetadata.value("glTextureFormat").toInt();
    _glTextureInternalFormat = _bufferMetadata.value("glTextureInternalFormat").toInt();
    _glTextureType = _bufferMetadata.value("glTextureType").toInt();
    
    if ((width == _width) &&
        (height == _height) &&
        (channelCount == _channelCount) &&
        (pixelSize == _pixelSize))
    {
        return;
    }
    
    _width = width;
    _height = height;
    _channelCount = channelCount;
    _pixelSize = pixelSize;

    _dataBytes = (_width * _height * _channelCount * _pixelSize);

    _data = std::malloc(_dataBytes);
}


void
DS_FrameBuffer::discardForReuse(DS_FrameBufferPtr fbuf)
{
    fbuf->setValid(false);
    fbuf->setReady(false);

    int dataBytes = fbuf->getDataBytes();

    DS_FrameBuffer::BUF_RING_LOCK->lock();

    if (DS_FrameBuffer::BUF_RING.contains(dataBytes))
    {
        DS_FrameBuffer::BUF_RING.value(dataBytes)->push_back(fbuf);
        // RLP_LOG_DEBUG("size for" << dataBytes << ":" << DS_FrameBuffer::BUF_RING.value(dataBytes)->size())

    } else
    {
        QList<DS_FrameBufferPtr>* l = new QList<DS_FrameBufferPtr>();
        DS_FrameBuffer::BUF_RING.insert(dataBytes, l);
        l->push_back(fbuf);

        
    }

    DS_FrameBuffer::BUF_RING_LOCK->unlock();

    for (int abx = 0; abx != fbuf->numAuxBuffers(/*includePrimary=*/false); ++abx)
    {
        DS_FrameBufferPtr auxFbuf = std::static_pointer_cast<DS_FrameBuffer>(
            fbuf->getAuxBuffer(abx));

        DS_FrameBuffer::discardForReuse(auxFbuf);
    }

    fbuf->clearAuxBufferList();
}


DS_FrameBufferPtr
DS_FrameBuffer::takeFirstFromBufRing(int dataSize)
{
    QMutexLocker l(DS_FrameBuffer::BUF_RING_LOCK);

    if ((DS_FrameBuffer::BUF_RING.contains(dataSize)) &&
    (!DS_FrameBuffer::BUF_RING.value(dataSize)->empty()))
    {
        // RLP_LOG_DEBUG("Got buffer size" << dataSize << " - " << DS_FrameBuffer::BUF_RING.value(dataSize)->size())

        DS_FrameBufferPtr buf = DS_FrameBuffer::BUF_RING.value(dataSize)->takeFirst();

        // RLP_LOG_DEBUG("size for" << dataSize << ":" << DS_FrameBuffer::BUF_RING.value(dataSize)->size())

        return buf;
    }

    return nullptr;
}


QImage
DS_FrameBuffer::toQImage()
{
    // RLP_LOG_DEBUG(_width << _height << _channelCount)

    QImage result;
    // return result;
    if (_channelCount == 3)
    {
        result = QImage((uchar*)_data, _width, _height, QImage::Format_BGR888);

    } else if (_channelCount == 4)
    {
        result = QImage((uchar*)_data, _width, _height, QImage::Format_RGBA8888);
        return result.rgbSwapped();
    }

    return result;
}
