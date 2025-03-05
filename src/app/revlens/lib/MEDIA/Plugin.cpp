//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/MEDIA/Plugin.h"

RLP_SETUP_LOGGER(revlens, MEDIA, Plugin)

MEDIA_Plugin::MEDIA_Plugin(QString name, int readThreadPoolSize):
    DS_Plugin(name, /*enabled=*/true)
{
}


QVariantList
MEDIA_Plugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    RLP_LOG_ERROR("base class called, should not reach here")

    QVariantList result;
    result.clear();

    return result;
}


QThreadPool*
MEDIA_Plugin::getReadThreadPool(DS_Node::NodeDataType nodeDataType)
{
    QThreadPool* tpool = nullptr;

    if (!(_readThreadPoolMap.contains(nodeDataType)))
    {
        RLP_LOG_ERROR("No thread pool for media type" << (int)nodeDataType);

        #ifdef SCAFFOLD_WASM
        RLP_LOG_WARN("Bypassing QThreadPool construction")
        #else
        // RLP_LOG_DEBUG("Constructing thread pool for media type " << (int)nodeDataType);
        // tpool = new QThreadPool();
        // _readThreadPoolMap.insert(nodeDataType, tpool);
        #endif

    } else
    {

        tpool = _readThreadPoolMap.value(nodeDataType);
    }

    // TODO FIXME: Assert tpool != nullptr

    return tpool;
}


DS_FrameBufferPtr
MEDIA_Plugin::getPreallocatedFrameBuffer(QVariantMap frameInfo)
{
    int width = frameInfo.value("width").toInt();
    int height = frameInfo.value("height").toInt();
    int channelCount = frameInfo.value("channelCount").toInt();
    int pixelSize = frameInfo.value("pixelSize").toInt();

    int dataSize = width * height * channelCount * pixelSize;

    return DS_FrameBuffer::takeFirstFromBufRing(dataSize);
}


DS_FrameBufferPtr
MEDIA_Plugin::getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo, bool forceNew)
{
    if (!forceNew)
    {
        DS_FrameBufferPtr buf = getPreallocatedFrameBuffer(frameInfo);
        if (buf)
        {
            return buf;
        }
    }

    return DS_FrameBufferPtr(new DS_FrameBuffer(owner, frameInfo));
}


int
MEDIA_Plugin::getBytesRequiredForFrame(DS_NodePtr node, qlonglong frameNum)
{
    // RLP_LOG_DEBUG(_name << frameNum)

    QVariantMap frameInfo = node->getProperty<QVariantMap>("video.frame_info.one");
    return frameInfo.value("byteCount").value<int>();
}
