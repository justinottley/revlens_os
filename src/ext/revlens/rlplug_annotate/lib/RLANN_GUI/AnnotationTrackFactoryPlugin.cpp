
#include "RlpExtrevlens/RLANN_GUI/AnnotationTrackFactoryPlugin.h"

#include "RlpExtrevlens/RLANN_GUI/AnnotationItem.h"

RLP_SETUP_EXT_LOGGER(RLANN_GUI, AnnotationTrackFactoryPlugin)


RLANN_GUI_AnnotationTrackFactoryPlugin::RLANN_GUI_AnnotationTrackFactoryPlugin(
    GUI_TL2_TrackManager* tmgr,
    RLANN_CNTRL_DrawController* drawController):
        GUI_TL2_TrackFactoryPluginBase::GUI_TL2_TrackFactoryPluginBase(
            QString("Annotation"),
            tmgr
        ),
        _drawController(drawController)
{
    initConnections();
}


GUI_TL2_TrackFactoryPluginBase*
RLANN_GUI_AnnotationTrackFactoryPlugin::create(GUI_TL2_TrackManager* tmgr)
{
    RLANN_CNTRL_DrawController* drawCntrl = RLANN_CNTRL_DrawController::instance();
    return new RLANN_GUI_AnnotationTrackFactoryPlugin(tmgr, drawCntrl);
}


void
RLANN_GUI_AnnotationTrackFactoryPlugin::initConnections()
{
    connect(
        CNTRL_MainController::instance(),
        SIGNAL(syncAction(QString, QVariantMap)),
        this,
        SLOT(onSyncAction(QString, QVariantMap))
    );


    connect(
        _drawController,
        SIGNAL(syncDrawStart(qlonglong, QString)),
        this,
        SLOT(onDrawStart(qlonglong, QString))
    );


    connect(
        _drawController,
        SIGNAL(syncHoldFrame(QString, QString, qlonglong, qlonglong)),
        this,
        SLOT(onHoldFrame(QString, QString, qlonglong, qlonglong))
    );


    connect(
        _drawController,
        SIGNAL(syncReleaseFrame(QString, QString, qlonglong)),
        this,
        SLOT(onReleaseFrame(QString, QString, qlonglong))
    );



}
GUI_TL2_Track*
RLANN_GUI_AnnotationTrackFactoryPlugin::runCreateTrack(QString uuidStr, QString trackName)
{
    DS_TrackPtr strack = controller()->session()->getTrackByUuid(uuidStr);
    RLANN_GUI_Track* track = new RLANN_GUI_Track(_trackManager, strack, trackName, _drawController);
    return track;
}


void
RLANN_GUI_AnnotationTrackFactoryPlugin::postCreateTrack(GUI_TL2_Track* track)
{
    RLP_LOG_DEBUG(track->name());
    
    _drawController->setActiveTrack(track->uuid().toString());
}


// ------

void
RLANN_GUI_AnnotationTrackFactoryPlugin::onSessionChanged(DS_SessionPtr session)
{
    // RLP_LOG_DEBUG("");

    connect(
        session.get(),
        SIGNAL(trackDataChanged(QString, QVariantMap)),
        this,
        SLOT(onTrackDataChanged(QString, QVariantMap))
    );
}


void
RLANN_GUI_AnnotationTrackFactoryPlugin::onDrawStart(qlonglong drawFrame, QString trackUuidStr)
{
    // RLP_LOG_DEBUG(drawFrame << " " << trackUuidStr);

    DS_SessionPtr session = _drawController->getMainController()->session();
    DS_TrackPtr track = session->getTrackByUuid(trackUuidStr);
    RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());

    RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(drawFrame);
    if (ann != nullptr)
    {
        RLANN_GUI_Track* guiTrack = dynamic_cast<RLANN_GUI_Track*>(_trackManager->getTrackByUuid(trackUuidStr));
        if (!guiTrack->hasAnnotationItem(drawFrame))
        {
            guiTrack->addAnnotationItem(ann, drawFrame);
        }
    }
}


void
RLANN_GUI_AnnotationTrackFactoryPlugin::onHoldFrame(QString trackUuidStr, QString annUuidStr, qlonglong srcFrame, qlonglong destFrame)
{
    RLP_LOG_DEBUG(trackUuidStr << " " << annUuidStr << " " << srcFrame << " " << destFrame);

    RLANN_GUI_Track* guiTrack = dynamic_cast<RLANN_GUI_Track*>(_trackManager->getTrackByUuid(trackUuidStr));
    if (guiTrack != nullptr)
    {
        guiTrack->holdFrame(srcFrame, destFrame);
        GUI_TL2_Item* item = guiTrack->getItemByUuid(annUuidStr);

        if (item != nullptr)
        {
            if ((destFrame + 1) > item->endFrame())
            {
                item->setEndFrame(destFrame + 1);

            } else if (destFrame < item->startFrame())
            {
                item->setStartFrame(destFrame);
            }
            
        } else
        {
            RLP_LOG_ERROR("Item not found with annotation Uuid " << annUuidStr);
        }

        guiTrack->view()->update();

    } else
    {
        RLP_LOG_WARN("Track not found: " << trackUuidStr);
    }
}


void
RLANN_GUI_AnnotationTrackFactoryPlugin::onReleaseFrame(QString trackUuidStr, QString annUuidStr, qlonglong frame)
{
    // RLP_LOG_DEBUG(trackUuidStr << " " << annUuidStr << " " << frame);

    RLANN_GUI_Track* guiTrack = dynamic_cast<RLANN_GUI_Track*>(_trackManager->getTrackByUuid(trackUuidStr));
    if (guiTrack != nullptr) {
        
        GUI_TL2_Item* item = guiTrack->getItemByUuid(annUuidStr);

        if (item != nullptr)
        {
            if (frame > item->startFrame())
            {
                for (qlonglong fx = frame; fx <= item->endFrame(); ++fx)
                {
                    guiTrack->releaseFrame(fx);    
                }

                item->setEndFrame(frame);

            } else if (frame <= item->startFrame())
            {
                for (qlonglong fx = frame; fx <= item->startFrame(); ++fx)
                {
                    guiTrack->releaseFrame(fx);
                }

                item->setStartFrame(frame + 1);
            }
            
        }

        guiTrack->view()->update();

    } else
    {
        RLP_LOG_WARN("Track not found: " << trackUuidStr);

    }
}




void
RLANN_GUI_AnnotationTrackFactoryPlugin::onTrackDataChanged(QString trackUuidStr, QVariantMap trackData)
{
    QString eventName = trackData.value("event").toString();

    // RLP_LOG_DEBUG(trackUuidStr << " " << eventName);

    if (eventName == "add_annotation") {
        
        QVariantList frameList = trackData.value("frame_list").value<QVariantList>();
        if (frameList.size() > 0) {

            DS_SessionPtr session = _drawController->getMainController()->session();
            DS_TrackPtr track = session->getTrackByUuid(trackUuidStr);
            RLANN_DS_Track* anntrack = dynamic_cast<RLANN_DS_Track*>(track.get());

            RLANN_DS_AnnotationPtr ann = anntrack->getAnnotation(frameList[0].toLongLong());
            
            onDrawStart(frameList[0].toLongLong(), trackUuidStr);
            for (int x=1; x != frameList.size(); ++x) {
                onHoldFrame(trackUuidStr, ann->uuid().toString(), frameList[0].toLongLong(), frameList[x].toLongLong());
            }
            
            
        }
    }
}


void
RLANN_GUI_AnnotationTrackFactoryPlugin::onSyncAction(QString action, QVariantMap kwargs)
{
    // RLP_LOG_DEBUG(action << kwargs)

    if (action == "holdFrame")
    {
        QVariantMap ukwargs = kwargs.value("update_info").toMap();
        QString viewUuid = ukwargs.value("view_uuid").toString();
        if (viewUuid != _trackManager->view()->uuid())
        {
            onHoldFrame(
                ukwargs.value("track_uuid").toString(),
                ukwargs.value("ann_uuid").toString(),
                ukwargs.value("frame").toLongLong(),
                ukwargs.value("dest_frame").toLongLong()
            );
        }
    } else if (action == "releaseFrame")
    {
        QVariantMap ukwargs = kwargs.value("update_info").toMap();
        QString viewUuid = ukwargs.value("view_uuid").toString();

        if (viewUuid != _trackManager->view()->uuid())
        {
            onReleaseFrame(
                ukwargs.value("track_uuid").toString(),
                ukwargs.value("ann_uuid").toString(),
                ukwargs.value("frame").toLongLong()
            );
        }
    }
}