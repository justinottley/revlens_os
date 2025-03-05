//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_BUFFER_H
#define REVLENS_DS_BUFFER_H

#include <memory>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include "RlpRevlens/DS/Global.h"



class DS_Buffer;
typedef std::shared_ptr<DS_Buffer> DS_BufferPtr;

typedef QList<DS_BufferPtr> DS_BufferList;
typedef QList<DS_BufferPtr>::iterator DS_BufferListIterator;

Q_DECLARE_METATYPE(DS_BufferPtr)

class REVLENS_DS_API DS_Buffer {

public:

    enum ReadResult {
        READ_UNKNOWN,
        READ_OK,
        READ_ERROR,
        READ_EOF,
        READ_EOF_OK,
        READ_NONE
    };

public:

    DS_Buffer(QObject* owner, QVariantMap bufferMetadata) :
        _owner(owner),
        _parent(nullptr),
        _bufferMetadata(bufferMetadata),
        _data(NULL),
        _dataBytes(0),
        _ready(false),
        _valid(true),
        _lock()
    {
        _bufferList.clear();

        _bufferMetadata.insert("layout_scale.x", 1.0);
        _bufferMetadata.insert("layout_scale.y", 1.0);
        _bufferMetadata.insert("layout_pan.x", 0.0);
        _bufferMetadata.insert("layout_pan.y", 0.0);
    }


    virtual ~DS_Buffer()
    {
        if (_data != NULL) {

            std::free(_data);
            _data = NULL;

        }

        _bufferList.clear();
    }


    virtual void release() {
        DS_BufferListIterator it;
        for (it=_bufferList.begin(); it != _bufferList.end(); ++it) {
            (*it)->release();
        }
    }

    virtual void reallocate() {}

    virtual int getDataBytes() { return _dataBytes; }


    void append(DS_BufferPtr buffer) { _bufferList.push_back(buffer); }

    virtual void reallocate(QVariantMap bufferMetadata)
    {
        _bufferMetadata = bufferMetadata;
        reallocate();
    }


    inline void copyTo(DS_BufferPtr destBuffer, bool metadata=true)
    {
        destBuffer->reallocate(_bufferMetadata);
        std::memcpy(destBuffer->data(), data(), _dataBytes);

        if (metadata)
        {
            destBuffer->getBufferMetadata()->clear();
            destBuffer->getBufferMetadata()->insert(_bufferMetadata);
        }

        destBuffer->setAuxBufferList(_bufferList);
    }


    QVariantMap* getBufferMetadata() { return &_bufferMetadata; }

    QVariantMap getBufferMetadataSafeCopy()
    {
        QMutexLocker m(&_lock);

        QVariantMap result = _bufferMetadata;
        return result;
    }

    virtual void* data()
    {
        return _data;
    }

    virtual void setData(void* data)
    {
        _data = data;
    }

    void* data(int index)
    {
        return _bufferList.at(index)->data();
    }

    //
    // Auxiliary Buffers
    //

    virtual int numAuxBuffers(bool includePrimary=true)
    {
        int bufListSize = _bufferList.size();
        if ((includePrimary) && (_dataBytes > 0))
        {
            bufListSize += 1;
        }

        return bufListSize;
    }

    virtual DS_BufferPtr getAuxBuffer(int index)
    {
        return _bufferList.at(index);
    }


    virtual DS_BufferList getAuxBufferList()
    {
        return _bufferList;
    }


    virtual void clearAuxBufferList()
    {
        _bufferList.clear();
    }

    virtual void appendAuxBuffer(DS_BufferPtr buf)
    {
        buf->setParent(this);
        _bufferList.push_back(buf);
    }

    virtual void setAuxBufferList(DS_BufferList bufList)
    {
        _bufferList = bufList;
    }

    // ----

    void setOwner(QObject* owner) { _owner = owner; }

    void setParent(DS_Buffer* parent) { _parent = parent; }

    DS_Buffer* getParent() { return _parent; }

    // inline int setDataBytes(int dataBytes) {  _dataBytes = dataBytes; }

    inline QObject* getOwner() { return _owner; }


    //
    // Concurrency state info
    //

    inline bool isReady()
    {

        bool result;

        _lock.lock();
        result = _ready;
        _lock.unlock();

        return result;
    }

    inline void setReady(bool value = true)
    {

        _lock.lock();
        _ready = value;
        _lock.unlock();
    }


    inline bool isValid()
    {

        bool result;

        _lock.lock();
        result = _valid;
        _lock.unlock();

        return result;
    }


    inline void setValid(bool value = true)
    {
        _lock.lock();
        _valid = value;
        _lock.unlock();
    }

    inline void setInvalid()
    {

        _lock.lock();
        _valid = false;
        _lock.unlock();
    }



protected:

    QObject* _owner;
    DS_Buffer* _parent; // hmm.. should this be refcounted?
    QVariantMap _bufferMetadata;

    void* _data;
    int _dataBytes;

    bool _ready;
    bool _valid;

    QMutex _lock;

    DS_BufferList _bufferList;
};

#endif

