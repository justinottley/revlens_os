
#include "RlpProd/CNTRL/StreamingController.h"
#include "RlpProd/CNTRL/LoadStreamTask.h"

#include "RlpRevlens/MEDIA/Factory.h"


RLP_SETUP_LOGGER(ext, CNTRL, StreamingController)

CNTRL_StreamingController* CNTRL_StreamingController::_instance = nullptr;


// -----------------


CNTRL_StreamingController::CNTRL_StreamingController():
    DS_ControllerBase(),
    _streamUrl(""),
    _drawFbuf(nullptr)
{
    _playbackState = DS_ControllerBase::PLAYSTATE_UNKNOWN;

    connect(
        this,
        &CNTRL_StreamingController::streamReady,
        this,
        &CNTRL_StreamingController::startStream
    );

    // Forward noticeEvent to Main Controller
    connect(
        this,
        &CNTRL_StreamingController::noticeEvent,
        CNTRL_MainController::instance(),
        &CNTRL_MainController::noticeEvent
    );

    _fbufFrameInfo.clear();
    _fbufFrameInfo.insert("draw.frame", 1);

    _session = DS_SessionPtr(new DS_Session("stream"));
    _session->addTrackNoSync("Stream");


    float frameRate = 18.0;
    int frameCount = 100;

    _videoManager = new CNTRL_Video(this, "Stream"); // , QThread::NormalPriority);
    _videoManager->setTargetFrameRate(frameRate);
    _videoManager->onFrameCountChanged(frameCount);

    _drawController = new RLANN_CNTRL_DrawController(/* setSingleton = */ false);
    _drawController->setMainController(this);
    _drawController->setCurrFrame(1);

    DS_TrackPtr annTrack = _session->addTrackByType("Annotation");
    _drawController->setActiveTrack(annTrack->uuid().toString());

    if (CNTRL_StreamingController::_instance == nullptr)
    {
        CNTRL_StreamingController::_instance = this;
    }
}


CNTRL_StreamingController*
CNTRL_StreamingController::instance()
{
    if (CNTRL_StreamingController::_instance == nullptr)
    {
        CNTRL_StreamingController* c = new CNTRL_StreamingController();

        RLP_LOG_DEBUG("Creating new Streaming Controller")
    }

    return CNTRL_StreamingController::_instance;
}


DS_EventPlugin*
CNTRL_StreamingController::initAnnotationDisplayPlugin()
{
    RLP_LOG_DEBUG("")

    DS_EventPlugin* dp = _drawController->duplicate();
    dp->setName("RlpAnnotationDisplayStreaming");
    dp->setEnabled(false);

    return dp;
}


void
CNTRL_StreamingController::loadStream(QString streamUrl, QVariantMap streamInfo)
{
    RLP_LOG_DEBUG(streamUrl)

    if (streamUrl == "")
    {
        RLP_LOG_ERROR("Invalid stream url")
        return;
    }

    stopStream();

    _streamUrl = streamUrl;
    _streamInfo = streamInfo;

    RLANN_DS_Track* atrack = qobject_cast<RLANN_DS_Track*>(_drawController->activeTrack().get());
    QString annTrackUuidStr = atrack->uuid().toString();
    atrack->clear();

    int fwidth = streamInfo.value("w").toInt();
    int fheight = streamInfo.value("h").toInt();

    _drawFbuf = DS_FrameBufferPtr(new RLANN_DS_FrameBuffer(this, _fbufFrameInfo, _drawController));

    RLANN_DS_Annotation* ann = _drawController->initAnnotation(1, annTrackUuidStr, fwidth, fheight);
    // _drawController->holdFrame("", annTrackUuidStr, 1, 100, /* emitSyncAction = */ false);

    CNTRL_LoadStreamTask* load = new CNTRL_LoadStreamTask(this, streamUrl, streamInfo);
    QThreadPool::globalInstance()->start(load);
}


void
CNTRL_StreamingController::startStream()
{
    emitNoticeEvent("video.stream_started", _streamInfo);

    RLP_LOG_DEBUG("")
    qlonglong frameByteCount = _streamInfo.value("w").toInt() * _streamInfo.value("h").toInt() * 4; // 960 * 540 * 4;

    _videoManager->cache()->setMaxSize((frameByteCount * 6) + 1024);
    _playbackState = DS_ControllerBase::PLAYSTATE_PLAYING;

    emit playStateChanged(_playbackState);

    _videoManager->updateToFrame(1, /*recenter=*/true);
    _videoManager->resume();
}


void
CNTRL_StreamingController::stopStream()
{
    RLP_LOG_DEBUG("")

    _playbackState = DS_ControllerBase::PLAYSTATE_PAUSED;
    emit playStateChanged(_playbackState);

    _videoManager->stop();
}


void
CNTRL_StreamingController::clearAnnotations()
{
    RLP_LOG_DEBUG("")

    RLANN_DS_Track* atrack = qobject_cast<RLANN_DS_Track*>(_drawController->activeTrack().get());
    atrack->clearAnnotationOnFrame(1);
    atrack->clear();

    int fwidth = _streamInfo.value("w").toInt();
    int fheight = _streamInfo.value("h").toInt();

    _drawController->initAnnotation(1, atrack->uuid().toString(), fwidth, fheight);
}


void
CNTRL_StreamingController::onMediaNoticeEvent(QString evtName, QVariantMap evtInfo)
{
    if (evtName == "stream_broken")
    {
        QVariantMap evtInfo;
        CNTRL_MainController::instance()->emitNoticeEvent("video.stream_stopped", evtInfo);
    }
}