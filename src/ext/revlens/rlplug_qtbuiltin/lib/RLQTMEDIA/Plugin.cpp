//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include <iostream>

#include "RlpCore/SEQ/Util.h"
#include "RlpExtrevlens/RLQTMEDIA/Plugin.h"

#include "RlpExtrevlens/RLQTMEDIA/VideoReader.h"
#include "RlpExtrevlens/RLQTMEDIA/NetworkReadHandler.h"

RLP_SETUP_EXT_LOGGER(RLQTMEDIA, Plugin)

RLQTMEDIA_Plugin::RLQTMEDIA_Plugin() :
    MEDIA_Plugin(QString("QtMedia"))
{
    QThreadPool* videoPool = new QThreadPool();
    videoPool->setExpiryTimeout(-1); // dont expire threads
    videoPool->setMaxThreadCount(4);

    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool);

    // Init and connect to remote frame server
    QString batch = getenv("RLP_BATCH");
    if (batch != "1")
    {
        RLQTMEDIA_NetworkPool::instance();
    }
    
}


// MEDIA_Plugin::MediaType
QVariantList
RLQTMEDIA_Plugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    RLP_LOG_DEBUG("")

    QVariantMap properties;
    QVariantList result;

    properties.clear();
    result.clear();

    if (!mediaInput.contains("media.video"))
    {

        // NOTE: early return
        //
        return result;
    }

    // only attempt to open frame sequence type component
    if (!mediaInput.contains("media.video.resolved_component"))
    {
        RLP_LOG_DEBUG("resolved_component not found, aborting")
        return result;
    }

    QString resolvedComponentType = mediaInput.value("media.video.resolved_component").toString();
    if (resolvedComponentType != "frames")
    {
        RLP_LOG_DEBUG("frame component not found, aborting")
        return result;
    }

    QString videoPath = mediaInput.value("media.video").toString();
    RLP_LOG_DEBUG(videoPath);

    SEQ_SequencePtr seq = SEQ_Util::getSequenceFromTemplateString(videoPath);
    if (seq == nullptr) {
        RLP_LOG_DEBUG("no sequences found");
        return result;
    }


    int numFrames = seq->size();

    RLP_LOG_DEBUG("sequence size: " << numFrames);


    if (numFrames > 0)
    {

        properties.insert("video.sequence", videoPath);

        properties.insert("video.start_frame", 1);
        properties.insert("video.end_frame", numFrames);


        QFileInfo firstFile = seq->at(0)->fileInfo();


        // Media name
        //
        QString mediaName = seq->itemTemplate()->prefix() + " " + seq->frange();

        properties.insert("media.name", firstFile.baseName());


        QString filePath = firstFile.absoluteFilePath();

        RLP_LOG_DEBUG("identifying " << filePath);

        QImageReader reader(filePath);
        QImage::Format fmt = reader.imageFormat();


        RLP_LOG_DEBUG("found " << numFrames << " files");


        properties.insert("video.frame_count", numFrames);
        properties.insert("media.frame_count", numFrames);



        if (fmt != QImage::Format_Invalid)
        {

            // info for one frame only - assuming for now all the frames are identical
            // TODO: identify up front if the frames are not identical and store
            // info for each frame
            //

            QImage image(filePath);

            QVariantMap frameInfo;

            frameInfo.insert("width", image.width());
            frameInfo.insert("height", image.height());
            frameInfo.insert("pixelSize", 1); // (image.depth() / 8));
            frameInfo.insert("channelCount", 4); // image.depth() / 8);
            frameInfo.insert("byteCount", image.sizeInBytes());
            frameInfo.insert("hasAlpha", image.hasAlphaChannel());

            RLP_LOG_DEBUG("  width: " << image.width());
            RLP_LOG_DEBUG("  height: " << image.height());
            RLP_LOG_DEBUG("  depth: " << image.depth());
            RLP_LOG_DEBUG("  bitPlaneCount: " << image.bitPlaneCount());
            RLP_LOG_DEBUG("  bitsPerPixel:" << image.pixelFormat().bitsPerPixel());
            int srcChannelCount = image.bitPlaneCount() / image.pixelFormat().bitsPerPixel();

            RLP_LOG_DEBUG("  has alpha:" << image.hasAlphaChannel())

            QVariantList frameInfoList;
            frameInfoList.push_back(frameInfo);
            properties.insert("video.frame_info", frameInfoList);
            properties.insert("video.frame_info.one", frameInfo);

            RLP_LOG_DEBUG("Identified VIDEO")


            properties.insert("video.format.type", MEDIA_Plugin::VIDEO_FRAMES);
            properties.insert("video.format.reader", name());
            properties.insert("video.colourspace", "sRGB");

            float frameRate = 24;
            if (mediaInput.contains("media.frame_rate"))
            {
                float frameRate = mediaInput.value("media.frame_rate").value<float>();  // value<QString>("video_dir");
            }

            properties.insert("video.frame_rate", frameRate);

            properties.insert("media_type", MEDIA_Plugin::VIDEO_FRAMES);

            result.push_back(properties);

        }

    }

    return result;

}


DS_NodePtr
RLQTMEDIA_Plugin::createVideoReader(QVariantMap* properties)
{
    RLP_LOG_DEBUG("");

    DS_NodePtr node(new RLQTMEDIA_VideoReader(properties));
    return node;
}


//


RLQTMEDIA_ReadTask::RLQTMEDIA_ReadTask(RLQTMEDIA_VideoReader* reader,
                                   DS_TimePtr timeInfo,
                                   DS_BufferPtr destFrame) :
    _reader(reader),
    _timeInfo(timeInfo),
    _destFrame(destFrame)
{
}


void
RLQTMEDIA_ReadTask::run()
{
    // RLP_LOG_DEBUG("RLQTMEDIA_ReadTask::run()");

    _reader->readVideo(_timeInfo, _destFrame);
}
