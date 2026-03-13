//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLANN_CNTRL/DrawController.h"
#include "RlpExtrevlens/RLANN_CNTRL/DisplayPlugin.h"

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

RLP_SETUP_LOGGER(extrevlens, RLANN_CNTRL, DrawController)

int RLANN_CNTRL_DrawController::UPDATE_TIMER_INTERVAL = 2500;

RLANN_CNTRL_DrawController* RLANN_CNTRL_DrawController::_instance;

RLANN_CNTRL_DrawController::RLANN_CNTRL_DrawController(bool setSingleton):
    RLANN_DS_DrawControllerBase(),
    _drawing(false),
    _paperTraceMode(RLANN_CNTRL_DrawController::PAPER_TRACE_ALWAYS_ON),
    _doPaperTrace(false),
    _paperTraceOpacity(0.7),
    _currFrame(-1),
    _doSyncAnnotations(true),
    _brushEngine("MyPaint")
{
    _tpool = new QThreadPool();
    _tpool->setMaxThreadCount(1);
    
    _updateSet.clear();
    connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(onTimerMediaUpdate()));

    // TODO FIXME: MAY NOT BE REQUIRED
    //
    _frameProbe = new RLANN_CNTRL_FrameProbe(this);
    
    _trackFactory = new RLANN_CNTRL_TrackFactoryPlugin();

    if (setSingleton)
    {
        RLANN_CNTRL_DrawController::setInstance(this);
    }

    RLANN_MP_Handler* mph = RLANN_MP_Handler::instance();

    _paperTraceFbuf = RLANN_DS_QImageFrameBufferPtr(
        new RLANN_DS_QImageFrameBuffer(this, 0, 0));

    _remoteUserPositionMap = new StringMousePosMap();

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
RLANN_CNTRL_DrawController::setMainController(DS_ControllerBase* controller)
{
    DS_EventPlugin::setMainController(controller);

    controller->session()->registerTrackFactoryPlugin(
        _trackFactory->name(),
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

    controller->registerCommand(new RLANN_CNTRL_AnnotateCommand("annotate", controller));

    CNTRL_MainController* mcntrl = qobject_cast<CNTRL_MainController*>(controller);
    if (mcntrl != nullptr)
    {
        mcntrl->getPluginManager()->registerVideoProbe(_frameProbe);
    }

    return true;
}




void
RLANN_CNTRL_DrawController::setActiveTrack(QString uuidStr)
{
    // RLP_LOG_DEBUG(uuidStr);

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
    // return;
    #endif

    RLP_LOG_DEBUG(doSync);

    _doSyncAnnotations = doSync;
    if (_doSyncAnnotations)
    {
        _updateTimer.start(RLANN_CNTRL_DrawController::UPDATE_TIMER_INTERVAL);
    }
}


DS_TrackPtr
RLANN_CNTRL_DrawController::activeTrack()
{
    DS_SessionPtr session = _controller->session();
    if (session != nullptr)
    {
        return _controller->session()->getTrackByUuid(_activeTrackUuidStr);
    }

    return nullptr;
}


RLANN_DS_Track*
RLANN_CNTRL_DrawController::getAnnotationTrackByUuid(QString trackUuidStr)
{
    RLANN_DS_Track* track = nullptr;
    if (trackUuidStr == "")
    {
        track = dynamic_cast<RLANN_DS_Track*>(activeTrack().get());
    } else
    {
        DS_TrackPtr strack = _controller->session()->getTrackByUuid(trackUuidStr);
        if ((strack) && (strack->trackType() == TRACK_TYPE_ANNOTATE))
        {
            track = dynamic_cast<RLANN_DS_Track*>(strack.get());
        }
    }

    if (track == nullptr)
    {
        RLP_LOG_ERROR("Could not obtain track from UUID" << trackUuidStr)
    }

    return track;
}


void
RLANN_CNTRL_DrawController::setupFrame(qlonglong frameNum, QString trackUuidStr, QString annUuidStr)
{
    // RLP_LOG_DEBUG(frameNum << " " << trackUuidStr << " " << annUuidStr);
    
    _currFrame = frameNum;
    
    // AnnotationFrameMap* annmap = getAnnotationMap();
    
    DS_TrackPtr atrack = nullptr;
    if (trackUuidStr == "")
    {
        atrack = activeTrack();

    } else
    {
        atrack = _controller->session()->getTrackByUuid(trackUuidStr);
    }

    if (atrack != nullptr)
    {

        RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(atrack.get());

        if (!anntrack->hasAnnotation(frameNum))
        {

            // RLP_LOG_DEBUG("no annotation on frame " << frameNum << " - initializing");

            DS_NodePtr node = _controller->session()->getNodeByFrame(frameNum);
            if (node == nullptr)
            {
                return;

            } else
            {

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

                QImage currImageFrame = _controller->getVideoManager()->currentFrameBuffer()->toQImage();
                
                // NOTE: high-quality non-realtime-held frame mode
                ann->saveImageToHistory(currImageFrame);

                // RLP_LOG_DEBUG("Saved background image:" << ann->historySize())
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
    } else
    {
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


QImage
RLANN_CNTRL_DrawController::extractAnnotation(QImage bg, QImage rawAnnotation)
{
    RLP_LOG_DEBUG("")

    return RLANN_DS_Annotation::extractAnnotation(bg, rawAnnotation);
}


QImage
RLANN_CNTRL_DrawController::flattenAnnotation(QImage bg, QImage rawAnnotation)
{
    RLP_LOG_DEBUG("")

    return RLANN_DS_Annotation::flattenAnnotation(bg, rawAnnotation);
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

    if (action == "drawStart")
    {

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
        QVariantMap mdata;
        mdata.insert("settings", brushSettings);
        QString brushName = brushSettings.value("name").toString();

        atrack->setBrush(brushName);
        atrack->brush()->setupFrame(ann);
        atrack->brush()->strokeBegin(lp, mdata);

        _drawing = true;

        emit syncDrawStart(frame, trackUuidStr);
        
    } else if (action == "draw")
    {

        float x = kwargs.value("x").toFloat();
        float y = kwargs.value("y").toFloat();
        QString user = kwargs.value("user").toString();

        QPointF point(x, y);
        
        MousePos mpos {
            kwargs.value("x").toFloat(),
            kwargs.value("y").toFloat(),
            kwargs.value("w").toFloat(),
            kwargs.value("h").toFloat()
        };

        _remoteUserPositionMap->insert(user, mpos);


        QVariantMap mdata;
        if (kwargs.contains("p"))
        {
            mdata.insert("pressure", kwargs.value("p"));
            mdata.insert("xtilt", kwargs.value("xt"));
            mdata.insert("ytilt", kwargs.value("yt"));
        }

        for (int ci=0; ci != _childPlugins.size(); ++ci)
        {
            qobject_cast<RLANN_CNTRL_DisplayPlugin*>(_childPlugins.at(ci))->strokeTo(
                atrack, frame, point, mdata);
        }
        

    } else if (action == "drawEnd")
    {

        _drawing = false;
        emit syncDrawEnd(frame, trackUuidStr);

        QString annUuidStr = kwargs.value("ann_uuid").toString();
        registerAnnotationForSync(trackUuidStr, annUuidStr);

    } else if (action == "holdFrame")
    {
        
        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
        if (track != nullptr)
        {
            
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

        } else
        {
            RLP_LOG_DEBUG("TRACK NOT FOUND: " << trackUuidStr);
        }
    } else if (action == "releaseFrame")
    {

        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
        if (track != nullptr)
        {
            
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
    } else if (action == "clearAnnotations")
    {

        QString func = "rlplug_annotate.cmds.clearAnnotations";
        QVariantList args;
        args.push_back((qlonglong)frame);
        args.push_back(false);
        args.push_back(trackUuidStr);

        PY_Interp::call(func, args);

    } else if (action == "hoverMove")
    {

        // RLP_LOG_DEBUG(kwargs);

        QString username = kwargs.value("user").toString();

        MousePos mpos {
            kwargs.value("x").toFloat(),
            kwargs.value("y").toFloat(),
            kwargs.value("w").toFloat(),
            kwargs.value("h").toFloat()
        };

        _remoteUserPositionMap->insert(username, mpos);

        updateDisplay();

    } else if (action == "setBrushSettings")
    {
        QString trackUuidStr = kwargs.value("track_uuid").toString();
        setBrushSettings(kwargs, trackUuidStr, false);
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

        if (ann != nullptr)
        {
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

    // offload / reload is screwing with this since the annotation data may not be available when this is called
    // RLP_LOG_WARN("DISABLED")
    // return;


    if ((_drawing) || (_controller->getPlaybackState() == CNTRL_MainController::PLAYSTATE_PLAYING))
    {
        RLP_LOG_WARN("drawing or playing, skipping");
        return;
    }

    // RLP_LOG_DEBUG("");

    QSet<QPair<QString, QString>>::iterator it;

    for (it = _updateSet.begin(); it != _updateSet.end(); ++it)
    {

        // RLP_LOG_DEBUG(_updateSet.size());

        QPair<QString, QString> trackAnnPair = (*it);
        QString trackUuidStr = trackAnnPair.first;
        QString annUuidStr = trackAnnPair.second;

        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
        if (track == nullptr)
        {
            return;
        }

        RLANN_DS_Track* atrack = dynamic_cast<RLANN_DS_Track*>(track.get());
        if (atrack == nullptr)
        {
            return;
        }

        RLANN_DS_AnnotationPtr ann = atrack->getAnnotationByUuid(annUuidStr);
        if (ann == nullptr)
        {
            return;
        }
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

        // RLP_LOG_DEBUG("emitting media update for track "
        //     << trackUuidStr << " "
        //     << " annotation: " << ann->uuid().toString()
        // );

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
    // return;
    #endif

    // RLP_LOG_DEBUG(playState);

    if (playState == (int)CNTRL_MainController::PLAYSTATE_PAUSED)
    {
        _updateTimer.start(RLANN_CNTRL_DrawController::UPDATE_TIMER_INTERVAL);

    } else if (playState == (int)CNTRL_MainController::PLAYSTATE_PLAYING)
    {
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
        RLP_LOG_DEBUG("Deregistering child plugin at" << cidx)
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
RLANN_CNTRL_DrawController::setBrush(QString brushName, QString trackUuidStr)
{
    RLP_LOG_DEBUG(brushName << trackUuidStr)

    bool result = false;

    RLANN_DS_Track* track = getAnnotationTrackByUuid(trackUuidStr);
    if (track != nullptr)
    {
        track->setBrush(brushName);
        
        QVariantMap info;
        info.insert("brush_name", brushName);
        info.insert("track_uuid", track->uuid());
        _controller->emitSyncAction("setBrush", info);

        result = true;
    }

    return result;
}


void
RLANN_CNTRL_DrawController::setBrushSettings(QVariantMap brushSettings, QString trackUuidStr, bool emitSyncAction)
{
    RLP_LOG_DEBUG(brushSettings << trackUuidStr)

    RLANN_DS_Track* track = getAnnotationTrackByUuid(trackUuidStr);
    if (track == nullptr)
    {
        return;
    }

    track->brush()->updateSettings(brushSettings);

    if (emitSyncAction)
    {
        brushSettings.insert("track_uuid", track->uuid().toString());
        _controller->emitSyncAction("setBrushSettings", brushSettings);
    }
}



void
RLANN_CNTRL_DrawController::scheduleReleaseTask(qlonglong frameNum)
{
    // RLP_LOG_DEBUG("DISABLED");

    return;
    
    for (int tx=0; tx != _controller->session()->numTracks(); ++tx) {

        DS_TrackPtr track = _controller->session()->getTrackByIndex(tx);

        if (track->trackType() == TRACK_TYPE_ANNOTATE)
        {

            RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());
            if (anntrack->hasAnnotation(frameNum))
            {
                RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(frameNum);
                if (ann->endFrame() == frameNum)
                {

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
    return;

    // RLP_LOG_DEBUG(frameNum);

    for (int tx=0; tx != _controller->session()->numTracks(); ++tx) {

        DS_TrackPtr track = _controller->session()->getTrackByIndex(tx);

        if (track->trackType() == TRACK_TYPE_ANNOTATE) {

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
    // return;
    #endif

    // RLP_LOG_DEBUG(trackUuidStr << " " << annUuidStr);

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
