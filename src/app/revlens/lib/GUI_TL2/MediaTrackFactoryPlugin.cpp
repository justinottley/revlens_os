
#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI_TL2/Track.h"
#include "RlpRevlens/GUI_TL2/MediaTrackFactoryPlugin.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, MediaTrackFactoryPlugin)

GUI_TL2_MediaTrackFactoryPlugin::GUI_TL2_MediaTrackFactoryPlugin(GUI_TL2_TrackManager* tmgr):
    GUI_TL2_TrackFactoryPluginBase::GUI_TL2_TrackFactoryPluginBase(QString("Media"), tmgr)
{
    startup();
}



GUI_TL2_MediaTrackFactoryPlugin::~GUI_TL2_MediaTrackFactoryPlugin()
{
    RLP_LOG_DEBUG("")
}


void
GUI_TL2_MediaTrackFactoryPlugin::startup()
{
    RLP_LOG_DEBUG("")

    onSessionChanged(controller()->session());
}


GUI_TL2_TrackFactoryPluginBase*
GUI_TL2_MediaTrackFactoryPlugin::create(GUI_TL2_TrackManager* tmgr)
{
    return new GUI_TL2_MediaTrackFactoryPlugin(tmgr);
}


void
GUI_TL2_MediaTrackFactoryPlugin::onSessionChanged(DS_SessionPtr session)
{
    // RLP_LOG_DEBUG("");

    connect(
        session.get(),
        SIGNAL(mediaAdded(DS_NodePtr, QString, qlonglong, qlonglong)),
        this,
        SLOT(onMediaAdded(DS_NodePtr, QString, qlonglong, qlonglong))
    );  

    connect(
        _trackManager->view()->controller(),
        &CNTRL_MainController::remoteCall,
        this,
        &GUI_TL2_MediaTrackFactoryPlugin::onRemoteCallReceived
    );
}


void
GUI_TL2_MediaTrackFactoryPlugin::onMediaAdded(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount)
{
    RLP_LOG_DEBUG(trackUuidStr << _trackManager)

    GUI_TL2_Track* track = _trackManager->getTrackByUuid(trackUuidStr);
    if (track != nullptr) {

        RLP_LOG_DEBUG("GOT TRACK: "  << track->name());

        if (track->trackType() == TRACK_TYPE_MEDIA)
        {
            dynamic_cast<GUI_TL2_MediaTrack*>(track)->addMediaItem(node, frameNum, mediaFrameCount);
        } else 
        {
            RLP_LOG_DEBUG("Not a media track: " << trackUuidStr << ", skipping")
        }
        
        
    } else {
        RLP_LOG_ERROR("Track UUID not found: " << trackUuidStr);

    }
}


void
GUI_TL2_MediaTrackFactoryPlugin::onRemoteCallReceived(QVariantMap callInfo)
{
    // RLP_LOG_VERBOSE(callInfo)

    QString methodName = callInfo.value("method").toString();
    if (methodName == "export_frames_complete")
    {
        QVariantList argList = callInfo.value("args").toList();
        QVariantList frameList = argList.at(1).toList();

        qlonglong frameNum = 1;
        if (frameList.size() > 1)
        {
            frameNum = frameList[1].toLongLong();
        }

        QString outputDir = argList.at(2).toString();
        QString exportPath = QString("%1/export_%2.jpg").arg(outputDir).arg(frameNum);

        RLP_LOG_DEBUG("GOT EXPORT FRAMES COMPLETE: " << frameList << exportPath)

        QVariantMap extra = argList.at(3).toMap();
        QString trackUuid = QString("{%1}").arg(extra.value("track_uuid").toString());
        QString mediaUuid = QString("{%1}").arg(extra.value("media_uuid").toString());

        
        RLP_LOG_DEBUG("Track uuid:" << trackUuid << "media uuid:" << mediaUuid)

        GUI_TL2_Track* track = _trackManager->getTrackByUuid(trackUuid);
        GUI_TL2_MediaItem* item = qobject_cast<GUI_TL2_MediaItem*>(track->getItemByUuid(mediaUuid));
        
        QVariantMap itemData;
        itemData.insert("data_type", "thumbnail");
        itemData.insert("thumbnail.path", exportPath);

        item->onDataReady(itemData);

    }
}


GUI_TL2_Track*
GUI_TL2_MediaTrackFactoryPlugin::runCreateTrack(QString uuidStr, QString trackName)
{
    RLP_LOG_DEBUG(trackName << " " << uuidStr);

    DS_TrackPtr strack = controller()->session()->getTrackByUuid(uuidStr);

    RLP_LOG_DEBUG("Got Session Track")
    
    GUI_TL2_MediaTrack* track = new GUI_TL2_MediaTrack(_trackManager, strack, trackName);
    
    return track;
}

