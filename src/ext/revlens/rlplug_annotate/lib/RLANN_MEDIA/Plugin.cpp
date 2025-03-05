//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLANN_DS/FrameBuffer.h"
#include "RlpExtrevlens/RLANN_MEDIA/Plugin.h"
#include "RlpExtrevlens/RLANN_MEDIA/ImageNode.h"

RLP_SETUP_EXT_LOGGER(RLANN_MEDIA, Plugin)


RLANN_MEDIA_Plugin::RLANN_MEDIA_Plugin():
    MEDIA_Plugin("Annotation"),
    _drawController(nullptr)
{
    QThreadPool* videoPool = new QThreadPool();
    videoPool->setMaxThreadCount(1);

    _readThreadPoolMap.insert(DS_Node::DATA_VIDEO, videoPool);
}


RLANN_MEDIA_Plugin::RLANN_MEDIA_Plugin(RLANN_DS_DrawControllerBase* drawController):
    MEDIA_Plugin("Annotation"),
    _drawController(drawController)
{
}


void
RLANN_MEDIA_Plugin::setDrawController(RLANN_DS_DrawControllerBase* controller)
{
    RLP_LOG_DEBUG(controller)
    _drawController = controller;
}


DS_FrameBufferPtr
RLANN_MEDIA_Plugin::createFrameBuffer(QObject* owner, QVariantMap frameInfo)
{
    std::cout << "RLANN_MEDIA_Plugin::createFrameBuffer()" << std::endl;
    
    return DS_FrameBufferPtr(new RLANN_DS_FrameBuffer(owner, frameInfo, _drawController));
}


// ------------------------


QVariantList
RLANN_MEDIA_Plugin::identifyMedia(QVariantMap mediaInput, DS_Node::NodeDataType dataType)
{
    RLP_LOG_DEBUG("")

    QVariantMap properties;
    QVariantList result;

    properties.clear();
    result.clear();
    
    if (!mediaInput.contains("media.video")) {
        
        // NOTE: early return
        //
        return result;
    }

    QString videoPath = mediaInput.value("media.video").value<QString>();  // value<QString>("video_dir");
    RLP_LOG_DEBUG(videoPath);

    if (videoPath.startsWith("@image:"))
    {
        RLP_LOG_DEBUG("FIXED COLOR BACKGROUND FOUND")
        QString bgInfo = videoPath.section(':', 1).trimmed();
        QJsonDocument resultData = QJsonDocument::fromJson(
            QByteArray::fromBase64(QByteArray(bgInfo.toLatin1())));
        QVariantMap data = resultData.object().toVariantMap();

        RLP_LOG_DEBUG(data)

        properties.insert("video.source", data.value("type"));
        properties.insert("video.colour", data.value("colour"));

        int rx = data.value("resolution.x").toInt();
        int ry = data.value("resolution.y").toInt();
        int byteCount = rx * ry * 4;

        QVariantMap frameInfo;
        frameInfo.insert("width", rx);
        frameInfo.insert("height", ry);
        frameInfo.insert("pixelSize", 1);
        frameInfo.insert("channelCount", 4);
        frameInfo.insert("byteCount", byteCount);

        QVariantList flist;
        flist.push_back(frameInfo);

        properties.insert("video.frame_info", flist);
        properties.insert("video.frame_info.one", frameInfo);


        properties.insert("video.format.type", MEDIA_Plugin::VIDEO_FRAMES);
        properties.insert("video.format.reader", name());

        // required api
        //
        properties.insert("media_type", MEDIA_Plugin::VIDEO_FRAMES);
        properties.insert("media.frame_count", 100);
        properties.insert("video.frame_rate", 24);

        result.push_back(properties);

        RLP_LOG_DEBUG("OK")
    }


    return result;
}


DS_NodePtr
RLANN_MEDIA_Plugin::createVideoReader(QVariantMap* properties)
{
    RLP_LOG_DEBUG("")

    DS_NodePtr node(new RLANN_MEDIA_ImageNode(properties));
    return node;
}
