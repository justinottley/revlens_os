//
//  Copyright (c) 2014-2021 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLSYNC_ROOMHANDLER_H
#define EXTREVLENS_RLSYNC_ROOMHANDLER_H

#include "RlpExtrevlens/RLSYNC/Global.h"
#include "RlpExtrevlens/RLSYNC/CmdFilter.h"

#include "RlpRevlens/DS/EventPlugin.h"
#include "RlpRevlens/CNTRL/MainController.h"


class EXTREVLENS_RLSYNC_API RLSYNC_RoomController : public DS_EventPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER
    
public:
    RLSYNC_RoomController();

    bool setMainController(CNTRL_MainController* controller);
    DS_EventPlugin* duplicate() { return this; }
    bool canDuplicate() { return false; }

signals:
    void dataIOEvent(QVariantMap info);

public slots: // EFS

    void initEdbFs();

    void efsPushTrack(QString trackUuidStr, QString trackName, QString trackType);
    void efsPushMedia(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount);
    void efsPushSession(QVariantMap sessionData);

public slots: // DB

    bool dbPushMedia(DS_NodePtr node);

public slots: // connections with the main controller

    void onSessionCleared();
    void onMediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame);

    void onRemoteCallReceived(QVariantMap msgObj);
    void onSyncAction(QString method, QVariantMap kwargs);

    void onTrackAdded(QString trackUuidStr, QString trackName, QString trackType);
    void onMediaAdded(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount);

    void onStorageDataIOEvent(QVariantMap info);
    void onNodeDataReady(QVariantMap info);

public slots: // real-time sync

    bool inSyncSession() { return _syncOn; }

    void handleClientCmd(QString method, QVariantMap kwargs, QString fromClient);
    void handleRoomUpdateCmd(QVariantList args, QVariantMap kwargs, QVariantMap sessionKwargs);


    void callFromClient(QString method, QVariantList args, QVariantMap kwargs);
    void callDirectFromClient(QString method, QVariantList args, QVariantMap kwargs);
    // QVariantMap properties() { return _properties; }

    // void insertProperty(QString key, QVariant value) { _properties.insert(key, value); }

    void setRoomName(QString roomName);
    QString currentRoom() { return _roomName; }
    QString roomName() { return _roomName; }
    QString clientIdent();

    void setRoomUuid(QString uuidStr);
    QString currentRoomUuid() { return _roomUuid; }
    QString currentUser() { return _currentUser; }

    QVariantMap skwargs() { return _skwargs; }

    void leaveRoom();

    void emitEventMessage(QString message, int duration=1400);

    RLSYNC_CmdFilter* cmdFilter() { return _cmdFilter; }

signals:
    
    void sendCallToClient(QVariantMap data);
    void eventMessage(QVariantMap evtInfo);

private:
    bool _syncOn;

    RLSYNC_CmdFilter* _cmdFilter;

    int _appPid;
    QString _currentUser;

    QString _roomName;
    QString _roomUuid;

    QVariantMap _skwargs; // rpc session kwargs

    QString _dataRootDir;

    bool _doDataSync;
};

#endif