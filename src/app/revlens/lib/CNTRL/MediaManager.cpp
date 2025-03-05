//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/MediaManager.h"

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/Track.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/PluginManager.h"
#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpRevlens/CNTRL/Video.h"


RLP_SETUP_LOGGER(revlens, CNTRL, MediaManager)

CNTRL_MediaManager::CNTRL_MediaManager(CNTRL_MainController* controller) :
    _controller(controller),
    _mediaFactory(MEDIA_Factory::instance(DS_ControllerPtr(controller)))
{

    QVariantList mediaTrackTypeList;
    mediaTrackTypeList.append(TRACK_TYPE_MEDIA);

    setTrackTypeIndexOrder(mediaTrackTypeList);
    setTrackIndexOrderPolicy(DS_Session::TRACK_IDX_BY_TYPE);

    // Will reorder tracks in timeline
    // setTrackIndexOrderPolicy(DS_Session::TRACK_IDX_AS_IS);
    
    createSession("Default");

    connect(
        _mediaFactory,
        &MEDIA_Factory::mediaCreated,
        this,
        &CNTRL_MediaManager::onMediaCreated
    );
}


bool
CNTRL_MediaManager::setTrackTypeIndexOrder(QVariantList orderList)
{
    RLP_LOG_DEBUG(orderList);

    _trackTypeIndexOrder = orderList;

    updateSessions();

    return true;
}


void
CNTRL_MediaManager::setTrackIndexOrderPolicy(DS_Session::TrackIndexOrderPolicy policy)
{
    RLP_LOG_DEBUG((int)policy);

    _trackIndexOrderPolicy = policy;

    updateSessions();
}


void
CNTRL_MediaManager::updateSessions()
{
    RLP_LOG_DEBUG("");

    QHash<QString, DS_SessionPtr>::iterator it;

    for (it = _sessionMap.begin(); it != _sessionMap.end(); ++it) {

        RLP_LOG_DEBUG("updating session: " << it.key());

        it.value()->setTrackTypeIndexOrder(_trackTypeIndexOrder);
        it.value()->setTrackIndexOrderPolicy(_trackIndexOrderPolicy);
    }
}

// -------------

DS_SessionPtr
CNTRL_MediaManager::createSession(QString name)
{
    RLP_LOG_DEBUG(name);
    
    DS_SessionPtr result;
    
    if (_sessionMap.contains(name)) {
        RLP_LOG_DEBUG("ERROR: session " << name << " already exists");

    } else {
        
        DS_SessionPtr session(new DS_Session(name));
        
        session->setTrackTypeIndexOrder(_trackTypeIndexOrder);
        session->setTrackIndexOrderPolicy(_trackIndexOrderPolicy);

        connect(
            session.get(),
            SIGNAL(trackCleared(QString)),
            this,
            SLOT(onTrackCleared(QString)));
        
        
        connect(
            session.get(),
            SIGNAL(trackVisibilityChanged(int, bool)),
            this,
            SLOT(onTrackVisibilityChanged(int, bool)));
        
        connect(
            session.get(),
            SIGNAL(syncAction(QString, QVariantMap)),
            this,
            SLOT(onSessionSyncAction(QString, QVariantMap)));
        
        connect(
            session.get(),
            SIGNAL(mediaRemoved(DS_NodePtr, QString, qlonglong)),
            this,
            SLOT(onMediaRemoved(DS_NodePtr, QString, qlonglong))
        );

        _sessionMap[name] = session;
        
        result = session;
    }
    
    return result;
}


DS_SessionPtr
CNTRL_MediaManager::getSession(QString name)
{
    RLP_LOG_DEBUG(name);
    
    DS_SessionPtr result;
    if (_sessionMap.contains(name)) {
        result = _sessionMap[name];
    }
    
    return result;
}


// --------------------


QVariantMap
CNTRL_MediaManager::identifyMedia(QVariantMap mediaInput)
{
    return _mediaFactory->identifyMedia(mediaInput);
}


QVariantMap
CNTRL_MediaManager::identifyMedia(QString mediaInput)
{
    return _mediaFactory->identifyMediaItem(mediaInput);
}


DS_NodePtr
CNTRL_MediaManager::onMediaCreated(DS_NodePtr node, bool avOnly)
{
    RLP_LOG_DEBUG("")

    if (node != nullptr)
    {
        connect(
            node->graph(),
            &DS_Graph::dataReady,
            this,
            &CNTRL_MediaManager::onNodeDataReady
        );

    }

    return node;
}


void
CNTRL_MediaManager::onNodeDataReady(QVariantMap info)
{
    // RLP_LOG_DEBUG(info)

    emit nodeDataReady(info);
}
// --------------------


void
CNTRL_MediaManager::forceRefreshVideo()
{
    RLP_LOG_DEBUG("");
    
    CNTRL_Video* vm = _controller->getVideoManager();
    
    vm->clearAll(false);
    vm->updateToFrameAsync(vm->currentFrameNum(), true, true);
    // vm->recenter(vm->currentFrameNum());
    
}


void
CNTRL_MediaManager::onMediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame)
{
    RLP_LOG_DEBUG("");

    forceRefreshVideo();
}


void
CNTRL_MediaManager::onTrackCleared(QString trackUuidStr)
{
    RLP_LOG_DEBUG(trackUuidStr);
    
    forceRefreshVideo();
}


void
CNTRL_MediaManager::onTrackVisibilityChanged(int idx, bool isEnabled)
{
    RLP_LOG_DEBUG(idx << " " << isEnabled);
    
    forceRefreshVideo();
}


void
CNTRL_MediaManager::onItemSelectionChanged(QVariantMap mediaInfoIn)
{
    RLP_LOG_DEBUG("")

    emit itemSelected(mediaInfoIn);
}


void
CNTRL_MediaManager::onSessionSyncAction(QString action, QVariantMap kwargs)
{
    RLP_LOG_DEBUG(action);

    _controller->emitSyncAction(action, kwargs);
}


void
CNTRL_MediaManager::loadMediaList(QVariantList mediaInputList, int trackIdx, QString trackName)
{
    // MEDIA_Plugin* dloader = _mediaFactory->getMediaPluginByName("DeferredLoad");
    // RLP_LOG_DEBUG(dloader);

    DS_TrackPtr track = nullptr;
    if (trackIdx == -1)
    {
        track = _controller->session()->addTrackByType("Media");
        track->setName(trackName);

    } else
    {
        track = _controller->session()->getTrackByIndex(trackIdx);
    }

    if (track == nullptr)
    {
        RLP_LOG_ERROR("Invalid track index:" << trackIdx)
        return;
    }

    _mediaFactory->loadMediaList(mediaInputList, track.get());

    QVariantMap ninfo;
    ninfo.insert("data_type", "media_list");
    emit nodeDataReady(ninfo);
}


QJsonObject
CNTRL_MediaManager::loadJsonFile(QString filePath)
{   
    QJsonObject noResult;
    
    QFile jsonFile(filePath);
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        RLP_LOG_DEBUG("Cannot open " << filePath);
        return noResult;
    }
    
    
    QString jsonIn = QTextStream(&jsonFile).readAll();
    QByteArray jsonData;
    jsonData.append(jsonIn.toLatin1());
    
    // RLP_LOG_DEBUG(jsonIn.toStdString().c_str());
    // RLP_LOG_DEBUG(jsonIn.size());
    
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);
    
    
    if (error.error == QJsonParseError::NoError) {
        
        RLP_LOG_DEBUG("load success");
        
        QJsonObject jsonObj = jsonDoc.object();
        
        return jsonObj;

    } else {
        
        RLP_LOG_DEBUG("Error reading json data");
        RLP_LOG_DEBUG(error.errorString());
    }
    
    return noResult;
    
    /*
    
    
    if (media == nullptr) {
        RLP_LOG_DEBUG("COULD NOT CREATE MEDIA FROM " << description.toStdString().c_str());
        
    } else {
        
    }
    */
    
    
    
    
    // return media;
}


int
CNTRL_MediaManager::loadSessionObject(QVariantMap sessionIn, bool emitAction)
{
    RLP_LOG_DEBUG("");
    
    int numLoaded = 0;

    QString sessionUuidStr = sessionIn.value("session_uuid").toString();

    _controller->session()->clear(sessionUuidStr);
    
    QVariantMap globals = UTIL_AppGlobals::instance()->globals();
    QString currOwner = globals.value("username").toString();

    QStringList allTrackList = sessionIn.value("track_list").value<QStringList>();
    QVariantMap allTrackData = sessionIn.value("track_data").toMap();
    
    for (int tx = 0; tx != allTrackList.size(); ++tx) {

        QString trackUuidStr = allTrackList.at(tx);
        QVariantMap trackData = allTrackData.value(trackUuidStr).toMap();

        QString trackType = trackData.value("track_type").toString();
        QString trackName = trackData.value("name").toString();
        QString trackOwner = trackData.value("owner").toString();
        QUuid trackUuid = QUuid(trackUuidStr);

        if (trackOwner != currOwner)
        {
            RLP_LOG_DEBUG("loading track from owner " << trackOwner);
        }

        RLP_LOG_DEBUG(trackName << " type: " << trackType);
        
        DS_TrackPtr track = _controller->session()->addTrack(trackName, trackType, trackUuid, trackOwner, emitAction);
        if (track != nullptr)
        {
            QVariantMap allMediaData = trackData.value("media").toMap();
            _mediaFactory->loadMediaList(allMediaData, track.get());
            // numLoaded += track->load(trackData, _mediaFactory);
            // _mediaFactory->loadMediaList(track
        } else
        {
            RLP_LOG_ERROR("Invalid track returned for" << trackName << trackType << trackUuid)
        }
        
    }

    _controller->setSession(_controller->session()->name());

    return numLoaded;
}


int
CNTRL_MediaManager::loadSessionFile(QString filePath)
{
    RLP_LOG_DEBUG(filePath);
    
    QJsonObject jsonSession = loadJsonFile(filePath);
    QVariantMap qvmSession = jsonSession.toVariantMap();
    int numLoaded = loadSessionObject(qvmSession);

    QVariantMap fp;
    fp.insert("session.path", filePath);

    _controller->session()->updateProperties(fp);

    UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();
    QVariantMap* glptr = ag->globalsPtr();
    glptr->insert("session.path", filePath);


    return numLoaded;
}


void
CNTRL_MediaManager::saveSessionFile(QString filePath, bool emitEvent)
{
    QJsonObject session = _controller->session()->toJson(true);
    QJsonDocument doc(session);

    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly)) {

        QTextStream stream(&f);
        stream << doc.toJson();
        // f.write(doc.toJson());
        f.flush();
        f.close();

        RLP_LOG_DEBUG("wrote " << filePath);

        if (emitEvent)
        {
            RLP_LOG_DEBUG("EMIT SESSION SAVWED")
            emit sessionSaved(filePath);
        }
    }

}

