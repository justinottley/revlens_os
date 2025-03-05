//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_CNTRL_MEDIAMANAGER_H
#define REVLENS_CNTRL_MEDIAMANAGER_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/MEDIA/Factory.h"

class CNTRL_MainController;


class REVLENS_CNTRL_API CNTRL_MediaManager : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

public:
    CNTRL_MediaManager(CNTRL_MainController* controller);

signals:
    void nodeDataReady(QVariantMap info); // re-emitted from node graph
    void sessionSaved(QString path);

public:
    QVariantMap identifyMedia(QString mediaInput);

public slots:

    DS_SessionPtr createSession(QString name);
    DS_SessionPtr getSession(QString name);

    MEDIA_Factory* getMediaFactory() { return _mediaFactory; }
    DS_NodePtr onMediaCreated(DS_NodePtr node, bool avOnly);

    void loadMediaList(QVariantList inputList, int trackIdx=0, QString trackName="");

    // Session
    //

    QJsonObject loadJsonFile(QString filePath);
    int loadSessionFile(QString sessionFilePath);
    int loadSessionObject(QVariantMap sessionObj, bool emitAction=true);
    void saveSessionFile(QString sessionFilePath, bool emitEvent=false);

    QVariantMap identifyMedia(QVariantMap mediaInput);


    // Order that tracks get assigned a track number (index)
    // allows grouping tracks by type
    //
    bool setTrackTypeIndexOrder(QVariantList indexOrder);
    QVariantList trackTypeIndexOrder() { return _trackTypeIndexOrder; }

    void setTrackIndexOrderPolicy(DS_Session::TrackIndexOrderPolicy policy);
    DS_Session::TrackIndexOrderPolicy trackIndexOrderPolicy() { return _trackIndexOrderPolicy; }

    // push track type ordering policy (and other?) to sessions
    void updateSessions();


public slots:

    void onMediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame);
    void onItemSelectionChanged(QVariantMap mediaInfoIn);
    void onTrackCleared(QString trackUuidStr);
    void onTrackVisibilityChanged(int idx, bool isEnabled);
    void onSessionSyncAction(QString action, QVariantMap kwargs);
    void onNodeDataReady(QVariantMap info);

signals:
    void itemSelected(QVariantMap mediaInfoIn);

private:

    void forceRefreshVideo();

private:

    CNTRL_MainController* _controller;

    MEDIA_Factory* _mediaFactory;

    // QHash<QString, DS_NodePtr> _media;

    QHash<QString, DS_SessionPtr> _sessionMap;

    // precedence order search path / order for how to assign / set a track index when creating a new track
    // should be list of ints
    QVariantList _trackTypeIndexOrder;
    DS_Session::TrackIndexOrderPolicy _trackIndexOrderPolicy;

};

#endif

