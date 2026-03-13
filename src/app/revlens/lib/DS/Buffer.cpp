//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/Buffer.h"


void
DS_Buffer::reallocate(QVariantMap bufferMetadata)
{
    _bufferMetadata = bufferMetadata;
    reallocate();
}


void
DS_Buffer::release()
{
    DS_BufferListIterator it;
    for (it=_bufferList.begin(); it != _bufferList.end(); ++it)
    {
        (*it)->release();
    }
}


int
DS_Buffer::getDataBytes(bool auxBuffers)
{
    if (!auxBuffers)
    {
        return _dataBytes;
    }

    int total = _dataBytes;
    for (int i=0; i != _bufferList.size(); ++i)
    {
        total += _bufferList.at(i)->getDataBytes(/*auxBuffers=*/true);
    }

    return total;
}


void
DS_Buffer::copyTo(DS_BufferPtr destBuffer, bool metadata)
{
    destBuffer->reallocate(_bufferMetadata);

    if ((destBuffer->data() != nullptr) && (data() != nullptr))
    {
        std::memcpy(destBuffer->data(), data(), _dataBytes);
    } else
    {
        qInfo() << "Buffer::copyTo(): MEMORY COPY FAILED";
    }

    if (metadata)
    {
        destBuffer->getBufferMetadata()->clear();
        destBuffer->getBufferMetadata()->insert(_bufferMetadata);
    }

    destBuffer->setAuxBufferList(_bufferList);
}


QVariantMap
DS_Buffer::getBufferMetadataSafeCopy()
{
    QMutexLocker m(&_lock);

    QVariantMap result = _bufferMetadata;
    return result;
}


int
DS_Buffer::numAuxBuffers(bool includePrimary)
{
    int bufListSize = _bufferList.size();
    if ((includePrimary) && (_dataBytes > 0))
    {
        bufListSize += 1;
    }

    return bufListSize;
}


void
DS_Buffer::appendAuxBuffer(DS_BufferPtr buf)
{
    buf->setParent(this);
    _bufferList.push_back(buf);
}