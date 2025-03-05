//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpExtrevlens/RLSYNC/RoomController.h"

#include "RlpCore/PY/Interp.h"
#include "RlpCore/UTIL/AppGlobals.h"

#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/MediaManager.h"
#include "RlpRevlens/CNTRL/Server.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI_TL2/Controller.h"


RLP_SETUP_LOGGER(ext, RLSYNC, RoomController)

RLSYNC_RoomController::RLSYNC_RoomController():
    DS_EventPlugin("Sync", true),
    _syncOn(false),
    _doDataSync(false)
{
    _cmdFilter = new RLSYNC_CmdFilter(this);

    UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();

    // QString fsRootDir = ag->globalsPtr()->value("fs_root").toString();

    _appPid = ag->globalsPtr()->value("app.pid").toInt();
    _currentUser = ag->globalsPtr()->value("username").toString();

    _skwargs.clear();
    // _skwargs.insert("app.name", "rooms");
    _skwargs.insert("client.pid", _appPid);
    _skwargs.insert("client.user", _currentUser);

    leaveRoom();
}


bool
RLSYNC_RoomController::setMainController(CNTRL_MainController* controller)
{
    DS_EventPlugin::setMainController(controller);

    RLP_LOG_DEBUG("");

    controller->addCommandFilter(_cmdFilter);

    connect(
        controller,
        SIGNAL(remoteCall(QVariantMap)),
        this,
        SLOT(onRemoteCallReceived(QVariantMap))
    );
    
    connect(
        controller,
        SIGNAL(syncAction(QString, QVariantMap)),
        this,
        SLOT(onSyncAction(QString, QVariantMap))
    );

    
    //
    // TODO FIXME: will need revisiting for multi session setup
    //

    connect(
        controller->session().get(),
        SIGNAL(sessionCleared()),
        this,
        SLOT(onSessionCleared())
    );

    connect(
        controller->session().get(),
        SIGNAL(trackAdded(QString, QString, QString)),
        this,
        SLOT(onTrackAdded(QString, QString, QString))
    );

    connect(
        controller->session().get(),
        SIGNAL(mediaRemoved(DS_NodePtr, QString, qlonglong)),
        this,
        SLOT(onMediaRemoved(DS_NodePtr, QString, qlonglong))
    );

    connect(
        controller->session().get(),
        SIGNAL(mediaAdded(DS_NodePtr, QString, qlonglong, qlonglong)),
        this,
        SLOT(onMediaAdded(DS_NodePtr, QString, qlonglong, qlonglong))
    );

    connect(
        controller->getMediaManager(),
        SIGNAL(nodeDataReady(QVariantMap)),
        this,
        SLOT(onNodeDataReady(QVariantMap))
    );

    return true;
}


QString
RLSYNC_RoomController::clientIdent()
{
    return UTIL_AppGlobals::instance()->globalsPtr()->value("client.ident").toString();
}


void
RLSYNC_RoomController::initEdbFs()
{
    //
    // Should be called via sig-slot connection on successful login
    // wired in python
    //

    RLP_LOG_DEBUG("")

    UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();

    QString siteName = ag->globalsPtr()->value("edb.site_name").toString();

    RLP_LOG_DEBUG("Site Name:" << siteName)

    _skwargs.insert("site_name", siteName);
}


void
RLSYNC_RoomController::setRoomName(QString roomName)
{
    RLP_LOG_DEBUG(roomName);

    _roomName = roomName;
    _skwargs.insert("room_name", _roomName);

    _syncOn = true;
}


void
RLSYNC_RoomController::setRoomUuid(QString roomUuid)
{
    RLP_LOG_DEBUG(roomUuid)

    _roomUuid = roomUuid;
    _skwargs.insert("room_uuid", _roomUuid);
}


void
RLSYNC_RoomController::leaveRoom()
{
    RLP_LOG_INFO("")

    _roomName = "";
    _roomUuid = "";
    _syncOn = false;

    _skwargs.remove("room_name");
    _skwargs.remove("room_uuid");
}


void
RLSYNC_RoomController::onSessionCleared()
{
    RLP_LOG_DEBUG("")
}


void
RLSYNC_RoomController::onMediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame)
{
    RLP_LOG_DEBUG("");

    QVariantMap kwargs;
    kwargs.insert("media_uuid", node->graph()->uuid().toString());
    kwargs.insert("track_uuid", trackUuidStr);
    kwargs.insert("frame", atFrame);

    onSyncAction("mediaRemoved", kwargs);
}


void
RLSYNC_RoomController::onTrackAdded(QString trackUuidStr, QString trackName, QString trackType)
{
    if (_roomName == "")
    {
        RLP_LOG_DEBUG("Not in a sync session, skipping data sync")
        return;
    }

    efsPushTrack(trackUuidStr, trackName, trackType);
}


void
RLSYNC_RoomController::onMediaAdded(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount)
{

    if (_roomName == "")
    {
        RLP_LOG_DEBUG("Not in a sync session, skipping data sync")
        return;
    }

    // Register with Production Entity DB (thumbnail, title, etc)
    //
    dbPushMedia(node);

    efsPushMedia(node, trackUuidStr, frameNum, mediaFrameCount);
}


void
RLSYNC_RoomController::onStorageDataIOEvent(QVariantMap info)
{
    // RLP_LOG_DEBUG(info)

    emit dataIOEvent(info);
}


void
RLSYNC_RoomController::onNodeDataReady(QVariantMap info)
{
    if (_roomName == "")
    {
        // RLP_LOG_DEBUG("Not in a sync session, skipping data sync")
        return;
    }

    // RLP_LOG_INFO(info)

    QString dataType = info.value("data_type").toString();
    if (dataType == "thumbnail_ready")
    {
        RLP_LOG_DEBUG("THUMBNALI READY")
        
        QString mediaUuidStr = info.value("graph.uuid").toString();

        QVariantList argList;
        argList.append(mediaUuidStr);

        QVariant rresult = PY_Interp::call("revlens_prod.cmds.register_thumbnail", argList);
    }
}


void
RLSYNC_RoomController::efsPushTrack(QString trackUuidStr, QString trackName, QString trackType)
{
    if (!_doDataSync)
    {
        RLP_LOG_DEBUG("Data Sync off - skipping")
        return;
    }


    RLP_LOG_DEBUG(trackUuidStr << " " << trackName << " " << trackType)

    DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);
    QString trackDir = QString("%1/track/t_%2").arg(_dataRootDir).arg(trackUuidStr);

    QDir d;
    bool mkdresult = d.mkpath(trackDir);

    if (mkdresult)
    {
        RLP_LOG_DEBUG("OK:" << trackDir)
    } else
    {
        RLP_LOG_ERROR("Error creating dir:" << trackDir)
    }

    // bool eresult = _efsc->run_mk_dir(trackDir, trackUuidStr);

    RLP_LOG_DEBUG(trackDir)
}


bool
RLSYNC_RoomController::dbPushMedia(DS_NodePtr node)
{

    QVariantMap mdata = node->getProperties();
    QString mediaUuidStr = mdata.value("graph.uuid").toString(); // node->getProperty<QUuid>("graph.uuid").toString();

    RLP_LOG_DEBUG(mediaUuidStr);

    QVariantList argList;
    argList.append(mediaUuidStr);

    QVariant rresult = PY_Interp::call("revlens_prod.cmds.register_media", argList);

    return rresult.toBool();
}


void
RLSYNC_RoomController::efsPushMedia(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount)
{
    if (!_doDataSync)
    {
        RLP_LOG_DEBUG("Data Sync off - skipping")
        return;
    }
    
    // Register with Production Entity DB (thumbnail, title, etc)
    //
    // Should be done already?
    //
    // dbPushMedia(node);

    QDir d;

    QVariantMap mdata = node->getProperties();
    QString mediaUuidStr = mdata.value("graph.uuid").toString(); // node->getProperty<QUuid>("graph.uuid").toString();

    RLP_LOG_DEBUG(mediaUuidStr << " " << frameNum)

    QString mediaDir = QString("%1/media/m_%2/components").arg(_dataRootDir, mediaUuidStr);



    if (mdata.contains("media.video.components"))
    {
        QVariantList videoComponents = mdata.value("media.video.components").toList();
        QVariantList::iterator it;

        for (it=videoComponents.begin(); it != videoComponents.end(); ++it)
        {
            QString componentName = (*it).toString();

            RLP_LOG_DEBUG("getting source path for component: " << componentName)

            QString componentNs = QString("media.video.component.%1.path").arg(componentName);
            QString pathValue = mdata.value(componentNs).toString();

            QString componentDir = QString("%1/%2").arg(mediaDir, componentName);
            


            QString componentFilePath = QString("%1/data").arg(componentDir);

            QVariantList argList;
            argList.append(pathValue);
            argList.append(componentFilePath);
            argList.append(mediaUuidStr);

            QVariant rresult = PY_Interp::call("revlens_prod.cmds.copy_to_fs", argList);

            bool addResult = rresult.toBool();
            // copy syncronously?
            // bool addResult = false; // _efsc->run_mk_file(componentFilePath, true, mediaUuidStr, pathValue);
            if (addResult)
            {
                RLP_LOG_DEBUG("Add OK: " << componentFilePath)
            }
        }
    }
}


void
RLSYNC_RoomController::efsPushSession(QVariantMap sessionData)
{

    // RLP_LOG_DEBUG(sessionData)

    QStringList allTrackList = sessionData.value("track_list").value<QStringList>();
    QVariantMap allTrackData = sessionData.value("track_data").toMap();
    
    for (int tx = 0; tx != allTrackList.size(); ++tx) {

        QString trackUuidStr = allTrackList.at(tx);
        QVariantMap trackData = allTrackData.value(trackUuidStr).toMap();

        QString trackType = trackData.value("track_type").toString();
        QString trackName = trackData.value("name").toString();
        
        efsPushTrack(trackUuidStr, trackName, trackType);

        DS_TrackPtr track = _controller->session()->getTrackByUuid(trackUuidStr);

        QVariantMap trackMedia = trackData.value("media").toMap();
            
        QStringList mediaUuids = trackMedia.keys(); 
        QStringList::iterator mit;
        
        int mediaCount = 0;
        int mediaTotal = mediaUuids.size();

        for (mit = mediaUuids.begin(); mit != mediaUuids.end(); ++mit) {
            
            QString mediaUuidIn = *mit;
            QVariantMap mediaData = trackMedia.value(*mit).toMap();

            qlonglong frameNum = mediaData.value("frame").toLongLong();
            QVariantMap nodeProps = mediaData.value("properties").toMap();

            DS_NodePtr node = track->getNodeByFrame(frameNum);
            qlonglong mediaFrameCount = nodeProps.value("media.frame_count").toLongLong();

            RLP_LOG_DEBUG("Pushing media - " << mediaUuidIn << " " << frameNum << " " << mediaFrameCount << " track: " << trackUuidStr)

            dbPushMedia(node);
            efsPushMedia(node, trackUuidStr, frameNum, mediaFrameCount);
        }
    }
}


void
RLSYNC_RoomController::callFromClient(QString method, QVariantList args, QVariantMap kwargs)
{
    RLP_LOG_DEBUG(method)
    LOG_Logging::pprint(kwargs);

    if (method == "rooms.create_room")
    {

        QString contents = kwargs.value("contents").toString();

        RLP_LOG_DEBUG("create_room - contents: " << contents);

        if (contents == "current_session")
        {
            QVariantMap sessionData = _controller->session()->toJson(true).toVariantMap();

            QVariantList l;
            l.push_back(sessionData);
            l.push_back(true); // remove_paths
            QVariant pyresult = PY_Interp::call("revlens.media.sanitize_session", l);

            // QVariantMap result;
            if (pyresult.canConvert<QVariantMap>())
            {
                QVariantMap result = pyresult.value<QVariantMap>();

                kwargs.insert("session_data", result);

            } else
            {

                RLP_LOG_ERROR("Python processing of media input failed");

                kwargs.insert("session_data", sessionData);

            }
            
            efsPushSession(sessionData);

        }
        
        
    } else if (method == "load_room")
    {
         
        // QString roomName = kwargs.value("room_name").toString();
        // setRoomName(roomName);
         
        // RLP_LOG_DEBUG(roomName );
        
        
    } else if (method == "leave_room")
    {

        RLP_LOG_DEBUG("Leaving room " << _roomName);
        kwargs.insert("room_name", _roomName);

        QString msg = "Leave Room - ";
        msg += _roomName;

        setRoomName("");

        emitEventMessage(msg);
    
    } else if (method == "get_properties")
    {

        QVariantMap msg;
        QVariantMap msgKwargs;
        QVariantMap props;
        props.insert("room_name", _roomName);

        msgKwargs.insert("properties", props);

        msg.insert("method", "on_get_properties");
        msg.insert("kwargs", msgKwargs);
        
        emit sendCallToClient(msg);

        return;

    } else if (method == "delete_room")
    {

        QString roomName = kwargs.value("room_name").toString();
        RLP_LOG_DEBUG("delete_room: " << roomName);
    }


    _controller->getServerManager()->sendCallToService(method, args, kwargs, _skwargs);
}


void
RLSYNC_RoomController::callDirectFromClient(QString method, QVariantList args, QVariantMap kwargs)
{
    RLP_LOG_DEBUG(method << " " << args << " " << kwargs)

    QVariantMap payload;
    payload.insert("method", method);
    payload.insert("args", args);
    payload.insert("kwargs", kwargs);

    onRemoteCallReceived(payload);
}


void
RLSYNC_RoomController::onRemoteCallReceived(QVariantMap msgEnvelope)
{
    // RLP_LOG_DEBUG("")

    // RLP_LOG_DEBUG(msgEnvelope)

    QString estatus = msgEnvelope.value("status").toString();
    if (estatus != "OK")
    {
        // RLP_LOG_ERROR("error on incoming message")
        return;
    }

    QVariantMap msgObj = msgEnvelope.value("result").toMap();

    // RLP_LOG_DEBUG(msgObj)

    QString fromMethod;
    QString status = msgObj.value("status").toString();
    QVariantMap sessionkw = msgObj.value("session").toMap();

    if (status != "")
    {
        // return result from a call
        fromMethod = sessionkw.value("from_method").toString();

    } else
    {
        fromMethod = msgObj.value("method").toString();
    }

    RLP_LOG_DEBUG(fromMethod)

    QVariantMap mainkw = msgObj.value("kwargs").value<QVariantMap>();

    if (fromMethod == "create_room")
    {

        RLP_LOG_DEBUG("received create_room, emitting" );

        QVariantMap msg;
        QVariantMap kwargs;
        
        kwargs.insert("result", msgObj.value("result").value<QVariantList>());
        msg.insert("kwargs", kwargs);
        msg.insert("method", "on_create_room");
        
        emit sendCallToClient(msg);

    } else if (fromMethod == "delete_room")
    {

        RLP_LOG_DEBUG("received delete_room, emitting" );

        QVariantMap msg;
        QVariantMap kwargs;
        
        kwargs.insert("result", msgObj.value("result").value<QVariantList>());
        msg.insert("kwargs", kwargs);
        msg.insert("method", "on_delete_room");
        
        emit sendCallToClient(msg);

    } else if (fromMethod == "rename_room")
    {

        RLP_LOG_DEBUG("received rename_room, emitting" );

        QVariantMap msg;
        QVariantMap kwargs;
        
        kwargs.insert("result", msgObj.value("result").value<QVariantList>());
        msg.insert("kwargs", kwargs);
        msg.insert("method", "on_rename_room");
        
        emit sendCallToClient(msg);

    } else if (fromMethod == "leave_room")
    {

        RLP_LOG_DEBUG("received leave_room, emitting");

        QVariantMap msg;
        QVariantMap kwargs;

        msg.insert("kwargs", msgObj);
        msg.insert("method", "on_leave_room");

        emit sendCallToClient(msg);

    } else if (fromMethod == "get_clients")
    {
        RLP_LOG_DEBUG("RECEIVED get_clients")

        QVariantMap msg;
        msg.insert("kwargs", msgObj);
        msg.insert("method", "on_get_clients");

        emit sendCallToClient(msg);
    }


    QString fromIdent = sessionkw.value("client.ident").toString();
    QString myIdent = clientIdent();

    // RLP_LOG_DEBUG("From Ident:" << fromIdent << "My Ident:" << myIdent)
    //
    // if (fromIdent == myIdent)
    // {
    //     RLP_LOG_WARN("message came from this process, skipping");
    //     return;
    // }

    QString fromClient = sessionkw.value("client.user").toString();


    //int fromPid = sessionkw.value("client.pid").toInt();
    //
    // if ((fromClient == _env.value("USER")) && (fromPid == _appPid))
    // {
    //     RLP_LOG_WARN("message came from this process, skipping");
    //     return;
    // }
    
    // RLP_LOG_DEBUG("from_method: " << fromMethod);
    
    if (fromMethod == "client_cmd")
    {

        // QVariantMap kkwargs = skwargs.value("kwargs").toMap();

        QString method = mainkw.value("from_method").toString();
        QVariantMap akwargs = mainkw.value("action_kwargs").toMap();

        handleClientCmd(method, akwargs, fromClient);


    } else if (fromMethod == "callFromClient")
    {

        // iPad - routed through locally running websocket server,
        // which calls handleRemoteCall(), which emits remoteCall(),
        // which is connected to this slot.. phew
        //
        RLP_LOG_DEBUG("GOT callFromClient" );

        QString method = mainkw.value("method").toString();
        QVariantList cargs = mainkw.value("args").value<QVariantList>();
        QVariantMap ckwargs = mainkw.value("kwargs").toMap();

        callFromClient(method, cargs, ckwargs);


    } else if (fromMethod == "get_rooms")
    {
        
        RLP_LOG_DEBUG("received get_rooms, emitting" );

        QVariantMap msg;
        QVariantMap kwargs;
        
        kwargs.insert("result", msgObj.value("result").value<QVariantList>());
        msg.insert("kwargs", kwargs);
        msg.insert("method", "on_get_rooms");
        
        emit sendCallToClient(msg);
        
    } else if (fromMethod == "load_room")
    {
        
        QVariantMap result = msgObj.value("result").toMap();

        // RLP_LOG_DEBUG(result)

        QVariantMap sessionData = result.value("session").toMap();
        QString roomName = result.value("room_name").toString();
        QString roomUuidStr = result.value("room_uuid").toString();
        bool sync = sessionkw.value("sync").toBool();

        RLP_LOG_DEBUG("running load_room - " << roomName );
        
        emitEventMessage(QString("Loading ") + roomName + " ..", 0);

        int numLoaded = _controller->getMediaManager()->loadSessionObject(sessionData, false);

        if (sync)
        {
            RLP_LOG_INFO("setting up sync to room " << roomName);
            setRoomName(roomName);
            setRoomUuid(roomUuidStr);
        }
        
        _controller->getVideoManager()->clearAll();
        _controller->gotoFrame(1, true, true, false);

        emitEventMessage(QString("Loaded ") + QString("%1").arg(numLoaded) + " items");

        QVariantMap msg;
        QVariantMap kwargs;
        kwargs.insert("room_name", roomName);
        kwargs.insert("room_uuid", roomUuidStr);

        msg.insert("kwargs", kwargs);
        msg.insert("method", "on_room_loaded");
        
        emit sendCallToClient(msg);

    } else if (fromMethod == "join_room")
    {

        QString clientName = sessionkw.value("client.user").toString();
        int clientPid = sessionkw.value("client.pid").toInt();
        
        if ((clientName == _skwargs.value("client.user").toString()) && (clientPid == _skwargs.value("client.pid").toInt())) {

            QString roomName = mainkw.value("room_name").toString();
            QString roomUuidStr = mainkw.value("room_uuid").toString();

            setRoomName(roomName);
            setRoomUuid(roomUuidStr);

            UTIL_AppGlobals::instance()->globalsPtr()->insert("room_uuid", roomUuidStr);
        }
        

    } else if (fromMethod == "update_room_session")
    {

        QVariantList args = msgObj.value("args").value<QVariantList>();
        handleRoomUpdateCmd(args, mainkw, sessionkw);


    } else
    {
        RLP_LOG_VERBOSE("no handler" );
    }
}


void
RLSYNC_RoomController::emitEventMessage(QString message, int duration)
{
    QVariantMap evtInfo;
    evtInfo.insert("message", message);
    evtInfo.insert("duration", duration);

    emit eventMessage(evtInfo);
    QApplication::instance()->processEvents();
}


void
RLSYNC_RoomController::handleClientCmd(QString method, QVariantMap kwargs, QString fromClient)
{
    // RLP_LOG_DEBUG(method)

    // For clarity only
    bool noEmitAction = false;
    
    if (method == "play")
    {
        _controller->play(noEmitAction);

    } else if (method == "pause")
    {

        long frame = kwargs.value("frame").toLongLong();
        _controller->pause(frame, noEmitAction);
        
    } else if (method == "onUpdateToFrame")
    {

        long frame = kwargs.value("frame").toLongLong();

        // _controller->gotoFrame(frame, false, true, noEmitAction);

        for (auto vit : GUI_TL2_Controller::instance()->views())
        {
            vit->syncUpdateImmediate(frame);
        }

        _controller->getVideoManager()->updateToFrame(frame, false, true);
        
    } else if (method == "onScrubStart")
    {

        long frame = kwargs.value("frame").toLongLong();
        _controller->getVideoManager()->onScrubStart(frame);
    
    } else if (method == "onScrubEnd")
    {

        long frame = kwargs.value("frame").toLongLong();
        // _controller->gotoFrame(frame, true, false, noEmitAction);
        
        _controller->getVideoManager()->onScrubEnd(frame);
        _controller->getAudioManager()->onScrubEnd(frame);
        
    } else if (method == "gotoFrame")
    {

        long frame = kwargs.value("frame").toLongLong();
        bool doRecenter = kwargs.value("recenter").toBool();

        _controller->gotoFrame(frame, doRecenter, true, noEmitAction);

    } else if (method == "setInFrame")
    {
        
        long frame = kwargs.value("frame").toLongLong();
        _controller->setInFrame(frame, false, false);

        for (auto vit : GUI_TL2_Controller::instance()->views())
        {
            vit->setInFrame(frame, noEmitAction);
            vit->update();
        }
        

    } else if (method == "setOutFrame")
    {

        long frame = kwargs.value("frame").toLongLong();

        _controller->setOutFrame(frame, false, false);
        
        for (auto vit : GUI_TL2_Controller::instance()->views()) {
            vit->setOutFrame(frame, noEmitAction);
            vit->update();
        }


    } else if ((method == "drawStart") ||
               (method == "draw") ||
               (method == "drawEnd") ||
               (method == "hoverMove"))
        {

        DS_EventPlugin* evp = _controller->getEventPluginByName("RlpAnnotation");
        if (evp != nullptr)
        {
            evp->handleSyncAction(method, kwargs);
        } else
        {
            RLP_LOG_WARN("ANNOTATION PLUGIN NOT FOUND, CANNOT DISPATCH EVENT")
        }
    }
    else if (method == "updatePermissions")
    {
        QVariantMap msg;
        msg.insert("method", method);
        msg.insert("kwargs", kwargs);
        emit sendCallToClient(msg);
    }
}


void
RLSYNC_RoomController::handleRoomUpdateCmd(QVariantList args, QVariantMap kwargs, QVariantMap sessionKwargs)
{
    QString method = kwargs.value("action").toString();
    QString fromClient = sessionKwargs.value("client.user").toString();

    RLP_LOG_DEBUG(method);

    if (method == "addTrack")
    {

        RLP_LOG_DEBUG("ADD TRACK" );

        QString trackName = kwargs.value("track_name").toString();
        QString trackType = kwargs.value("track_type").toString();
        QUuid trackUuid(kwargs.value("track_uuid").toString());
        
        _controller->session()->addTrack(trackName, trackType, trackUuid, fromClient, false);

    } else if (method == "deleteTrack")
    {

        QString trackUuidStr(kwargs.value("track_uuid").toString());
        
        _controller->session()->deleteTrack(trackUuidStr, false);

    } else if (method == "insert_media_at_frame")
    {

        RLP_LOG_DEBUG("insert_media_at_frame");

        QVariantMap mediaIn = kwargs.value("media_in").toMap();
        int trackIdx = kwargs.value("track_idx").toInt();
        qlonglong frameNum = kwargs.value("frame").toLongLong();

        QVariantList argList;
        argList.append(mediaIn);
        argList.append(trackIdx);
        argList.append(frameNum);
        argList.append(false); // emit_sync_action - no
        argList.append(true); // identify - to trigger download / build of best media based on components per receiver

        PY_Interp::call("revlens.media.insert_media_at_frame", argList);

    } else if (method == "load_media_list")
    {

        RLP_LOG_DEBUG("load_media_list");

        QVariantMap mediaListWrapper = kwargs.value("media_input_list").toMap();
        QVariant mediaInputList = mediaListWrapper.value("media").value<QVariantList>();
        int trackIdx = kwargs.value("track_idx").toInt();
        qlonglong startFrame = kwargs.value("start_frame").toLongLong();
        bool clear = kwargs.value("clear").toBool();

        QVariantList argList;
        argList.append(mediaInputList);
        argList.append(trackIdx);
        argList.append(startFrame);
        argList.append(clear);
        argList.append(false); // emit_sync_action - no

        PY_Interp::call("revlens.media.load_media_list", argList);

    } else if (method == "load_previous_shot")
    {

        RLP_LOG_DEBUG("load_previous_shot");

        QString trackUuidStr = kwargs.value("track_uuid").toString();
        qlonglong fromFrame = kwargs.value("from_frame").toLongLong();
        QString dept = kwargs.value("department").toString();

        QVariantList argList;
        argList.append(trackUuidStr);
        argList.append(fromFrame);
        argList.append(dept);

        PY_Interp::call(
            "rlplug_review.cmds.shot._on_load_previous_shot",
            argList
        );

    } else if (method == "setSession")
    {
        QVariantMap sessionData = args[0].toMap();
        int numLoaded = _controller->getMediaManager()->loadSessionObject(sessionData, false);

    } else if ((method == "holdFrame") ||
               (method == "releaseFrame") ||
               (method == "clearAnnotations"))
    {

        QVariantMap info = kwargs.value("update_info").toMap();
        DS_EventPlugin* evp = _controller->getEventPluginByName("RlpAnnotation");
        if (evp != nullptr) {
            evp->handleSyncAction(method, info);
        }
    } else if (method == "message")
    {
        RLP_LOG_DEBUG("Got message")
        RLP_LOG_DEBUG(kwargs)

        emit sendCallToClient(kwargs);

    } else
    {
        RLP_LOG_WARN("no handler - " << method);
    }

}


void
RLSYNC_RoomController::onSyncAction(QString action, QVariantMap actionKwargs)
{
    // RLP_LOG_DEBUG(_roomName << " - " << action);

    if (_roomName == "")
    {
        // RLP_LOG_WARN("no current room, cannot send sync action");
        return;
    }

    // RLP_LOG_DEBUG(_roomName << " - " << action);

    QVariantMap kwargs;
    kwargs.insert("action", action);
    kwargs.insert("action_kwargs", actionKwargs);

    QVariantList args;

    // RLP_LOG_DEBUG("sending " << action );

    _controller->getServerManager()->sendCallToService("rooms.client_cmd", args, kwargs, _skwargs);

}