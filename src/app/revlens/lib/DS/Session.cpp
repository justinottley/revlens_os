//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpCore/UTIL/AppGlobals.h"

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/Session.h"

const int DS_Session::VERSION = 1;

RLP_SETUP_LOGGER(revlens, DS, Session)

DS_Session::DS_Session(QString name):
    DS_Session(name, QUuid::createUuid())
{
    _properties.clear();
}


DS_Session::DS_Session(QString name, QUuid uuid):
    _name(name),
    _uuid(uuid),
    _frameCount(1),
    _inFrame(1),
    _outFrame(1)
{
    _trackMap.clear();
    _trackList.clear();
    
    _trackUuidMap.clear();
    _trackPlugins.clear();
    _nodeUuidMap.clear();

    // initCacheDir();
    registerTrackFactoryPlugin("Media", new DS_TrackFactoryPlugin("Media"));
}


bool
DS_Session::initCacheDir()
{
    bool result = false;

    // QString tmpDir = // QProcessEnvironment::systemEnvironment().value("TMPDIR");
    QDir cacheDir = QDir(QDir::tempPath() + "/revlens/session/" + _uuid.toString());
    if (!cacheDir.exists())
    {
        RLP_LOG_DEBUG("Creating cache dir: " << cacheDir.path())
        result = cacheDir.mkpath(cacheDir.path());
        if (result) {
            cacheDir.mkdir("track");
        }
    }

    _cacheDir = cacheDir;

    RLP_LOG_DEBUG(_cacheDir.path());
    

    UTIL_AppGlobals* ag = UTIL_AppGlobals::instance();
    if (ag->globalsPtr()->contains("site_slug"))
    {
        QString siteSlug = ag->globalsPtr()->value("site_slug").toString();

        QDir siteCacheDir = QDir(QDir::tempPath() + "/revlens/" + siteSlug);
        if (!siteCacheDir.exists())
        {
            RLP_LOG_DEBUG("Creating Site cache dir: " << siteCacheDir.path())
            result = siteCacheDir.mkpath(siteCacheDir.path());
            if (result)
            {
                siteCacheDir.mkdir("media");
            }
        }

        _siteCacheDir = siteCacheDir;
    } else 
    {
        RLP_LOG_WARN("No site info, using session cache dir for site cache dir")

        _siteCacheDir = _cacheDir;
    }
    

    return result;
}


bool
DS_Session::registerTrackFactoryPlugin(QString trackType, DS_TrackFactoryPlugin* plugin)
{
    RLP_LOG_DEBUG(trackType);
    
    _trackPlugins.insert(trackType, plugin);
    return true;
}


bool
DS_Session::setTrackTypeIndexOrder(QVariantList orderList)
{
    RLP_LOG_DEBUG("");

    _trackTypeIndexOrder = orderList;
    return true;
}


void
DS_Session::setTrackIndexOrderPolicy(DS_Session::TrackIndexOrderPolicy policy)
{
    RLP_LOG_DEBUG((int)policy);

    _trackIndexOrderPolicy = policy;
}


QString
DS_Session::cacheDir(QString subdir)
{
    QString result = _cacheDir.path();
    if (subdir != "")
    {
        result = QString("%1/%2").arg(result, subdir);
    }
    
    return result;
}


QString

DS_Session::siteCacheDir()
{
    return _siteCacheDir.path();
}


DS_TrackPtr
DS_Session::addTrack(QString name, QString trackTypeName, QUuid uuid, QString owner, bool emitSyncAction)
{
    RLP_LOG_DEBUG(
        name << " " << trackTypeName << " " 
        << uuid.toString() << " owner: " << owner);

    if (!_trackPlugins.contains(trackTypeName))
    {
        RLP_LOG_ERROR("No plugin available for track type" << trackTypeName << ", skipping")
        return nullptr;
    }

    DS_TrackFactoryPlugin* plugin = _trackPlugins.value(trackTypeName);

    int trackType = plugin->trackType();
    int nextTrackIndex = getNextTrackIndex(plugin->trackType());
    // int nextTrackIndex = _trackList.size();

    RLP_LOG_DEBUG("Creating track using plugin " << plugin->name());

    DS_TrackPtr track = plugin->runCreateTrack(this, nextTrackIndex, uuid);

    track->setTrackTypeName(trackTypeName);
    // DS_TrackPtr(new DS_Track(this, trackIdx));

    if (owner == "") {
        QVariantMap* globals = UTIL_AppGlobals::instance()->globalsPtr();
        owner = globals->value("username").toString();
    }
    
    track->setOwner(owner);

    if (name == "") {
        name = QString("%1 %2").arg(trackTypeName).arg(_trackMap.value(trackType).size() + 1);
    }

    track->setName(name);
    
    connect(
        track.get(),
        SIGNAL(mediaAdded(DS_NodePtr, QString, qlonglong, qlonglong)),
        this,
        SLOT(onMediaAdded(DS_NodePtr, QString, qlonglong, qlonglong))
    );
    
    connect(
        track.get(),
        SIGNAL(mediaRemoved(DS_NodePtr, QString, qlonglong)),
        this,
        SLOT(onMediaRemoved(DS_NodePtr, QString, qlonglong))
    );
    
    connect(
        track.get(),
        &DS_Track::trackDataChanged,
        this,
        &DS_Session::onTrackDataChanged
    );


    _trackList.insert(nextTrackIndex, track);
    
    // Reset everybody's track index
    //
    QList<DS_TrackPtr>::iterator it;
    for (int tx = 0; tx != _trackList.size(); ++tx) {
        _trackList.at(tx)->setTrackIndex(tx);
    }


    if (!_trackMap.contains(trackType)) {
        QList<DS_TrackPtr> trackTypeList;
        trackTypeList.append(track);
        _trackMap.insert(trackType, trackTypeList);


    } else {
        QList<DS_TrackPtr> trackTypeList = _trackMap.value(trackType);
        trackTypeList.append(track);

        _trackMap.insert(trackType, trackTypeList);
    }


    _trackUuidMap.insert(track->uuid().toString(), track);
    
    emit trackAdded(track->uuid().toString(), track->name(), trackTypeName);
    
    if (emitSyncAction) {

        QVariantMap kwargs;

        kwargs.insert("track_uuid", track->uuid().toString());
        kwargs.insert("track_name", track->name());
        kwargs.insert("track_type", trackTypeName);
        
        // QVariantMap skwargs;
        // skwargs.insert("update_info", kwargs);
        emit syncAction("addTrack", kwargs);
    }

    return track;
}


int
DS_Session::numTracks()
{
    int result = 0;

    QHash<int, QList<DS_TrackPtr> >::iterator it;
    for (it = _trackMap.begin(); it != _trackMap.end(); ++it) {
        result += it.value().size();
    }

    return result;
}


int
DS_Session::getNextTrackIndex(int inTrackType)
{
    if (_trackIndexOrderPolicy == TRACK_IDX_AS_IS)
    {
        return numTracks();

    } else
    {
        int nextIdx = 0;

        QVariantList::iterator it;
        for (it = _trackTypeIndexOrder.begin(); it != _trackTypeIndexOrder.end(); ++it) {
            int trackType = it->toInt();

            // RLP_LOG_DEBUG("iterating on track type " << trackType);

            QList<DS_TrackPtr> trackList = _trackMap.value(trackType);
            nextIdx += trackList.size();
            if (trackType == inTrackType) {
                break;                
            }
        }
        
        return nextIdx;
    }
}


DS_TrackPtr
DS_Session::getTrackByIndex(int idx)
{
    DS_TrackPtr result = nullptr;
    
    if ((idx >= 0) && (_trackList.size() > idx)) {
        result = _trackList.at(idx);
    }
    
    return result;
}


DS_TrackPtr
DS_Session::getTrackByUuid(QString uuidStr)
{
    DS_TrackPtr result = nullptr;

    if (_trackUuidMap.contains(uuidStr)) {
        result = _trackUuidMap.value(uuidStr);
    }

    return result;
}


bool
DS_Session::clearTrack(int idx)
{
    bool result = false;
    
    if ((idx >= 0) && (_trackList.size() > idx)) {
        
        RLP_LOG_DEBUG("" << idx);
        QString trackUuidStr = _trackList.at(idx)->uuid().toString();
        _trackList.at(idx)->clear();
        
        result = true;
        emit trackCleared(trackUuidStr);
    }
    
    return result;
}


bool
DS_Session::deleteTrack(QString trackUuidStr, bool emitSyncAction)
{
    RLP_LOG_DEBUG(trackUuidStr);
    
    bool result = false;

    if (_trackUuidMap.contains(trackUuidStr)) {
        DS_TrackPtr track = _trackUuidMap.value(trackUuidStr);

        RLP_LOG_DEBUG("Got track " << track->name());

        int trackTypeInt = (int)track->trackType();

        if (_trackMap.value(trackTypeInt).contains(track)) {

            RLP_LOG_DEBUG("Got track from track type map");
            _trackMap[trackTypeInt].removeAll(track);
        }

        if (_trackList.contains(track)) {
            RLP_LOG_DEBUG("Got track from track list");
            _trackList.removeAll(track);
        }

        _trackUuidMap.remove(trackUuidStr);

        result = true;

        emit trackDeleted(trackUuidStr);

        if (emitSyncAction) {

            QVariantMap kwargs;

            kwargs.insert("track_uuid", track->uuid().toString());
            kwargs.insert("track_name", track->name());
            
            emit syncAction("deleteTrack", kwargs);
        }

    } else {
        RLP_LOG_WARN("Track not found:" << trackUuidStr)
    }
    
    return result;
}


bool
DS_Session::setTrackEnabled(int idx, bool isEnabled)
{
    bool result = false;
    DS_TrackPtr track = getTrackByIndex(idx);
    
    if (track) {
        
        RLP_LOG_DEBUG("" << idx << " " << isEnabled);
        
        track->setEnabled(isEnabled);
        result = true;
        emit trackVisibilityChanged(idx, isEnabled);
    }
    
    return result;
}


DS_NodePtr
DS_Session::getNodeByFrame(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    DS_NodePtr result = nullptr;

    for (int i=_trackList.size() - 1; i >= 0; --i) {

        if (_trackList.at(i)->isEnabled())
        {
            DS_NodePtr node = _trackList.at(i)->getNodeByFrame(frameNum);
            if (node != nullptr)
            {
                return node;
            }
        }
    }

    return result;
}


QList<DS_NodePtr>
DS_Session::getAllNodesByFrame(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    QList<DS_NodePtr> result;

    for (int i=_trackList.size() - 1; i >= 0; --i) {
        
        if (_trackList.at(i)->isEnabled())
        {
            DS_NodePtr node = _trackList.at(i)->getNodeByFrame(frameNum);
            if (node != nullptr)
            {
                result.append(node);
            }
        }
    }

    return result;
}


DS_NodePtr
DS_Session::getNodeByUuid(QString uuidStr)
{

    QUuid uu(uuidStr);

    DS_NodePtr result = nullptr;
    if (_nodeUuidMap.contains(uu))
    {
        result = _nodeUuidMap.value(uu);
    }

    return result;
}


qlonglong
DS_Session::getNodeFrameIndex(DS_NodePtr node)
{
    // RLP_LOG_DEBUG("");

    qlonglong result = 0;

    if (_trackList.size() == 0)
    {
        // RLP_LOG_ERROR("No tracks in session!");
        return result;
    }


    DS_Track* track = node->getProperty<DS_Track*>("session.track");
    QString trackUuidStr = track->uuid().toString();

    return _trackUuidMap.value(trackUuidStr)->getNodeFrameIndex(node);
}


/*
QVariantMap
DS_Session::getFrameTrackMap(QVariantList inFrames)
{
    QVariantMap result;
    QMap<qlonglong, bool> _tmpMap;
    
    // for speed
    QMap<qlonglong, bool> _inFrameMap;
    
    QVariantList::iterator infit;
    
    for (infit = inFrames.begin(); infit != inFrames.end(); ++infit) {
        _inFrameMap[(qlonglong)(*infit).toqlonglongLong()] = true;
    }
    
    
    for (int i=_trackList.size() - 1; i >= 0; --i) {
        
        if (_trackList.at(i)->enabled()) {
            
            QList<qlonglong> frameList = _trackList.at(i)->getFrames();
            
            QList<qlonglong>::iterator fit;
            for (fit=frameList.begin(); fit != frameList.end(); ++fit) {
                if ((_inFrameMap.contains(*fit)) && (!(_tmpMap.contains(*fit)))) {
                    _tmpMap[*fit] = true;
                    
                    QString frameStr = QString("%1").arg(*fit);
                    result[frameStr] = i;
                }
            }
            // QString frame("%1").arg(
        }
    }
    
    return result;
}
*/


void
DS_Session::setFrameCount(qlonglong frameCount)
{
    // RLP_LOG_DEBUG(frameCount);
    
    if (_outFrame == _frameCount) {
        _outFrame = frameCount;
    }

    _frameCount = frameCount;
    
    emit frameCountChanged(frameCount);
}


void
DS_Session::setInFrame(qlonglong inFrame)
{
    // RLP_LOG_DEBUG(inFrame);
    
    _inFrame = inFrame;

    emit inFrameChanged(inFrame);
}


void
DS_Session::setOutFrame(qlonglong outFrame)
{
    // RLP_LOG_DEBUG(outFrame);
    
    _outFrame = outFrame;

    emit outFrameChanged(outFrame);
}


bool
DS_Session::isFrameValid(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);
    
    return ((frameNum > 0) && (frameNum <= _frameCount));
}


void
DS_Session::clear(QString uuidStr)
{
    RLP_LOG_DEBUG("")
    
    /*
    int numTracks = _trackList.size();
    for (int i=0; i != numTracks; ++i) {
        clearTrack(0);
    }
    */
    
    _trackList.clear();
    _trackMap.clear();
    
    _trackUuidMap.clear();
    _nodeUuidMap.clear();
    _properties.clear();

    _frameCount = 1;
    _inFrame = 1;
    _outFrame = 1;

    if (uuidStr != "")
    {
        RLP_LOG_DEBUG("Setting Session UUID: " << uuidStr)

        _uuid = QUuid(uuidStr);

        initCacheDir();
    }

    // addTrack();
    
    emit sessionCleared();
}


void
DS_Session::onMediaAdded(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount)
{
    RLP_LOG_DEBUG("track: " << trackUuidStr << " on frame: " << frameNum);

    QUuid nodeUuid = node->getProperty<QUuid>("graph.uuid");
    _nodeUuidMap[nodeUuid] = node;

    qlonglong mediaEndFrame = frameNum + mediaFrameCount - 1;

    if (mediaEndFrame > _frameCount)
    {
        RLP_LOG_DEBUG("media add end frame past end of session, expanding to " << mediaEndFrame);

        setFrameCount(mediaEndFrame);
    }

    // TODO FIXME: deprecate?
    emit mediaAdded(node, trackUuidStr, frameNum, mediaFrameCount);

    QVariantMap minfo;
    minfo.insert("node_properties", node->getProperties());
    minfo.insert("track_uuid", trackUuidStr);
    minfo.insert("frame", frameNum);
    minfo.insert("media_frame_count", mediaFrameCount);

    emit trackDataChanged("media_added", trackUuidStr, minfo);
}


void
DS_Session::onMediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame)
{
    RLP_LOG_DEBUG("");

    emit mediaRemoved(node, trackUuidStr, atFrame);
}


void
DS_Session::onTrackDataChanged(QString eventName, QString trackUuidStr, QVariantMap info)
{
    emit trackDataChanged(eventName, trackUuidStr, info);
}

QJsonObject
DS_Session::toJson(bool data)
{
    QJsonObject result;
    QJsonArray trackList;
    QJsonObject trackData;

    QList<DS_TrackPtr>::iterator it;

    for (it = _trackList.begin(); it != _trackList.end(); ++it) {
        
        trackList.append((*it)->uuid().toString());
        trackData.insert((*it)->uuid().toString(), (*it)->toJson(data));
    }

    result.insert("version", DS_Session::VERSION);
    result.insert("session_uuid", _uuid.toString());
    result.insert("track_list", trackList);
    result.insert("track_data", trackData);

    return result;
}


