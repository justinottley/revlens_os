//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpExtrevlens/RLMEDIA/Plugin.h"
#include "RlpExtrevlens/RLMEDIA/VideoReader.h"

#include "RlpRevlens/DS/QImageFrameBuffer.h"

RLP_SETUP_LOGGER(ext, RLMEDIA, Plugin)

RLMEDIA_Plugin::RLMEDIA_Plugin() :
    MEDIA_Plugin(QString("RevlensMedia"))
{
    QThreadPool* videoPool = new QThreadPool();
    videoPool->setExpiryTimeout(-1); // dont expire threads
    int threadCountMax = 2;
    #ifdef SCAFFOLD_IOS
    threadCountMax = 2;
    #endif

    videoPool->setMaxThreadCount(threadCountMax);

    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool);
}


// MEDIA_Plugin::MediaType
QVariantList
RLMEDIA_Plugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    // RLP_LOG_DEBUG("")

    QVariantMap properties;
    QVariantList result;

    properties.clear();
    result.clear();

    if (!mediaInput.contains("media.video"))
    {
        // NOTE: early return
        //
        RLP_LOG_DEBUG("No media.video, aborting")
        return result;
    }

    // only attempt to open frame sequence type component
    if (!mediaInput.contains("media.video.resolved_component"))
    {
        RLP_LOG_DEBUG("resolved_component not found, aborting")
        return result;
    }

    QString resolvedComponentType = mediaInput.value("media.video.resolved_component").toString();
    if (resolvedComponentType != "movie")
    {
        RLP_LOG_DEBUG("movie component not found, aborting")
        return result;
    }

    QString videoPath = mediaInput.value("media.video").toString();
    if (videoPath.indexOf(".rlm") == -1)
    {
        RLP_LOG_DEBUG(".rlm extension not found, aborting")
        return result;
    }

    RLP_LOG_DEBUG("Attempting:" << videoPath);

    EdbMedia_Controller c(videoPath);
    if (!c.isValid())
    {
        RLP_LOG_DEBUG("Not an rlmedia container:" << videoPath)
        return result;
    }

    RLP_LOG_DEBUG("Found rlmedia container:" << videoPath)

    QString mainStreamName = "main.full";
    if (!c.hasStreamByName(mainStreamName))
    {
        RLP_LOG_DEBUG("Stream not found:" << mainStreamName)
        RLP_LOG_DEBUG("Container streams:" << c.getAllStreams())
        RLP_LOG_DEBUG("Aborting")
        return result;
    }

    RLP_LOG_DEBUG("OK:" << mainStreamName)

    QVariantMap streamInfo = c.getVideoStreamInfo(mainStreamName);
    if (!streamInfo.contains("codec"))
    {
        RLP_LOG_ERROR("Insufficient stream info")

        return result;
    }

    RLP_LOG_DEBUG("Got stream info:")
    LOG_Logging::pprint(streamInfo, 2);


    // Media name
    //
    QString mediaName = mainStreamName; // TODO FIXME REPLACE
    properties.insert("media.name", mediaName);

    qlonglong startFrame = streamInfo.value("start_frame").toLongLong();
    qlonglong endFrame = streamInfo.value("end_frame").toLongLong();
    qlonglong frameCount = endFrame - startFrame + 1;
    QString videoCodec = streamInfo.value("codec").toString();

    int width = streamInfo.value("width").toInt();
    int height = streamInfo.value("height").toInt();
    int channelCount = 4; // streamInfo.value("channel_count").toInt();
    int pixelSize = 1;
    int byteCount = width * height * channelCount * pixelSize;

    properties.insert("video.start_frame", startFrame);
    properties.insert("video.end_frame", endFrame);

    properties.insert("video.frame_count", frameCount);
    properties.insert("media.frame_count", frameCount);

    properties.insert("video.codec", videoCodec);

    QVariantMap frameInfo;

    frameInfo.insert("width", width);
    frameInfo.insert("height", height);
    frameInfo.insert("pixelSize", pixelSize); // (image.depth() / 8));
    frameInfo.insert("channelCount", channelCount); // image.depth() / 8);
    frameInfo.insert("byteCount", byteCount);
    frameInfo.insert("hasAlpha", true); // image.hasAlphaChannel());
    
    RLP_LOG_DEBUG("  width: " << width);
    RLP_LOG_DEBUG("  height: " << height);
    // RLP_LOG_DEBUG("  depth: " << image.depth());
    // RLP_LOG_DEBUG("  bitPlaneCount: " << image.bitPlaneCount());
    // RLP_LOG_DEBUG("  bitsPerPixel:" << image.pixelFormat().bitsPerPixel());
    // int srcChannelCount = image.bitPlaneCount() / image.pixelFormat().bitsPerPixel();

    // RLP_LOG_DEBUG("  has alpha:" << image.hasAlphaChannel())

    QVariantList frameInfoList;
    frameInfoList.push_back(frameInfo);
    properties.insert("video.frame_info", frameInfoList);
    properties.insert("video.frame_info.one", frameInfo);

    properties.insert("video.format.type", MEDIA_Plugin::VIDEO_FRAMES);
    properties.insert("video.format.reader", name());
    properties.insert("video.colourspace", "sRGB");

    float frameRate = 24;
    if (streamInfo.contains("frame_rate"))
    {
        frameRate = streamInfo.value("frame_rate").toFloat();
    }

    RLP_LOG_DEBUG("frame rate:" << frameRate)

    properties.insert("video.frame_rate", frameRate);

    properties.insert("media_type", MEDIA_Plugin::VIDEO_FRAMES);

    result.push_back(properties);


    return result;

}



DS_FrameBufferPtr
RLMEDIA_Plugin::getOrCreateFrameBuffer(QObject* owner, QVariantMap frameInfo, bool forceNew)
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

    return DS_FrameBufferPtr(new DS_QImageFrameBuffer(owner, frameInfo, QImage::Format_RGB32));
}


DS_NodePtr
RLMEDIA_Plugin::createVideoReader(QVariantMap* properties)
{
    RLP_LOG_DEBUG("");

    DS_NodePtr node(new RLMEDIA_VideoReader(properties));
    return node;
}


//


RLMEDIA_ReadTask::RLMEDIA_ReadTask(RLMEDIA_VideoReader* reader,
                                   DS_TimePtr timeInfo,
                                   DS_BufferPtr destFrame) :
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLMEDIA_ReadTask::run()
{
    // RLP_LOG_DEBUG("RLMEDIA_ReadTask::run()");
    
    _reader->readVideo(_timeInfo, _destFrame);
}
