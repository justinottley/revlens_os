//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_QIMAGEFRAMEBUFFER_H
#define EXTREVLENS_RLANN_DS_QIMAGEFRAMEBUFFER_H

#include "RlpExtrevlens/RLANN_DS/Global.h"

#include "RlpRevlens/DS/FrameBuffer.h"


class RLANN_DS_QImageFrameBuffer;
typedef std::shared_ptr<RLANN_DS_QImageFrameBuffer> RLANN_DS_QImageFrameBufferPtr;

class EXTREVLENS_RLANN_DS_API RLANN_DS_QImageFrameBuffer : public DS_FrameBuffer {

public:
    RLP_DEFINE_LOGGER


public:
    
    RLANN_DS_QImageFrameBuffer(QObject* owner, int width, int height);
    RLANN_DS_QImageFrameBuffer(QObject* owner, QVariantMap bufferMetadata);
    
    virtual ~RLANN_DS_QImageFrameBuffer();
    void* data();
    virtual void reallocate();

    // This is defined in DS_Buffer. WHY IS IT NOT BEING FOUND?
    void reallocate(QVariantMap bufferMetadata)
    {
        _bufferMetadata = bufferMetadata;
        reallocate();
    }

    // deallocate / free memory associated with this annotation
    bool deallocate();

    // Modified since last saved to disk
    bool isDirty();
    void setDirty(bool value = true);

    void loadFromImage(QImage& img);
    bool loadFromFile(QString& filePath);
    void loadFromData(QByteArray& data, const char* format="PNG");

    QImage* getQImage() { return _image; }


protected:
    
    QImage* _image;

    bool _isDirty;
};

#endif