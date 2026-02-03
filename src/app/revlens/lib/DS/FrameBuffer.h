//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_FRAMEBUFFER_H
#define REVLENS_DS_FRAMEBUFFER_H

#include <memory>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Buffer.h"


class DS_FrameBuffer;
typedef std::shared_ptr<DS_FrameBuffer> DS_FrameBufferPtr;

typedef QMap<QString, DS_FrameBufferPtr> DS_FrameBufferMap;
typedef QMap<QString, DS_FrameBufferPtr>::iterator DS_FrameBufferMapIterator;

Q_DECLARE_METATYPE(DS_FrameBufferPtr)

typedef QMap<qlonglong, QImage> ThumbCacheMap;

class DS_GLTextureInfo {

public:
    DS_GLTextureInfo(QString name, int textureId, int textureUnit):
        _name(name),
        _textureId(textureId),
        _textureUnit(textureUnit)
    {
    }

    QString getName() { return _name; }
    int getGLTextureId() { return _textureId; }
    int getGLTextureUnit() { return _textureUnit; }

private:
    QString _name;
    int _textureId;
    int _textureUnit;
};


class REVLENS_DS_API DS_FrameBuffer : public DS_Buffer {

public:
    RLP_DEFINE_LOGGER

    enum ChannelOrder {
        CHANNEL_ORDER_BGRA,
        CHANNEL_ORDER_RGBA,
        CHANNEL_ORDER_RGB,
        CHANNEL_ORDER_ST,
        CHANNEL_ORDER_YUV
    };

    enum ColourSpace {
        COLSPACE_NONLINEAR,
        COLSPACE_LINEAR,
        COLSPACE_XYZ
    };

    enum FrameQuality {
        FRAME_QUALITY_ANY,
        FRAME_QUALITY_FAST,
        FRAME_QUALITY_FULL
    };

    static QMap<QString, DS_GLTextureInfo*> GL_TEXTURE_INFO;

    static bool registerGLTextureName(QString name);
    static DS_GLTextureInfo* getGLTextureInfoByName(QString name);

    static void exportFramebufferList(DS_FrameBuffer* fbuf, QVariantList* result);

public:

    DS_FrameBuffer(QObject* owner, QVariantMap bufferMetadata);

    // flag that this framebuffer can be reused for a new frame
    static void discardForReuse(DS_FrameBufferPtr fbuf); 
    static DS_FrameBufferPtr takeFirstFromBufRing(int dataSize);

    virtual void reallocate();

    // Why is this not inheriting from DS_Buffer??
    virtual void reallocate(QVariantMap bufferMetadata)
    {
        _bufferMetadata = bufferMetadata;
        reallocate();
    }

    virtual int getChannelOrder()
    {
        return _channelOrder;
    }


    virtual void setChannelOrder(ChannelOrder order)
    {
        _channelOrder = (int)order;
        _bufferMetadata.insert("channelOrder", order);
    }


    virtual int getColourSpace()
    {
        return _colourSpace;
    }

    virtual void setColourSpace(ColourSpace cspace)
    {
        _colourSpace = (int)cspace;
        _bufferMetadata.insert("colourSpace", cspace);
    }

    virtual int getGLTextureFormat()
    {
        return _glTextureFormat;
    }

    virtual void setGLTextureFormat(int format)
    {
        _glTextureFormat = format;
        _bufferMetadata.insert("glTextureFormat", format);
    }


    virtual int getGLTextureInternalFormat()
    {
        return _glTextureInternalFormat;
    }

    virtual void setGLTextureInternalFormat(int format)
    {
        _glTextureInternalFormat = format;
        _bufferMetadata.insert("glTextureInternalFormat", format);
    }


    virtual int getGLTextureType()
    {
        return _glTextureType;
    }

    virtual void setGLTextureType(int type)
    {
        _glTextureType = type;
        _bufferMetadata.insert("glTextureType", type);
    }


    QString getGLTextureName()
    {
        return _glTextureName;
    }


    QList<QString> getNamedBuffers()
    {
        return _bufferMap.keys();
    }

    DS_FrameBufferPtr getNamedBuffer(QString name)
    {
        return _bufferMap.value(name);
    }

    void setNamedBuffer(QString name, DS_FrameBufferPtr buffer)
    {
        _bufferMap.insert(name, buffer);
    }

    void setFrameNum(long frameNum) { _frameNum = frameNum; }

    inline int getWidth() { return _width; }
    inline int getHeight() { return _height; }
    inline int getPixelSize() { return _pixelSize; }
    inline int getChannelCount() { return _channelCount; }
    inline long getFrameNum() { return _frameNum; }


    virtual QImage toQImage();

    static QHash<int, QList<DS_FrameBufferPtr>* > BUF_RING;
    static QMutex* BUF_RING_LOCK;


protected:
    int _width;
    int _height;
    int _pixelSize;
    int _channelCount;
    int _bytePadding; // required for some ffmpeg api codecs

    long _frameNum;

    int _channelOrder;
    int _colourSpace;

    QString _glTextureName;
    int _glTextureFormat;
    int _glTextureInternalFormat;
    int _glTextureType;

    DS_FrameBufferMap _bufferMap;

};

class REVLENS_DS_API DS_NullFrameBuffer : public DS_FrameBuffer {
    
public:
    
    DS_NullFrameBuffer(QObject* owner):
        DS_FrameBuffer(owner, QVariantMap())
    {
        _bufferMetadata.insert("width", 0);
        _bufferMetadata.insert("height", 0);
        _bufferMetadata.insert("channelCount", 0);
        _bufferMetadata.insert("pixelSize", 0);
    }
    
    
};


#endif

