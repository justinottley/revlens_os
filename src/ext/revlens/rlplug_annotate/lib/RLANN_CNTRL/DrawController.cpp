//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLANN_CNTRL/DrawController.h"

#include "RlpCore/PY/Interp.h"


#include "RlpRevlens/VIDEO/LookaheadCache.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/PluginManager.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI_TL2/Controller.h"

#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"
#include "RlpExtrevlens/RLANN_CNTRL/TrackFactoryPlugin.h"
#include "RlpExtrevlens/RLANN_DS/AnnotationTask.h"
#include "RlpExtrevlens/RLANN_DS/AnnotationEvent.h"

#include "RlpExtrevlens/RLANN_MP/Handler.h"

#include "RlpExtrevlens/RLANN_CNTRL/FrameProbe.h"
#include "RlpExtrevlens/RLANN_CNTRL/DrawCommands.h"

RLP_SETUP_EXT_LOGGER(RLANN_CNTRL, DrawController)
RLP_SETUP_EXT_LOGGER(RLANN_CNTRL, DisplayPlugin)

int RLANN_CNTRL_DrawController::UPDATE_TIMER_INTERVAL = 2500;

RLANN_CNTRL_DrawController* RLANN_CNTRL_DrawController::_instance;

RLANN_CNTRL_DrawController::RLANN_CNTRL_DrawController():
    RLANN_DS_DrawControllerBase(),
    _drawing(false),
    _paperTraceMode(RLANN_CNTRL_DrawController::PAPER_TRACE_ALWAYS_ON),
    _doPaperTrace(false),
    _currFrame(-1),
    _doSyncAnnotations(true),
    _brushEngine("Basic")
{
    _tpool = new QThreadPool();
    _tpool->setMaxThreadCount(1);
    
    _updateSet.clear();
    connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(onTimerMediaUpdate()));

    // TODO FIXME: MAY NOT BE REQUIRED
    //
    _frameProbe = new RLANN_CNTRL_FrameProbe(this);
    
    _trackFactory = new RLANN_CNTRL_TrackFactoryPlugin();

    RLANN_CNTRL_DrawController::setInstance(this);

    RLANN_MP_Handler* mph = RLANN_MP_Handler::instance();

    _paperTraceFbuf = RLANN_DS_QImageFrameBufferPtr(
        new RLANN_DS_QImageFrameBuffer(this, 0, 0));

    _remoteUserMap = new StringPointMap();

    initPreferences();

    RLP_LOG_DEBUG("initialized");
}


void
RLANN_CNTRL_DrawController::initPreferences()
{
    RLP_LOG_DEBUG("")

    setProperty("pref.show_remote_cursors", false);
}


RLANN_MP_Handler*
RLANN_CNTRL_DrawController::mypaintHandler()
{
    return RLANN_MP_Handler::instance();
}


DS_EventPlugin*
RLANN_CNTRL_DrawController::duplicate()
{
    // RLP_LOG_DEBUG("")

    RLANN_CNTRL_DisplayPlugin* dp = new RLANN_CNTRL_DisplayPlugin(this);
    dp->setMainController(_controller);

    _childPlugins.append(dp);
    return dp;
}


bool
RLANN_CNTRL_DrawController::setMainController(CNTRL_MainController* controller)
{
    DS_EventPlugin::setMainController(controller);

    controller->getPluginManager()->registerVideoProbe(_frameProbe);

    controller->session()->registerTrackFactoryPlugin(
        "Annotation",
        _trackFactory
    );


    connect(
        controller->session().get(),
        SIGNAL(trackDataChanged(QString, QVariantMap)),
        this,
        SLOT(onTrackDataChanged(QString, QVariantMap))
    );

    
    connect(
        controller,
        SIGNAL(playStateChanged(int)),
        this,
        SLOT(onPlayStateChanged(int))
    );

    connect(
        controller->getVideoManager()->cache(),
        SIGNAL(lookaheadRelease(qlonglong)),
        this,
        SLOT(onLookaheadRelease(qlonglong))
    );

    connect(
        controller->getVideoManager(),
        SIGNAL(displayDeregistered(DISP_GLView*)),
        this,
        SLOT(onDisplayDeregistered(DISP_GLView*))
    );

    connect(
        this,
        SIGNAL(activeTrackChanged(QString)),
        GUI_TL2_Controller::instance(),
        SLOT(onActiveTrackChanged(QString))
    );

    controller->registerCommand(new RLANN_CNTRL_AnnotateCommand("Draw", controller));

    return true;
}




void
RLANN_CNTRL_DrawController::setActiveTrack(QString uuidStr)
{
    RLP_LOG_DEBUG(uuidStr);

    _activeTrackUuidStr = uuidStr;
    setupFrame(_currFrame, uuidStr);

    emit activeTrackChanged(uuidStr);
}


/*
AnnotationFrameMap*
RLANN_CNTRL_DrawController::getAnnotationMap(int idx)
{
    
    if (idx == -1) {
        idx = _activeTrackIdx;
    }
    
    DS_TrackPtr track = _controller->session()->getTrack(idx);
    
    if (track != nullptr) {
        RLANN_DS_Track* annTrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        return annTrack->getAnnotationMap();
        
    } else {
        
        RLP_LOG_DEBUG("RLANN_CNTRL_DrawController::getAnnotationMap(): WARNING: returning nullptr");
        return nullptr;
    }
}
*/


void
RLANN_CNTRL_DrawController::setSyncAnnotations(bool doSync)
{
    #ifdef SCAFFOLD_IOS
    return;
    #endif

    RLP_LOG_DEBUG(doSync);

    _doSyncAnnotations = doSync;
    if (_doSyncAnnotations) {
        _updateTimer.start(RLANN_CNTRL_DrawController::UPDATE_TIMER_INTERVAL);
    }
}


DS_TrackPtr
RLANN_CNTRL_DrawController::activeTrack()
{
    DS_SessionPtr session = _controller->session();
    if (session != nullptr) {
        return _controller->session()->getTrackByUuid(_activeTrackUuidStr);
    }
    
    return nullptr;
}


void
RLANN_CNTRL_DrawController::setupFrame(qlonglong frameNum, QString trackUuidStr, QString annUuidStr)
{
    /*
    if (_controller->getPlaybackState() != CNTRL_MainController::PAUSED) {
        return;
    }
    */
    
    // RLP_LOG_DEBUG(frameNum << " " << trackUuidStr << " " << annUuidStr);
    
    _currFrame = frameNum;
    
    // AnnotationFrameMap* annmap = getAnnotationMap();
    
    DS_TrackPtr atrack = nullptr;
    if (trackUuidStr == "") {
        atrack = activeTrack();

    } else {
        atrack = _controller->session()->getTrackByUuid(trackUuidStr);
    }

    if (atrack != nullptr) {

        RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(atrack.get());

        if (!anntrack->hasAnnotation(frameNum)) {

            // RLP_LOG_DEBUG("no annotation on frame " << frameNum << " - initializing");

            DS_NodePtr node = _controller->session()->getNodeByFrame(frameNum);
            if (node == nullptr) {
                return;
                
            } else {
            
            
                QVariantMap frameInfo = node->getProperty<QVariantMap>("video.frame_info.one");
                int frameWidth = frameInfo.value("width").value<int>();
                int frameHeight = frameInfo.value("height").value<int>();
                
                // RLP_LOG_DEBUG("resolution: " << frameWidth << " " << frameHeight);
                
                RLANN_DS_AnnotationPtr ann = RLANN_DS_AnnotationPtr(
                    new RLANN_DS_Annotation(frameWidth, frameHeight, annUuidStr));
                
                connect(
                    ann.get(),
                    &RLANN_DS_Annotation::annotationImageChanged,
                    this,
                    &RLANN_CNTRL_DrawController::onAnnotationImageChanged
                );

                ann->reallocate();
                ann->setFrameRange(frameNum, frameNum);

                anntrack->brush()->setupFrame(ann);

                updateAnnDisplayWindow(ann);

                // NOTE: constructor? is this a bad idea?
                // saving reference to track for track specific info (cachedir, etc)
                ann->setTrack(atrack);

                anntrack->insertAnnotation(frameNum, ann);
            }

        } else
        {

            RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(frameNum);
            if ((annUuidStr != "") && (!ann->isValid()) && (ann->uuid().toString() != annUuidStr))
            {
                RLP_LOG_WARN("Remaping annotation UUID to " << annUuidStr);

                QUuid destUuid(annUuidStr);
                anntrack->updateAnnotationUuid(ann->uuid(), destUuid);

                ann->setUuid(annUuidStr);

                anntrack->brush()->setupFrame(ann);

                updateAnnDisplayWindow(ann);



            }
            else
            {
                anntrack->brush()->setupFrame(ann);

                updateAnnDisplayWindow(ann);

            }

        }
    } else {
        RLP_LOG_WARN("NO ACTIVE TRACK")
    }
}


RLANN_DS_Annotation*
RLANN_CNTRL_DrawController::initAnnotation(qlonglong frameNum, QString trackUuidStr, int frameWidth, int frameHeight)
{
    RLP_LOG_DEBUG(trackUuidStr << frameNum << frameWidth << frameHeight)

    DS_TrackPtr atrack = _controller->session()->getTrackByUuid(trackUuidStr);

    if (atrack == nullptr)
    {
        RLP_LOG_ERROR("Invalid track from" << trackUuidStr)
        return nullptr;
    }

    RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(atrack.get());

    RLP_LOG_DEBUG(anntrack)

    RLANN_DS_AnnotationPtr ann = RLANN_DS_AnnotationPtr(
        new RLANN_DS_Annotation(frameWidth, frameHeight));
    
    ann->reallocate();
    ann->setFrameRange(frameNum, frameNum);

    anntrack->brush()->setupFrame(ann);

    updateAnnDisplayWindow(ann);

    // NOTE: constructor? is this a bad idea?
    // saving reference to track for track specific info (cachedir, etc)
    ann->setTrack(atrack);

    anntrack->insertAnnotation(frameNum, ann);

    emit syncDrawStart(frameNum, trackUuidStr);

    return ann.get();
}


void
RLANN_CNTRL_DrawController::updateAnnDisplayWindow(RLANN_DS_AnnotationPtr ann)
{
    // RLP_LOG_DEBUG("")

    for (int ci=0; ci != _childPlugins.size(); ++ci)
    {
        qobject_cast<RLANN_CNTRL_DisplayPlugin*>(_childPlugins.at(ci))->updateAnnDisplayWindow(ann);
    }
}


void
RLANN_CNTRL_DrawController::holdFrame(QString viewUuidStr, QString trackUuidStr, qlonglong srcFrame, qlonglong destFrame, bool emitSyncAction)
{
    // RLP_LOG_DEBUG("View:" << viewUuidStr << srcFrame << " -> " << destFrame);
    
    DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
    if (track != nullptr) {
        RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        atrack->holdFrame(srcFrame, destFrame);

        if (emitSyncAction) {

            RLANN_DS_AnnotationPtr ann = atrack->getAnnotation(srcFrame);

            QVariantList frameList = atrack->getFrameList(ann->uuid());
            
            QVariantMap mkwargs;
            mkwargs.insert("frame", (qlonglong)srcFrame);
            mkwargs.insert("dest_frame", (qlonglong)destFrame);
            mkwargs.insert("track_uuid", atrack->uuid().toString());
            mkwargs.insert("view_uuid", viewUuidStr);
            mkwargs.insert("ann_uuid", ann->uuid().toString());
            mkwargs.insert("frame_list", frameList);

            QVariantMap kwargs;
            kwargs.insert("update_info", mkwargs);
            
            _controller->emitSyncAction("holdFrame", kwargs);
        }
    }
}


void
RLANN_CNTRL_DrawController::releaseFrame(QString viewUuidStr, QString trackUuidStr, qlonglong srcFrame, bool emitSyncAction)
{
    RLP_LOG_DEBUG(srcFrame);
    
    DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
    if (track != nullptr) {
        RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        RLANN_DS_AnnotationPtr ann = atrack->getAnnotation(srcFrame);

        if (ann != nullptr) {
            atrack->releaseFrame(srcFrame);

            if (emitSyncAction) {

                QVariantList annFrameList = atrack->getFrameList(ann->uuid());
                
                QVariantMap mkwargs;
                mkwargs.insert("frame", (qlonglong)srcFrame);
                mkwargs.insert("track_uuid", atrack->uuid().toString());
                mkwargs.insert("ann_uuid", ann->uuid().toString());
                mkwargs.insert("view_uuid", viewUuidStr);
                mkwargs.insert("frame_list", annFrameList);
                
                QVariantMap kwargs;
                kwargs.insert("update_info", mkwargs);

                _controller->emitSyncAction("releaseFrame", kwargs);
            }
            
        }
        
    }
}


bool
RLANN_CNTRL_DrawController::event(QEvent* event)
{
    if (event->type() == RLANN_DS_AnnotationOffloadEventType) {

        // RLP_LOG_INFO("received offload event");

        RLANN_DS_AnnotationOffloadEvent* aoe = dynamic_cast<RLANN_DS_AnnotationOffloadEvent *>(event);
        DS_TrackPtr track = _controller->session()->getTrackByIndex(aoe->trackIdx());
        RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track *>(track.get());
        RLANN_DS_AnnotationPtr ann = atrack->getAnnotationByUuid(aoe->annUuidStr());

        ann->deallocate();

    }

    return RLANN_DS_DrawControllerBase::event(event);
}



bool
RLANN_CNTRL_DrawController::keyPressEventInternal(QKeyEvent* event)
{
    return false;
}


void
RLANN_CNTRL_DrawController::handleSyncAction(QString action, QVariantMap kwargs)
{
    // RLP_LOG_DEBUG(action)

    qlonglong frame = kwargs.value("frame").toLongLong();
    QString trackUuidStr = kwargs.value("track_uuid").toString();

    // _currFrame = currFrame;

    // RLP_LOG_DEBUG(action << " " << frame);

    RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(_controller->session()->getTrackByUuid(trackUuidStr).get());

    if (action == "drawStart") {

        // QString annUuidStr = kwargs.value("ann_uuid").toString();

        // RLP_LOG_DEBUG("Got Annotation UUID: " << annUuidStr);

        QString annUuidStr = kwargs.value("ann_uuid").toString();

        setupFrame(frame, trackUuidStr, annUuidStr);

        RLANN_DS_AnnotationPtr ann = atrack->getAnnotation(frame);
        if (ann != nullptr) {
            ann->setValid();
        }

        float x = kwargs.value("x").toFloat();
        float y = kwargs.value("y").toFloat();
        QPointF lp(x, y);

        QVariantMap brushSettings = kwargs.value("brush").toMap();
        QString brushName = brushSettings.value("name").toString();

        atrack->setBrush(_trackFactory->getBrush(brushName));
        atrack->brush()->setupFrame(ann);
        atrack->brush()->strokeBegin(lp, brushSettings);

        _drawing = true;
        
        emit syncDrawStart(frame, trackUuidStr);
        
    } else if (action == "draw") {

        float x = kwargs.value("x").toFloat();
        float y = kwargs.value("y").toFloat();
        QString user = kwargs.value("user").toString();

        QPointF point(x, y);
        QPoint ipoint(kwargs.value("mx").toInt(), kwargs.value("my").toInt());

        _remoteUserMap->insert(user, ipoint);

        for (int ci=0; ci != _childPlugins.size(); ++ci)
        {
            qobject_cast<RLANN_CNTRL_DisplayPlugin*>(_childPlugins.at(ci))->strokeTo(point, frame, atrack);
        }
        

    } else if (action == "drawEnd") {

        _drawing = false;
        emit syncDrawEnd(frame, trackUuidStr);

        QString annUuidStr = kwargs.value("ann_uuid").toString();
        registerAnnotationForSync(trackUuidStr, annUuidStr);

    } else if (action == "holdFrame") {
        
        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
        if (track != nullptr) {
            
            RLP_LOG_DEBUG("FOUND TRACK");
            qlonglong destFrame = kwargs.value("dest_frame").toLongLong();
            // QList<long> frameList = kwargs.value("frame_list").value<QList<long> >();

            QString annUuidStr = kwargs.value("ann_uuid").toString();

            RLP_LOG_DEBUG("GOT ANNOTATION UUID: " << annUuidStr);

            RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(track.get());
            atrack->holdFrame(frame, destFrame);

            // kwargs.insert("event", "hold_frame");
            // _controller->session()->emitTrackDataChanged(trackUuidStr, kwargs);

            // For GUI or other clients
            //
            emit syncHoldFrame(atrack->uuid().toString(), annUuidStr, frame, destFrame);

        } else {
            RLP_LOG_DEBUG("TRACK NOT FOUND: " << trackUuidStr);
        }
    } else if (action == "releaseFrame") {

        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
        if (track != nullptr) {
            
            qlonglong destFrame = kwargs.value("frame").toLongLong();
            // QList<long> frameList = kwargs.value("frame_list").value<QList<long> >();

            QString annUuidStr = kwargs.value("ann_uuid").toString();

            RLP_LOG_DEBUG("GOT ANNOTATION UUID: " << annUuidStr);
            
            RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(track.get());
            atrack->releaseFrame(destFrame);

            // kwargs.insert("event", "release_frame");
            // _controller->session()->emitTrackDataChanged(trackUuidStr, kwargs);

            // For GUI or other clients
            //
            emit syncReleaseFrame(atrack->uuid().toString(), annUuidStr, destFrame);

        }
    } else if (action == "clearAnnotations") {

        QString func = "rlplug_annotate.cmds.clearAnnotations";
        QVariantList args;
        args.push_back((qlonglong)frame);
        args.push_back(false);
        args.push_back(trackUuidStr);

        PY_Interp::call(func, args);

    } else if (action == "hoverMove") {

        // RLP_LOG_DEBUG(kwargs);

        QString username = kwargs.value("user").toString();
        QPoint pos = QPoint(kwargs.value("x").toInt(), kwargs.value("y").toInt());
        _remoteUserMap->insert(username, pos);

        updateDisplay();

    }
}


void
RLANN_CNTRL_DrawController::onSessionChanged(DS_SessionPtr session)
{
    // RLP_LOG_DEBUG("");
}


void
RLANN_CNTRL_DrawController::onTrackDataChanged(QString eventName, QString trackUuidStr, QVariantMap trackData)
{
    // RLP_LOG_DEBUG(trackData)

    if (eventName == "add_annotation")
    {
        
        // Loading annotations back from a session
        //
        QVariantList frameList = trackData.value("frame_list").value<QVariantList>();

        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
        RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(frameList[0].toLongLong());

        if (ann != nullptr) {
            RLP_LOG_DEBUG("setting window on annotation");

            updateAnnDisplayWindow(ann);
        }
    }
}


void
RLANN_CNTRL_DrawController::onTimerMediaUpdate()
{
    #ifdef SCAFFOLD_IOS
    // Do nothing on iOS
    return;
    #endif

    // RLP_LOG_DEBUG("");
    
    // offload / reload is screwing with this since the annotation data may not be available when this is called
    // return;
    if ((_drawing) || (_controller->getPlaybackState() == CNTRL_MainController::PLAYING)) {

        RLP_LOG_WARN("drawing or playing, skipping");
        return;
    }


    QSet<QPair<QString, QString>>::iterator it;

    for (it = _updateSet.begin(); it != _updateSet.end(); ++it) {

        RLP_LOG_DEBUG(_updateSet.size());

        QPair<QString, QString> trackAnnPair = (*it);
        QString trackUuidStr = trackAnnPair.first;
        QString annUuidStr = trackAnnPair.second;

        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
        RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        RLANN_DS_AnnotationPtr ann = atrack->getAnnotationByUuid(annUuidStr);

        // QString annData = QString(ann->getBase64Data());
        QVariantMap annData = ann->toJson(true).toVariantMap();
        QVariantList frameList = atrack->getFrameList(ann->uuid());

        annData.insert("frame_list", frameList);

        QVariantMap mediaKwargs;
        
        
        mediaKwargs.insert("media_type", "annotation");
        mediaKwargs.insert("track_uuid", trackUuidStr);
        mediaKwargs.insert("ann_data", annData);

        QVariantMap kwargs;
        kwargs.insert("update_info", mediaKwargs);
        /*
        mediaKwargs.insert("data", annData);
        mediaKwargs.insert("frame", (qlonglong)frame);
        mediaKwargs.insert("ann_uuid", ann->uuid());
        mediaKwargs.insert("frame_list", frameList);
        */

        RLP_LOG_DEBUG("emitting media update for track "
                  << trackUuidStr << " "
                  << " annotation: " << ann->uuid().toString()
                 );

        _controller->emitSyncAction("mediaUpdate", kwargs);
    }
    
    _updateSet.clear();
    _updateTimer.stop();
}


void
RLANN_CNTRL_DrawController::onTrackActivated(QString uuidStr)
{
    RLP_LOG_DEBUG(uuidStr);

    setActiveTrack(uuidStr);
}


void
RLANN_CNTRL_DrawController::onPlayStateChanged(int playState)
{
    #ifdef SCAFFOLD_IOS
    return;
    #endif

    // RLP_LOG_DEBUG(playState);

    if (playState == (int)CNTRL_MainController::PAUSED) {
        RLP_LOG_DEBUG("RLANN_CNTRL_DrawController::onPlayStateChanged(): starting update timer");
        _updateTimer.start(RLANN_CNTRL_DrawController::UPDATE_TIMER_INTERVAL);

    } else if (playState == (int)CNTRL_MainController::PLAYING) {
        RLP_LOG_DEBUG("RLANN_CNTRL_DrawController::onPlayStateChanged(): stopping update timer");
        _updateTimer.stop();
    }
}


void
RLANN_CNTRL_DrawController::onLookaheadRelease(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame);
    
}


void
RLANN_CNTRL_DrawController::onDisplayDeregistered(DISP_GLView* display)
{
     int cidx = -1;
    for (int ci=0; ci != _childPlugins.size(); ++ci)
    {
        if (_childPlugins.at(ci)->display() == display)
        {
            cidx = ci;
            break;
        }
    }

    if (cidx >= 0)
    {
        RLP_LOG_DEBUG("Reregistering child plugin at" << cidx)
        _childPlugins.takeAt(cidx);
    }
}


void
RLANN_CNTRL_DrawController::onAnnotationImageChanged(RLANN_DS_Annotation* ann)
{
    // RLP_LOG_DEBUG("")
    updateDisplay();
}


bool
RLANN_CNTRL_DrawController::setBrush(QString brushName)
{
    RLP_LOG_DEBUG(brushName);

    bool result = false;

    if (_trackFactory->hasBrush(brushName)) {
        RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track*>(activeTrack().get());
        if (track != nullptr) {
            track->setBrush(_trackFactory->getBrush(brushName));
        }
    }

    return result;
}


void
RLANN_CNTRL_DrawController::setBrushSettings(QVariantMap settings)
{
    RLP_LOG_DEBUG(settings)
    
    RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track*>(activeTrack().get());
    if (track != nullptr) {
        track->brush()->updateSettings(settings);
    }
    
    // default value for not yet created tracks
    //
    _trackFactory->setBrushSettings(settings);
}



void
RLANN_CNTRL_DrawController::scheduleReleaseTask(qlonglong frameNum)
{
    // RLP_LOG_DEBUG("DISABLED");

    return;
    
    for (int tx=0; tx != _controller->session()->numTracks(); ++tx) {

        DS_TrackPtr track = _controller->session()->getTrackByIndex(tx);

        if (track->trackType() == TRACK_TYPE_ANNOTATION) {

            RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());
            if (anntrack->hasAnnotation(frameNum)) {
                
                RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(frameNum);
                if (ann->endFrame() == frameNum) {

                    // RLP_LOG_DEBUG("Schedule release: " << frameNum << " " << ann->uuid().toString());

                    QRunnable* task = new RLANN_DS_AnnotationReleaseTask(
                        this,
                        tx,
                        ann
                    );

                    _tpool->start(task);
                }
            }
        }
        
    }
}


void
RLANN_CNTRL_DrawController::scheduleReloadAnnotationTask(qlonglong frameNum, bool runImmediate)
{
    // return;

    // RLP_LOG_DEBUG(frameNum);

    for (int tx=0; tx != _controller->session()->numTracks(); ++tx) {

        DS_TrackPtr track = _controller->session()->getTrackByIndex(tx);

        if (track->trackType() == TRACK_TYPE_ANNOTATION) {

            RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());
            if (anntrack->hasAnnotation(frameNum)) {

                RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(frameNum);
                if (!ann->isReady()) {

                    // RLP_LOG_DEBUG(frameNum);

                    // ann->reload();
                    ann->reallocate();

                    QRunnable* task = new RLANN_DS_AnnotationReloadTask(ann);

                    if (runImmediate) {
                        task->run();
                        delete task;

                    } else {
                        _tpool->start(task);
                    }

                }
            }
        }
    }
}


void
RLANN_CNTRL_DrawController::registerAnnotationForSync(QString trackUuidStr, QString annUuidStr)
{
    #ifdef SCAFFOLD_IOS
    return;
    #endif
    

    RLP_LOG_DEBUG(trackUuidStr << " " << annUuidStr);

    if (!_doSyncAnnotations)
    {
        RLP_LOG_DEBUG("Skipping sync annotations")
        return;
    }


    // Queue a sync update of the media for this track the next time the update timer rolls around
    QPair<QString, QString> trackFramePair(trackUuidStr, annUuidStr);
    _updateSet.insert(trackFramePair);

    _updateTimer.start(RLANN_CNTRL_DrawController::UPDATE_TIMER_INTERVAL);
}


//
// -----------------
//

RLANN_CNTRL_DisplayPlugin::RLANN_CNTRL_DisplayPlugin(RLANN_CNTRL_DrawController* annCntrl):
    DS_EventPlugin("Annotation Display Plugin", true),
    _annCntrl(annCntrl),
    _drawing(false),
    _windowWidth(0),
    _windowHeight(0),
    _displayZoom(1.0)
{
    _username = UTIL_AppGlobals::instance()->globalsPtr()->value("username").toString();
}


RLANN_CNTRL_DisplayPlugin::~RLANN_CNTRL_DisplayPlugin()
{
    RLP_LOG_DEBUG("")
}


bool
RLANN_CNTRL_DisplayPlugin::setDisplay(DISP_GLView* display)
{
    DS_EventPlugin::setDisplay(display);

    _displayZoom = _display->getZoom();

    connect(
        _display,
        SIGNAL(displayZoomChanged(float)),
        this,
        SLOT(onDisplayZoomChanged(float))
    );

    connect(
        _display,
        &DISP_GLView::hoverEnter,
        this,
        &RLANN_CNTRL_DisplayPlugin::onDisplayHoverEnter
    );

    return true;
}



void
RLANN_CNTRL_DisplayPlugin::strokeTo(const QPointF &endPoint, qlonglong frame, RLANN_DS_Track* track)
{
    // RLP_LOG_DEBUG(endPoint.x() << endPoint.y());
    
    if (!track->hasAnnotation(frame)) {
        RLP_LOG_ERROR("NO CURRENT ANNOTATION ON FRAME " << frame);
        return;
    }
    
    RLANN_DS_AnnotationPtr ann = track->getAnnotation(frame); // getAnnotationMap()->value(_currFrame);
    
    // RLP_LOG_DEBUG(endPointIn.x() << "->" << endPoint.x() << "   " << endPointIn.y() << "->" << endPoint.y());
    // RLP_LOG_DEBUG(ann->uuid().toString().toStdString().c_str());
    
    QPainter painter(ann->getQImage());

    track->brush()->strokeTo(&painter, endPoint);


    // NOTE: I don't think the update rect is being respected here - paintGL()
    // is updating the whole frame
    //
    /*
    int rad = (_penWidth / 2) + 2;
    _display->update(QRect(_lastPoint, endPoint)
                     .normalized()
                     .adjusted(-rad, -rad, +rad, +rad));
    */
    
    // Update all Views
    _annCntrl->getMainController()->getVideoManager()->update();

}


bool
RLANN_CNTRL_DisplayPlugin::hoverMoveEventInternal(QHoverEvent* event)
{
    qlonglong currFrame = _annCntrl->currFrame();
    DS_TrackPtr atrack = _annCntrl->activeTrack();
    if (atrack == nullptr)
    {
        // RLP_LOG_WARN("no active track")
        return false;
    }

    RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(atrack.get());

    if (anntrack->brush() == nullptr)
    {
        RLP_LOG_WARN("No brush on current annotation track")
        return false;
    }

    RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(currFrame);
    if (ann == nullptr)
    {
        // RLP_LOG_WARN("no annotation")
        return false;
    }


    // ann->setValid();
    // ann->setDirty();

    QPointF panPercent = _display->getPanPercent();
    
    QPointF transformedPoint = ann->mapToImage(
        _display->uuid(),
        event->pos(),
        panPercent
    );

    QVariantMap extra;
    QPointF epos = event->position();
    extra.insert("event.pos", epos);

    anntrack->brush()->strokeMove(transformedPoint, extra);

    QVariantMap kwargs;
    kwargs.insert("x", epos.x());
    kwargs.insert("y", epos.y());
    kwargs.insert("user", _username);

    // RLP_LOG_DEBUG("emit hoverMove for sync" << kwargs)
    _controller->emitSyncAction("hoverMove", kwargs);

    return false;
}


bool
RLANN_CNTRL_DisplayPlugin::mousePressEventInternal(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    // if (_annCntrl->isPicking())
    // {
    //     RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(_annCntrl->activeTrack().get());
    //     QVariantMap settings = atrack->brush()->settings();
    //     _annCntrl->emitBrushInfo(settings);

    //     return false;
    // }

    CmdFilterResult res = _controller->checkCommand("Draw");
    if (!res.first)
    {
        return false;
    }

    // Ignore if any key modifiers are pressed
    //
    if (!event->modifiers().testFlag(Qt::NoModifier))
    {
        return false;
    }

    qlonglong currFrame = _annCntrl->currFrame();

    // RLP_LOG_DEBUG("");

    // NOTE: hardcode disable plugin for now
    // return true;
    
    RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(_annCntrl->activeTrack().get());
    if (!atrack)
    {
        RLP_LOG_WARN("No active track!")
        return false;
    }

    QVariantMap settings = atrack->brush()->settings();
    _annCntrl->emitBrushInfo(settings);

    if ((event->button() == Qt::LeftButton) &&
        (atrack != nullptr) &&
        (atrack->isEditable()) && 
        (atrack->hasAnnotation(currFrame)))
    {

        RLANN_DS_AnnotationPtr ann = atrack->getAnnotation(currFrame);
        ann->setValid();
        ann->setDirty();

        QPointF panPercent = _display->getPanPercent();

        QPointF lastPoint = ann->mapToImage(
            _display->uuid(),
            event->pos(),
            panPercent
        );

        _drawing = true;
        atrack->brush()->setupFrame(ann);
        atrack->brush()->strokeBegin(lastPoint);

        _annCntrl->stopUpdateTimer();
        _annCntrl->emitSyncDrawStart(currFrame, atrack->uuid().toString());


        QVariantMap kwargs;
        kwargs.insert("ann_uuid", ann->uuid());
        kwargs.insert("frame", currFrame);
        kwargs.insert("x", lastPoint.x());
        kwargs.insert("y", lastPoint.y());
        kwargs.insert("track_uuid", atrack->uuid().toString());
        kwargs.insert("brush", atrack->brush()->settings());

        _controller->emitSyncAction("drawStart", kwargs);

        // RLP_LOG_DEBUG("LAST POS: " << lastPoint.x() << " " << lastPoint.y());

        return true;
    }

    return false;
}


bool
RLANN_CNTRL_DisplayPlugin::mouseMoveEventInternal(QMouseEvent* event)
{

    CmdFilterResult res = _controller->checkCommand("Draw");
    if (!res.first)
    {
        return false;
    }

    // Ignore if any key modifiers are pressed
    //
    if (!event->modifiers().testFlag(Qt::NoModifier))
    {
        return false;
    }


    if ((event->buttons() & Qt::LeftButton) && _drawing)
    {

        // RLP_LOG_DEBUG("RLANN_CNTRL_DrawController::mouseMoveEventInternal(): [SYNC ACTION] " << event->pos().x() << " " << event->pos().y());
        qlonglong currFrame = _annCntrl->currFrame();

        RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track *>(_annCntrl->activeTrack().get());
        RLANN_DS_AnnotationPtr ann = track->getAnnotation(currFrame);
        
        QPointF panPercent = _display->getPanPercent();
        float displayZoom = _display->getZoom();

        // RLP_LOG_DEBUG(pp.x() << " " << pp.y());
        
        // localPos() is correct (instead of event->pos()) if event is QMouseEvent
        //
        QPointF imgPoint = ann->mapToImage(
            _display->uuid(),
            event->pos(),
            panPercent
        );

        // RLP_LOG_DEBUG(event->pos() << " " << imgPoint << " " << displayZoom << " pan: " << panPercent)

        strokeTo(imgPoint, currFrame, track);

        QVariantMap kwargs;
        
        kwargs.insert("frame", (qlonglong)currFrame);
        kwargs.insert("x", imgPoint.x());
        kwargs.insert("y", imgPoint.y());
        kwargs.insert("mx", event->pos().x()); // mouse x
        kwargs.insert("my", event->pos().y()); // mouse y
        kwargs.insert("user", _username);
        kwargs.insert("track_uuid", track->uuid().toString());

        _controller->emitSyncAction("draw", kwargs);
    }

    return true;
}


bool
RLANN_CNTRL_DisplayPlugin::mouseReleaseEventInternal(QMouseEvent* event)
{
    RLP_LOG_DEBUG("")

    CmdFilterResult res = _controller->checkCommand("Draw");
    if (!res.first)
    {
        return false;
    }

    // if (_annCntrl->isPicking())
    // {
    //     DS_TrackPtr atrack = _annCntrl->activeTrack();
    //     if (atrack == nullptr) {
    //         RLP_LOG_ERROR("no active track!");
    //         return false;
    //     }

    //     RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track *>(atrack.get());

    //     track->brush()->strokeEnd();
    //     return false;

    // }


    // Ignore if any key modifiers are pressed
    //
    if (!event->modifiers().testFlag(Qt::NoModifier)) {
        return false;
    }

    // RLP_LOG_DEBUG("");

    _drawing = false;

    DS_TrackPtr atrack = _annCntrl->activeTrack();
    if (atrack == nullptr) {
        RLP_LOG_ERROR("no active track!");
        return false;
    }

    RLANN_DS_Track* track = dynamic_cast<RLANN_DS_Track *>(atrack.get());

    if (!track->isEditable())
    {
        track->brush()->strokeEnd(nullptr);
        return false;
    }

    qlonglong currFrame = _annCntrl->currFrame();
    RLANN_DS_AnnotationPtr ann = track->getAnnotation(currFrame);

    track->brush()->strokeEnd(ann);

    if (ann != nullptr) {

        // QString annImageFilePath = track->getCacheFilePath(ann->fileName());
        // ann->save(annImageFilePath);
        // QString annCacheImagePath = track->getAnnotationCacheFilePathQString("_currFrame);
        // ann->getQImage()->save(annCacheImagePath);

        // RLP_LOG_DEBUG("RLANN_CNTRL_DrawController::mouseReleaseEventInternal(): SAVED: " << annImageFilePath.toStdString().c_str());
        ann->emitStrokeEnd();

        QVariantMap kwargs;
        kwargs.insert("frame", (qlonglong)currFrame);
        kwargs.insert("ann_uuid", ann->uuid().toString());
        kwargs.insert("track_uuid", track->uuid().toString());

        _controller->emitSyncAction("drawEnd", kwargs);

        _annCntrl->registerAnnotationForSync(track->uuid().toString(), ann->uuid().toString());
    }
    

    _annCntrl->emitSyncDrawEnd(currFrame, track->uuid().toString());
    
    return true;
}


void
RLANN_CNTRL_DisplayPlugin::updateAnnDisplayWindow(RLANN_DS_AnnotationPtr ann)
{
    // RLP_LOG_DEBUG(_windowWidth << _windowHeight)

    ann->setWindow(
        _display->uuid(),
        _windowWidth,
        _windowHeight,
        _displayZoom
    );

}


void
RLANN_CNTRL_DisplayPlugin::onDisplayZoomChanged(float displayZoom)
{
    // RLP_LOG_DEBUG(displayZoom << "current zoom:" << _display->getZoom())

    // For some reason, we may get a delayed sig-slot connection delivered here
    // for a zoom value that has since been outdated (maybe because of a destructed view or a resized view)
    // use the zoom value that is currently on the display, and not the incoming value
    _displayZoom = _display->getZoom();

    resizeGLInternal(_windowWidth, _windowHeight);
}


void
RLANN_CNTRL_DisplayPlugin::onDisplayHoverEnter()
{
    // RLP_LOG_DEBUG("")

    RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(_annCntrl->activeTrack().get());
    if (anntrack != nullptr)
    {
        anntrack->brush()->activate(_display, _annCntrl);
    }
}


void
RLANN_CNTRL_DisplayPlugin::resizeGLInternal(int width, int height)
{
    // RLP_LOG_DEBUG(width << " " << height);

    _windowWidth = width;
    _windowHeight = height;
    
    qlonglong currFrame = _annCntrl->currFrame();

    DS_TrackPtr track = _annCntrl->activeTrack();
    if (track != nullptr) {
        RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        if (anntrack->hasAnnotation(currFrame))
        {

            anntrack->getAnnotation(currFrame)->setWindow(
                _display->uuid(),
                width,
                height,
                _displayZoom
            );

        }
    }
}


void
RLANN_CNTRL_DisplayPlugin::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata)
{
    DS_TrackPtr track = _annCntrl->activeTrack();
    if (track != nullptr)
    {
        RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        anntrack->brush()->paintGLPostDraw(painter, mdata);
    }

    if (_annCntrl->property("pref.show_remote_cursors").toBool())
    {
        StringPointMap* spm = _annCntrl->userPositionMap();
        StringPointMap::iterator it;

        for (it = spm->begin(); it != spm->end(); ++it)
        {
            QString username = it.key();
            QPoint pos = it.value();

            painter->setPen(Qt::black);
            painter->drawCircle(pos.x() + 1, pos.y() + 1, 3);
            painter->drawText(pos.x() + 14, pos.y() + 14, username);
            painter->setPen(Qt::white);
            painter->drawCircle(pos.x(), pos.y(), 3);
            painter->drawText(pos.x() + 12, pos.y() + 12, username);
        }
    }
}