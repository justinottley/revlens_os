//
//  Copyright (c) 2014 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLQPSD_MEDIA/Plugin.h"
#include "RlpExtrevlens/RLQPSD_MEDIA/VideoReader.h"

#include "RlpExtrevlens/QPSD/qpsdhandler.h"


RLP_SETUP_LOGGER(extrevlens, RLQPSD_MEDIA, Plugin)

RLQPSD_MEDIA_Plugin::RLQPSD_MEDIA_Plugin() :
    MEDIA_Plugin(QString("QPsd"))
{
    QThreadPool* videoPool = new QThreadPool();
    videoPool->setMaxThreadCount(4);

    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool);

    RLP_LOG_DEBUG("")
}


QVariantList
RLQPSD_MEDIA_Plugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    // RLP_LOG_DEBUG("")

    QVariantMap properties;
    QVariantList result;

    properties.clear();
    result.clear();

    if (!mediaInput.contains("media.video"))
    {
        return result;
    }

    const QString filePath = mediaInput.value("media.video").value<QString>();  // value<QString>("video_dir");
    int numFiles = 1;

    RLP_LOG_DEBUG("Attempting PSD detection:" << filePath)

    QPsdHandler psdHandler;
    QFile f(filePath);

    f.open(QIODevice::ReadOnly);

    if (!psdHandler.canRead(&f))
    {
        // RLP_LOG_DEBUG("Cannot read PSD")
        return result;
    }


    QImage image;

    psdHandler.setDevice(&f);
    psdHandler.read(&image);

    int width = image.width();
    int height = image.height();
    int channelCount = image.bitPlaneCount() / 8;
    int pixelSize = image.depth() / 8;

    int byteCount = width * height * pixelSize;

    float frameRate = 24.0;

    RLP_LOG_DEBUG("")
    RLP_LOG_DEBUG("  width:  " << width)
    RLP_LOG_DEBUG("  height: " << height)
    RLP_LOG_DEBUG("  channel count:" << channelCount)
    RLP_LOG_DEBUG("  pixelSize: " << pixelSize)
    RLP_LOG_DEBUG("  byteCount: " << byteCount)


    RLP_LOG_DEBUG("found" << numFiles << " files")


    // TODO CBB: proper sequence parsing
    // properties.set("frameCount", numFiles);

    properties.insert("video.frame_count", numFiles);
    properties.insert("video.start_frame", 1);
    properties.insert("video.end_frame", 1 + numFiles);
    properties.insert("video.frame_rate", frameRate);

    // properties.insert("media.name", filePath.baseName());
    properties.insert("media.frame_count", numFiles);

    // NOTE:
    // info for one frame only - assuming for now all the frames are identical
    // TODO: identify up front if the frames are not identical and store
    // info forr each frame
    //

    QVariantMap frameInfo;

    frameInfo.insert("width", width);
    frameInfo.insert("height", height);
    frameInfo.insert("pixelSize", pixelSize);
    frameInfo.insert("channelCount", channelCount); // TODO FIXME: what should this be? is this used?
    frameInfo.insert("byteCount", byteCount);



    QVariantList frameInfoList;
    frameInfoList.push_back(frameInfo);
    properties.insert("video.frame_info", frameInfoList);
    properties.insert("video.frame_info.one", frameInfo);

    properties.insert("video.format.type", MEDIA_Plugin::VIDEO_FRAMES);
    properties.insert("video.format.reader", name());

    // properties.insert("video.colourspace", "linear");


    properties.insert("media_type", MEDIA_Plugin::VIDEO_FRAMES);
    result.push_back(properties);

    return result;
}


DS_NodePtr
RLQPSD_MEDIA_Plugin::createVideoReader(QVariantMap* properties)
{
    RLP_LOG_DEBUG("")

    DS_NodePtr node(new RLQPSD_MEDIA_VideoReader(properties));

    return node;
}


//


RLQPSD_MEDIA_ReadTask::RLQPSD_MEDIA_ReadTask(RLQPSD_MEDIA_VideoReader* reader,
                                           DS_TimePtr timeInfo,
                                           DS_FrameBufferPtr destFrame) :
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLQPSD_MEDIA_ReadTask::run()
{
    // std::cout << "RLQPSD_MEDIA_ReadTask::run()" << std::endl;
    //
    //if ((_reader == nullptr) || (_reader == NULL)) {
    //    return;
    //}

    _reader->readVideo(_timeInfo, _destFrame);
}
