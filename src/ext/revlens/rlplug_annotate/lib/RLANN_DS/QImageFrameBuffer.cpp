//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLANN_DS/QImageFrameBuffer.h"

RLP_SETUP_EXT_LOGGER(RLANN_DS, QImageFrameBuffer)

RLANN_DS_QImageFrameBuffer::RLANN_DS_QImageFrameBuffer(
    QObject* owner, int width, int height):
    DS_FrameBuffer(owner, QVariantMap()),
    _image(nullptr),
    _isDirty(true)
{
    _bufferMetadata.insert("width", width);
    _bufferMetadata.insert("height", height);
}


RLANN_DS_QImageFrameBuffer::RLANN_DS_QImageFrameBuffer(
    QObject* owner, QVariantMap bufferMetadata):
    DS_FrameBuffer(owner, bufferMetadata),
    _image(nullptr),
    _isDirty(true)
{
}


void
RLANN_DS_QImageFrameBuffer::reallocate()
{
    int width = _bufferMetadata.value("width").value<int>();
    int height = _bufferMetadata.value("height").value<int>();
    
    // RLP_LOG_DEBUG(width << " " << height);

    /*
    if ((width == _width) && (height == _height)) {
        return;
    }
    */

    if ((_image != nullptr) && (!_image->isNull()))
    {
        delete _image;
    }

    /// RLP_LOG_DEBUG("Allocating new QImage");

    // _image = new QImage(width, height, QImage::Format_RGBA8888);
    _image = new QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    _image->fill(Qt::transparent);
    
    // _image = new QImage(width, height, QImage::Format_RGBA8888_Premultiplied);
    // _image = new QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    
    int channelCount = 4; // _image->bitPlaneCount() / 8;
    int pixelSize = 1; // _image->depth() / 8;
    _bufferMetadata.insert("pixelSize", pixelSize);
    
    _width = width;
    _height = height;
    _pixelSize = pixelSize;
    _channelOrder = DS_FrameBuffer::CHANNEL_ORDER_RGBA;
    _channelCount = 4;
    _dataBytes = (_width * _height * _pixelSize);

    setReady(true);
}


bool
RLANN_DS_QImageFrameBuffer::deallocate()
{
    // RLP_LOG_DEBUG("");
    
    delete _image;
    _image = nullptr;

    setReady(false);

    return true;
}


void
RLANN_DS_QImageFrameBuffer::loadFromImage(QImage& img)
{
    delete _image;
    _image = new QImage(img.convertToFormat(QImage::Format_ARGB32_Premultiplied));

    _width = _image->width();
    _height = _image->height();
    _pixelSize = _image->depth() / 8;
    _dataBytes = (_width * _height * _pixelSize);
    _channelOrder = DS_FrameBuffer::CHANNEL_ORDER_RGBA;
    _channelCount = 4;

    _bufferMetadata.insert("width", _width);
    _bufferMetadata.insert("height", _height);
    _bufferMetadata.insert("pixelSize", _pixelSize);

    setReady(true);
}


bool
RLANN_DS_QImageFrameBuffer::loadFromFile(QString& file)
{
    RLP_LOG_DEBUG(file);

    QImage img(file);
    img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    
    img = img.scaled(_image->width(), _image->height(), Qt::KeepAspectRatioByExpanding);
    // RLP_LOG_DEBUG(img.width() << " " << img.height() << " " << img.depth() << " " << img.byteCount() << " " << _dataBytes);

    _image->swap(img);

    return true;
}


void
RLANN_DS_QImageFrameBuffer::loadFromData(QByteArray& data, const char* format)
{
    RLP_LOG_DEBUG("");

    QImage img = QImage::fromData(data, format);
    loadFromImage(img);
}


RLANN_DS_QImageFrameBuffer::~RLANN_DS_QImageFrameBuffer()
{
    RLP_LOG_DEBUG("");

    delete _image;
}


void*
RLANN_DS_QImageFrameBuffer::data()
{
    return (void*)_image->bits();
}


bool
RLANN_DS_QImageFrameBuffer::isDirty()
{
    bool result;

    _lock.lock();
    result = _isDirty;
    _lock.unlock();

    return result;
}


void
RLANN_DS_QImageFrameBuffer::setDirty(bool value)
{
    _lock.lock();
    _isDirty = value;
    _lock.unlock();
}