//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_QIMAGEFRAMEBUFFER_H
#define REVLENS_DS_QIMAGEFRAMEBUFFER_H

#include "RlpRevlens/DS/Global.h"

#include "RlpRevlens/DS/FrameBuffer.h"


class DS_QImageFrameBuffer;
typedef std::shared_ptr<DS_QImageFrameBuffer> DS_QImageFrameBufferPtr;

class REVLENS_DS_API DS_QImageFrameBuffer : public DS_FrameBuffer {

public:
    RLP_DEFINE_LOGGER


public:
    
    DS_QImageFrameBuffer(
        QObject* owner,
        int width,
        int height,
        QImage::Format format=QImage::Format_ARGB32_Premultiplied
    );

    DS_QImageFrameBuffer(
        QObject* owner,
        QVariantMap bufferMetadata,
        QImage::Format format=QImage::Format_ARGB32_Premultiplied
    );

    virtual ~DS_QImageFrameBuffer();
    void* data();
    void reallocate();

    // deallocate / free memory associated with this annotation
    bool deallocate();

    // Modified since last saved to disk
    bool isDirty();
    void setDirty(bool value = true);

    void loadFromImage(QImage& img);
    bool loadFromFile(QString& filePath);
    void loadFromData(QByteArray& data, const char* format="PNG");

    QImage* getQImage() { return _image; }
    QImage toQImage();
    QImage::Format format() { return _format; }

protected:

    QImage* _image;
    QImage::Format _format;

    bool _isDirty;
};

#endif