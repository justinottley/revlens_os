
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadPlugin.h"
#include "RlpExtrevlens/RLQTMEDIA/DeferredLoadReader.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkReader.h"


RLP_SETUP_LOGGER(ext, RLQTMEDIA, DeferredLoadPlugin)
RLP_SETUP_LOGGER(ext, RLQTMEDIA, DeferredLoadReadTask)

RLQTMEDIA_DeferredLoadPlugin::RLQTMEDIA_DeferredLoadPlugin():
    MEDIA_Plugin("DeferredLoad"),
    _loadingFrame(nullptr)
{
    disable(); // disallow MEDIA_Factory from using this plugin

    int width = 960;
    int height = 540;
    int channelCount = 4;
    int pixelSize = 1;
    int byteCount = width * height * channelCount * pixelSize;

    _frameInfo.clear();
    _frameInfo.insert("width", width);
    _frameInfo.insert("height", height);
    _frameInfo.insert("channelCount", channelCount);
    _frameInfo.insert("pixelSize", pixelSize);
    _frameInfo.insert("byteCount", byteCount);

    int threadCountMax = 8;
    QThreadPool* videoPool = new QThreadPool();
    videoPool->setMaxThreadCount(threadCountMax);
    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool); // this is for the loading pre-frame only
    _readThreadPoolMap.insert(DS_Node::DATA_LOAD, videoPool);

    QThreadPool* audioPool = new QThreadPool();
    audioPool->setMaxThreadCount(1);
    _readThreadPoolMap.insert(DS_Node::DATA_AUDIO, audioPool);
}


void
RLQTMEDIA_DeferredLoadPlugin::setupReadThreadPool(DS_Node::NodeDataType dataType, MEDIA_Plugin* srcPlugin)
{
    RLP_LOG_DEBUG("Relinking thread pool for data type:" << dataType << ":" << srcPlugin)

    _readThreadPoolMap.insert(dataType, srcPlugin->getReadThreadPool(dataType));
}


DS_FrameBufferPtr
RLQTMEDIA_DeferredLoadPlugin::getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo, bool forceNew)
{
    // RLP_LOG_DEBUG("")

    if (!forceNew)
    {
        DS_FrameBufferPtr buf = getPreallocatedFrameBuffer(frameInfo);
        if (buf)
        {
            return buf;
        }
    }

    DS_FrameBufferPtr fbuf(new DS_QImageFrameBuffer(owner, _frameInfo, QImage::Format_RGB32));
    return fbuf;
}


DS_NodePtr
RLQTMEDIA_DeferredLoadPlugin::createVideoReader(QVariantMap* properties)
{
    // RLP_LOG_DEBUG(*properties)
    QVariantMap srcInfo = properties->value("media.source_info").toMap();
    if (srcInfo.contains("media.remote_url"))
    {
        return DS_NodePtr(new RLQTMEDIA_NetworkReader(properties, DS_Node::DATA_VIDEO));
    }
    
    return DS_NodePtr(new RLQTMEDIA_DeferredLoadReader(properties, DS_Node::DATA_VIDEO));
}


DS_NodePtr
RLQTMEDIA_DeferredLoadPlugin::createAudioReader(QVariantMap* properties)
{
    // need to copy to help ensure audio and video can be loaded concurrently
    QVariantMap* aprops = new QVariantMap();
    aprops->insert(*properties);

    DS_NodePtr node(new RLQTMEDIA_DeferredLoadReader(aprops, DS_Node::DATA_AUDIO));
    return node;
}


QVariantList
RLQTMEDIA_DeferredLoadPlugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    RLP_LOG_DEBUG("")

    QVariantList result;

    if (mediaInput.contains("media.video") &&
        mediaInput.contains("media.frame_count"))
    {

        QVariantMap properties;
        properties.insert("media.source_info", mediaInput);

        QVariant v;
        v.setValue(this);
        properties.insert("media.plugin.loader", v);

        properties.insert("media.name", mediaInput.value("media.name"));
        properties.insert("media.frame_count", mediaInput.value("media.frame_count"));

        QVariantList frameInfoList;
        frameInfoList.push_back(_frameInfo);
        properties.insert("video.frame_info", frameInfoList);
        properties.insert("video.frame_info.one", _frameInfo);
        properties.insert("video.format.type", MEDIA_Plugin::VIDEO_FRAMES);
        properties.insert("video.format.reader", name());
        properties.insert("video.frame_rate", 24);
        properties.insert("media_type", MEDIA_Plugin::VIDEO_FRAMES);

        properties.insert("audio.format.reader", name());

        result.push_back(properties);
    }

    return result;
}


RLQTMEDIA_DeferredLoadReadTask::RLQTMEDIA_DeferredLoadReadTask(
    RLQTMEDIA_DeferredLoadReader* reader,
    DS_TimePtr timeInfo,
    DS_BufferPtr destFrame):
        _reader(reader),
        _timeInfo(timeInfo),
        _destFrame(destFrame)
{
}


void
RLQTMEDIA_DeferredLoadReadTask::run()
{
    _reader->readVideo(_timeInfo, _destFrame);
}
