//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/QImageFrameBuffer.h"

RLP_SETUP_LOGGER(revlens, DS, QImageFrameBuffer)

DS_QImageFrameBuffer::DS_QImageFrameBuffer(
    QObject* owner, int width, int height, QImage::Format format):
    DS_FrameBuffer(owner, QVariantMap()),
    _image(nullptr),
    _isDirty(true)
{
    _bufferMetadata.insert("width", width);
    _bufferMetadata.insert("height", height);
    _bufferMetadata.insert("format", (int)format);
}


DS_QImageFrameBuffer::DS_QImageFrameBuffer(
    QObject* owner, QVariantMap bufferMetadata, QImage::Format format):
    DS_FrameBuffer(owner, bufferMetadata),
    _image(nullptr),
    _isDirty(true)
{
    _bufferMetadata.insert("format", (int)format);
}


void
DS_QImageFrameBuffer::reallocate()
{

    // RLP_LOG_DEBUG("")
    int width = _bufferMetadata.value("width").toInt();
    int height = _bufferMetadata.value("height").toInt();
    QImage::Format format = (QImage::Format)_bufferMetadata.value("format").toInt();

    int channelCount = 4; // _image->bitPlaneCount() / 8;
    int pixelSize = 1; // _image->depth() / 8;
    _bufferMetadata.insert("pixelSize", pixelSize);

    _width = width;
    _height = height;
    _pixelSize = pixelSize;
    _format = format;
    _channelCount = channelCount;
    _channelOrder = DS_FrameBuffer::CHANNEL_ORDER_RGBA;
    _dataBytes = (_width * _height * _channelCount * _pixelSize);

    // RLP_LOG_DEBUG(width << " " << height);

    /*
    if ((width == _width) && (height == _height)) {
        return;
    }
    */

    if ((_image != nullptr) &&
        (!_image->isNull()) &&
        (_image->sizeInBytes() != _dataBytes))
    {
        RLP_LOG_WARN("Freeing existing image, size mismatch")
        delete _image;
    }

    if (_image == nullptr)
    {
        _image = new QImage(width, height, format);
        // _image->fill(Qt::transparent);
    }

    setReady(true);
}


bool
DS_QImageFrameBuffer::deallocate()
{
    // RLP_LOG_DEBUG("");

    delete _image;
    _image = nullptr;

    setReady(false);

    return true;
}


void
DS_QImageFrameBuffer::loadFromImage(QImage& img)
{
    delete _image;
    _image = new QImage(img.convertToFormat(QImage::Format_RGBA8888));

    _width = _image->width();
    _height = _image->height();
    _pixelSize = _image->depth() / 8;
    _channelCount = 4;
    _channelOrder = DS_FrameBuffer::CHANNEL_ORDER_RGBA;
    _dataBytes = (_width * _height * _pixelSize);

    _bufferMetadata.insert("width", _width);
    _bufferMetadata.insert("height", _height);
    _bufferMetadata.insert("pixelSize", _pixelSize);
}


bool
DS_QImageFrameBuffer::loadFromFile(QString& file)
{
    RLP_LOG_DEBUG(file);

    QImage img(file);
    img = img.convertToFormat(_format);

    img = img.scaled(_image->width(), _image->height(), Qt::KeepAspectRatioByExpanding);
    // RLP_LOG_DEBUG(img.width() << " " << img.height() << " " << img.depth() << " " << img.byteCount() << " " << _dataBytes);

    _image->swap(img);

    return true;
}


void
DS_QImageFrameBuffer::loadFromData(QByteArray& data, const char* format)
{
    RLP_LOG_DEBUG("");

    QImage img = QImage::fromData(data, format);
    loadFromImage(img);
}


DS_QImageFrameBuffer::~DS_QImageFrameBuffer()
{
    RLP_LOG_DEBUG("");

    delete _image;
}


void*
DS_QImageFrameBuffer::data()
{
    return (void*)_image->bits();
}


QImage
DS_QImageFrameBuffer::toQImage()
{
    QImage result = _image->copy();
    return result.rgbSwapped();
}


bool
DS_QImageFrameBuffer::isDirty()
{
    bool result;

    _lock.lock();
    result = _isDirty;
    _lock.unlock();

    return result;
}


void
DS_QImageFrameBuffer::setDirty(bool value)
{
    _lock.lock();
    _isDirty = value;
    _lock.unlock();
}