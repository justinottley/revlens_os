
#include "RlpProd/CNTRL/LoadStreamTask.h"
#include "RlpProd/CNTRL/StreamingController.h"

#include "RlpRevlens/MEDIA/Factory.h"


RLP_SETUP_LOGGER(ext, CNTRL, LoadStreamTask)

void
CNTRL_LoadStreamTask::setupMediaInfo(QVariantMap* minfo)
{
    // Setup all media info for faster startup (skip identify)
    //

    int frameWidth = _streamInfo.value("w").toInt(); // 1920;
    int frameHeight = _streamInfo.value("h").toInt(); // 1080;
    int channelCount = 4;

    qlonglong frameByteCount = frameWidth * frameHeight * channelCount; // 960 * 540 * 4;
    float frameRate = 18.0;
    int frameCount = 100;

    QVariantMap frameInfo;
    frameInfo.insert("bitsPerPixel", 32);
    frameInfo.insert("bytePadding", 64);
    frameInfo.insert("pixelSize", 1);
    frameInfo.insert("width", frameWidth);
    frameInfo.insert("height", frameHeight);
    frameInfo.insert("channelCount", channelCount);
    frameInfo.insert("byteCount", frameByteCount);

    minfo->insert("media.frame_count", frameCount);
    minfo->insert("media_type", 1);
    minfo->insert("video.codec", "h264");
    minfo->insert("video.codec_long", "H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10");
    minfo->insert("vode.colourspace", "sRGB");
    minfo->insert("video.format.reader", "FFmpeg");
    minfo->insert("video.format.type", 1);
    minfo->insert("video.frame_rate", frameRate);
    minfo->insert("video.frame_count", frameCount); // doesn't matter
    minfo->insert("video.start_time", 0);
    minfo->insert("video.streaming", 1);

    minfo->insert("video.frame_info.one", frameInfo);
    QVariantList frameList;
    frameList.push_back(frameInfo);
    minfo->insert("video.frame_info", frameList);
}


void
CNTRL_LoadStreamTask::run()
{
    _cntrl->session()->getTrackByIndex(0)->clear();

    MEDIA_Factory* factory = MEDIA_Factory::instance();
    MEDIA_Plugin* ffplug = factory->getMediaPluginByName("FFmpeg");

    QVariantMap* mediaStreamInfo = new QVariantMap();
    factory->insertInitialMediaProperties(mediaStreamInfo);
    // mediaStream.insert("media.video", "rtsp://jasmine19a:8554/live.stream");
    // mediaStream.insert("media.video", "rtmp://jasmine19a:1935/live.stream");

    setupMediaInfo(mediaStreamInfo);
    mediaStreamInfo->insert("media.video", _streamUrl);


    // LOG_Logging::pprint(mediaStream);

    // Skip identify for speed of startup
    //
    // QVariantList streamList = ffplug->identifyMedia(mediaStream, DS_Node::DATA_VIDEO);
    // if (streamList.size() == 1)

    DS_FrameBufferPtr fbuf = _cntrl->drawFbuf();

    DS_NodePtr node = ffplug->createVideoReader(mediaStreamInfo);

    QObject::connect(
        node->graph(),
        &DS_Graph::noticeEvent,
        _cntrl,
        &CNTRL_StreamingController::onMediaNoticeEvent
    );

    // DS_NodePtr annNode = DS_NodePtr(
    //     new RLANN_MEDIA_StreamingNode(_cntrl->drawController(), node->getPropertiesPtr(), node));

    QVariant vw;
    vw.setValue(ffplug);

    QVariant fw;
    fw.setValue(fbuf);

    node->setProperty("video.plugin", vw);
    node->setProperty("extra.fbuf", fw);

    _cntrl->session()->getTrackByIndex(0)->insertNode(node, 1);
    _cntrl->emitStreamReady();
}

