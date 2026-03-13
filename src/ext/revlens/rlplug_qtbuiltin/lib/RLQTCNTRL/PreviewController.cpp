

#include "RlpExtrevlens/RLQTCNTRL/PreviewController.h"

#include "RlpRevlens/VIDEO/ThreadedClock.h"

#include "RlpRevlens/MEDIA/Factory.h"


RLP_SETUP_LOGGER(extrevlens, RLQTCNTRL, IdentifyPathTask)
RLP_SETUP_LOGGER(extrevlens, RLQTCNTRL, PreviewVideo)
RLP_SETUP_LOGGER(extrevlens, RLQTCNTRL, PreviewController)


RLQTCNTRL_IdentifyPathTask::RLQTCNTRL_IdentifyPathTask(
    RLQTCNTRL_PreviewController* cntrl, QString path
):
    _cntrl(cntrl),
    _path(path)
{
}


void
RLQTCNTRL_IdentifyPathTask::run()
{
    // RLP_LOG_DEBUG(_path)

    MEDIA_Factory* factory = MEDIA_Factory::instance();

    QVariantMap mediaInput;
    mediaInput.insert("media.video", _path);

    QVariantMap result = factory->identifyMedia2(mediaInput); //  DS_Node::DATA_VIDEO);

    QVariantMap* rr = new QVariantMap();
    rr->insert(result);

    factory->insertInitialMediaProperties(rr);

    DS_NodePtr node = nullptr;
    node = factory->createVideoReader(rr);

    if (node)
    {
        QVariantMap info;
        QVariant v;
        v.setValue(node);
        info.insert("node", v);

        _cntrl->emitNoticeEvent("identify_ready", info);
    }
}


RLQTCNTRL_PreviewVideo::RLQTCNTRL_PreviewVideo(
    DS_ControllerBase* cntrl
):
    VIDEO_SyncTarget(),
    _controller(cntrl),
    _playbackMode(new DS_PlaybackMode()),
    _cache(new VIDEO_LookaheadCache()),
    _display(nullptr)
{
    _avclock = new VIDEO_ThreadedClock(this, 24);
    _cache->setController(cntrl);
    _cache->setPlaybackMode(_playbackMode);

    initFbuf();
}


void
RLQTCNTRL_PreviewVideo::initFbuf()
{
    RLP_LOG_DEBUG("")

    // TODO FIXME: FIX FOR CRASH AT STARTUP ON WINDOWS
    // initializeGL() on windows at startup time requires
    // glTexImage2D to be initialized with a nonzero size buffer

    int defaultChannelCount = 4;
    int defaultPixelTypeDepth = 1; // sizeof(half);
    int defaultPixelSize = defaultChannelCount * defaultPixelTypeDepth;

    QVariantMap frameInfo;
    frameInfo.insert("width", 853);
    frameInfo.insert("height", 480);
    frameInfo.insert("channelCount", defaultChannelCount);
    frameInfo.insert("pixelSize", defaultPixelTypeDepth);
    
    _fbuf = std::shared_ptr<DS_FrameBuffer>(new DS_FrameBuffer(this, frameInfo));
    _fbuf->reallocate();
    _fbuf->setFrameNum(0);
}


void
RLQTCNTRL_PreviewVideo::setDisplay(RLQTDISP_PreviewViewBase* display)
{
    RLP_LOG_DEBUG(display)

    _display = display;
}


void
RLQTCNTRL_PreviewVideo::displayNextFrame()
{
    // RLP_LOG_DEBUG(_currentFrame);

    if (_display == nullptr)
    {
        RLP_LOG_ERROR("No display")
        return;
    }


    _fbuf->setFrameNum(_currentFrame);
    bool result = _cache->present(_currentFrame, _fbuf);

    _display->setImage(_fbuf->toQImage());
    _display->update();


    std::pair<qlonglong, DS_PlaybackMode::DirectionMode> nextFrameResult = _playbackMode->getNextFrameNum(
        _currentFrame,
        DS_PlaybackMode::FORWARD,
        DS_PlaybackMode::INC
    );

    _currentFrame = nextFrameResult.first;
}


void
RLQTCNTRL_PreviewVideo::resume()
{
    RLP_LOG_DEBUG("")

    _avclock->resume();
}


void
RLQTCNTRL_PreviewVideo::stop()
{
    RLP_LOG_DEBUG("")

    _avclock->stop();
}


// -----

RLQTCNTRL_PreviewController::RLQTCNTRL_PreviewController()
{
    _session = DS_SessionPtr(new DS_Session("preview"));
    _session->addTrackNoSync("Preview");

    _videoManager = new RLQTCNTRL_PreviewVideo(this);

    connect(
        this,
        &RLQTCNTRL_PreviewController::noticeEvent,
        this,
        &RLQTCNTRL_PreviewController::onNoticeEvent
    );
}


void
RLQTCNTRL_PreviewController::play()
{
    RLP_LOG_DEBUG("")

    _playbackState = RLQTCNTRL_PreviewController::PLAYSTATE_PLAYING;
    _videoManager->resume();
}


void
RLQTCNTRL_PreviewController::pause()
{
    _playbackState = RLQTCNTRL_PreviewController::PLAYSTATE_PAUSED;
    _videoManager->stop();
}


void
RLQTCNTRL_PreviewController::loadMediaItem(QString path)
{
    RLP_LOG_DEBUG(path)

    _videoManager->cache()->expireAll();
    _videoManager->gotoFrame(1);

    RLQTCNTRL_IdentifyPathTask* task = new RLQTCNTRL_IdentifyPathTask(this, path);
    QThreadPool::globalInstance()->start(task);
}


void
RLQTCNTRL_PreviewController::onNoticeEvent(QString noticeName, QVariantMap noticeInfo)
{
    RLP_LOG_DEBUG(noticeName)

    if (noticeName == "identify_ready")
    {
        DS_NodePtr node = noticeInfo.value("node").value<DS_NodePtr>();

        if (node != nullptr)
        {
            RLP_LOG_DEBUG("Load OK")

            DS_TrackPtr track = _session->getTrackByIndex(0);
            track->clear();
            track->insertNode(node, 1);

            _session->setFrameCount(track->getMaxFrameNum());
            _videoManager->playbackMode()->setFrameRange(1, _session->frameCount());

            _videoManager->cache()->recenter(1);


            QTimer::singleShot(500, this, &RLQTCNTRL_PreviewController::play);
            // _cntrl->play();
        }
    }
}
