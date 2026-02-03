//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/Track.h"
#include "RlpRevlens/DS/Factory.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpCore/UTIL/Convert.h"

RLP_SETUP_LOGGER(revlens, DS, Track)

DS_Track::DS_Track(DS_Session* session, int idx, int trackType, QUuid uuid, QString owner):
    _session(session),
    _idx(idx),
    _trackType(trackType),
    _trackTypeName(""),
    _uuid(uuid),
    _owner(owner),
    _enabled(true)
{
    RLP_LOG_DEBUG("type: " << trackType << " index: " << _idx);

    _name = QString("ZTrack ");
    _name += QString("%1").arg(idx + 1);

    _mediaMap.clear();
    _frameNumMap.clear();

    initCacheDir();
}


DS_Track::~DS_Track()
{
    RLP_LOG_DEBUG(_name);

    clear();
}


bool
DS_Track::initCacheDir()
{
    RLP_LOG_WARN("BYPASSED")
    return true;

    QDir trackCacheDir = QDir(_session->cacheDir("track") + "/" + _uuid.toString());

    RLP_LOG_DEBUG("creating " << trackCacheDir.path() );

    bool result = trackCacheDir.mkpath(trackCacheDir.path());
    if (result) {
        _cacheDir = trackCacheDir;
    }

    return result;
}


bool
DS_Track::isEditable()
{
    QString username = UTIL_AppGlobals::instance()->globalsPtr()->value("username").toString();
    return (_owner == username);
}


DS_NodePtr
DS_Track::getNodeByFrame(qlonglong frameNum)
{
    // RLP_LOG_DEBUG("DS_Track::getNodeByFrame(): " << frameNum);

    DS_NodePtr result = nullptr;

    if (_frameNumMap.contains(frameNum))
    {
        result = _frameNumMap[frameNum];
    }

    return result;
}


qlonglong
DS_Track::getNodeFrameIndex(DS_NodePtr node)
{
    QUuid nodeUuid = node->getProperty<QUuid>("graph.uuid");
    return _mediaMap[nodeUuid].frame;
}


qlonglong
DS_Track::getNodeFrameByUuid(QString uuidStr)
{
    QUuid u(uuidStr);
    if (_mediaMap.contains(u))
    {
        return _mediaMap[u].frame;
    }

    return -1;
}


qlonglong
DS_Track::getMediaFrameNum(qlonglong frameNum)
{
    RLP_LOG_ERROR("NOT IMPLEMENTED");

    qlonglong result = -1;

    return result;

}


QVariantList
DS_Track::getFrames()
{
    QList<qlonglong> frameList = _frameNumMap.keys();
    std::sort(frameList.begin(), frameList.end());

    QVariantList l;
    for (auto f : frameList)
    {
        l.append(f);
    }

    return l;
}


qlonglong
DS_Track::getMaxFrameNum()
{
    qlonglong maxFrame = 0;
    QVariantList fl = getFrames();
    for (auto it : fl)
    {
        qlonglong nf = it.toLongLong();
        if (nf > maxFrame)
        {
            maxFrame = nf;
        }
    }

    return maxFrame;
}


QVariantMap
DS_Track::getNodeInfo()
{
    QVariantMap result;

    MediaMapIterator it;
    for (it=_mediaMap.begin(); it != _mediaMap.end(); ++it)
    {
        NodeFrameInfo nfi = *it;
        QString nodeUuidStr = it.key().toString();
        qlonglong frame = nfi.frame;

        result.insert(nodeUuidStr, frame);
    }

    return result;
}


QVariantList
DS_Track::getNodeFrameList()
{
    QVariantList result;

    QMap<qlonglong, QString> frameReverseMap;

    MediaMapIterator it;
    for (it = _mediaMap.begin(); it != _mediaMap.end(); ++it)
    {
        if (it->node.get() == nullptr)
        {
            RLP_LOG_WARN(_name << _uuid << " - Invalid node at" << it->frame << ", skipping")
            continue;
        }

        NodeFrameInfo nfi = *it;
        QString nodeUuidStr = it.key().toString();
        qlonglong frame = nfi.frame;

        frameReverseMap.insert(frame, nodeUuidStr);
    }

    QList<qlonglong> frameList = frameReverseMap.keys();
    std::sort(frameList.begin(), frameList.end());

    QList<qlonglong>::iterator fit;

    for (fit = frameList.begin(); fit != frameList.end(); ++fit)
    {
        QVariantMap item;
        item.insert("frame", *fit);
        item.insert("node_uuid", frameReverseMap.value(*fit));

        result.push_back(item);
    }

    return result;
}



bool
DS_Track::hasNode(qlonglong frameNum)
{
    return _frameNumMap.contains(frameNum);
}


void
DS_Track::updateMediaIndex(DS_NodePtr media, qlonglong frameNum, bool reset)
{
    RLP_LOG_DEBUG(frameNum)

    QUuid mediaUuid = media->getProperty<QUuid>("graph.uuid");
    qlonglong frameCount = media->getProperty<qlonglong>("media.frame_count");

    RLP_LOG_DEBUG(frameNum);

    if ((reset) && (_mediaMap.contains(mediaUuid)))
    {

        RLP_LOG_DEBUG("Resetting media " << mediaUuid.toString());

        qlonglong currFrame = media->getProperty<qlonglong>("session.frame");
        for (qlonglong y=currFrame; y != (currFrame+ frameCount); ++y)
        {
            _frameNumMap.remove(y);
        }

        _mediaMap.remove(mediaUuid);

    }

    NodeFrameInfo nfi;
    nfi.node = media;
    nfi.frame = frameNum;
    nfi.frameCount = frameCount;
    nfi.enabled = true;

    _mediaMap[mediaUuid] = nfi;

    for (qlonglong y=frameNum; y != (frameNum + frameCount); ++y)
    {
        _frameNumMap[y] = media;
    }

    media->setProperty<qlonglong>("session.frame", frameNum);

}


void
DS_Track::updateMediaIndex2(QUuid atNodeUuid, qlonglong newLength)
{
    // RLP_LOG_DEBUG(atNodeUuid << "new frame count:" << newLength)

    QString atNodeUuidStr = atNodeUuid.toString();
    QVariantList currNodeList = getNodeFrameList();


    int nodeIndex = -1;
    qlonglong startFrame = -1;

    for (int i=0; i != currNodeList.size(); ++i)
    {
        QVariantMap ninfo = currNodeList.at(i).toMap();
        QString nUuidStr = ninfo.value("node_uuid").toString();
        if (atNodeUuidStr == nUuidStr)
        {
            nodeIndex = i;
            startFrame = ninfo.value("frame").toLongLong();

            // RLP_LOG_DEBUG("Got node at index" << nodeIndex)
            break;
        }
    }

    if ((nodeIndex > -1) && (nodeIndex < currNodeList.size()))
    {
        // RLP_LOG_DEBUG("Got change node at index:" << nodeIndex)

        // Remove everything in the frame index starting at the current node frame
        //

        for (qlonglong f = startFrame; f <= _session->frameCount(); ++f)
        {
            if (!_frameNumMap.contains(f))
            {
                RLP_LOG_WARN(f << "not in framenum map?")
                continue;
            }

            // RLP_LOG_DEBUG("Removing" << f)
            _frameNumMap.remove(f);
        }



        // Update the changed media
        //
        QVariantMap changeNodeInfo = currNodeList.at(nodeIndex).toMap();
        QUuid changeNodeUuid = QUuid(changeNodeInfo.value("node_uuid").toString());

        NodeFrameInfo nri = _mediaMap.value(changeNodeUuid);
        nri.frameCount = newLength;
        _mediaMap[changeNodeUuid] = nri;

        RLP_LOG_DEBUG("Updated length:" << changeNodeUuid << newLength)

        // Shift everything after forward or backward
        //
        QVariantList mediaChangeList;
        qlonglong atFrame = startFrame;

        for (int ci = nodeIndex; ci != currNodeList.size(); ++ci)
        {
            QVariantMap cNodeInfo = currNodeList.at(ci).toMap();
            QUuid nodeUuid = QUuid(cNodeInfo.value("node_uuid").toString());

            if (!_mediaMap.contains(nodeUuid))
            {
                RLP_LOG_WARN("nodeUuid not found in mediamap:" << nodeUuid)
                continue;
            }

            NodeFrameInfo nri = _mediaMap.value(nodeUuid);
            nri.frame = atFrame;
            _mediaMap[nodeUuid] = nri;

            for (qlonglong nf = nri.frame; nf != (nri.frame + nri.frameCount); ++nf)
            {
                _frameNumMap[nf] = nri.node;

                // RLP_LOG_DEBUG(nf << "->" << nodeUuid)
            }

            atFrame = atFrame + nri.frameCount;

            QVariantMap cInfo;
            cInfo.insert("node_uuid", nodeUuid.toString());
            cInfo.insert("frame", nri.frame);
            cInfo.insert("frame_count", nri.frameCount);

            mediaChangeList.append(cInfo);
        }

        RLP_LOG_DEBUG("Done")

        QVariantMap evtInfo;
        evtInfo.insert("media", mediaChangeList);
        
        if (!_isLoading)
        {
            emit trackDataChanged("media_updated", _uuid.toString(), evtInfo);
        }
        
    }

}


bool
DS_Track::insertNode(DS_NodePtr media, qlonglong frameNum)
{
    RLP_LOG_DEBUG(frameNum);

    qlonglong frameCount = media->getProperty<qlonglong>("media.frame_count");
    QUuid mediaUuid = media->getProperty<QUuid>("graph.uuid");

    // RLP_LOG_DEBUG(_name << " on frame " << frameNum << " Media UUID: "
    //           << mediaUuid.toString() << " "
    //           << "frame count: "
    //           << frameCount);


    // RLP_LOG_DEBUG("media map size: " << _mediaMap.size());

    // check that no other node occupies the same frame range as the incoming node


    bool result = false;

    if (_mediaMap.contains(mediaUuid))
    {
        RLP_LOG_ERROR("Cannot insert node - node already present in track");
    } else
    {
        // RLP_LOG_DEBUG("media UUID not found, proceeding to framenum map");

        bool mediaFound = false;
        for (qlonglong i=frameNum; i != (frameNum + frameCount); ++i)
        {

            // RLP_LOG_DEBUG("Checking framenum map for " << i);

            if (_frameNumMap.contains(i))
            {

                mediaFound = true;
                QString message = QString("Cannot insert node - another node present at frame %1").arg(i);

                RLP_LOG_ERROR(message);

                break;
            }
        }

        if (!mediaFound)
        {

            // RLP_LOG_DEBUG("adding at " << frameNum);

            QVariant trackWrapper;
            trackWrapper.setValue(this);

            media->setProperty<QVariant>("session.track", trackWrapper); // _uuid.toString());
            media->setProperty<qlonglong>("session.frame", frameNum);

            QVariant sessionWrapper;
            sessionWrapper.setValue(_session);

            media->setProperty<QVariant>("session", sessionWrapper);

            updateMediaIndex(media, frameNum);

            connect(
                media->graph(),
                &DS_Graph::dataReady,
                this,
                &DS_Track::onMediaDataReady
            );

            result = true;
        }

        emit mediaAdded(media, _uuid.toString(), frameNum, frameCount);
    }

    return result;
}


bool
DS_Track::moveNodeToFrame(DS_NodePtr node, qlonglong frameNum)
{
    QUuid mediaUuid = node->getProperty<QUuid>("graph.uuid");
    qlonglong frameCount = node->getProperty<qlonglong>("media.frame_count");

    if (_mediaMap.contains(mediaUuid))
    {
        updateMediaIndex(node, frameNum, true);

        emit mediaAdded(node, _uuid.toString(), frameNum, frameCount);

        return true;

    } else {

        RLP_LOG_ERROR("media not present in this track - " << mediaUuid.toString());

        return false;
    }

}


bool
DS_Track::removeNodeByFrame(qlonglong frameNum, bool emitSyncAction)
{
    bool result = false;


    if (_frameNumMap.contains(frameNum))
    {

        DS_NodePtr node = _frameNumMap[frameNum];

        QUuid mediaUuid = node->getProperty<QUuid>("graph.uuid");
        qlonglong startFrame = node->getProperty<qlonglong>("session.frame");
        qlonglong frameCount = node->getProperty<qlonglong>("media.frame_count");

        for (qlonglong f=startFrame; f != (startFrame + frameCount); ++ f)
        {
            if (_frameNumMap.value(f) == node)
            {

                // RLP_LOG_DEBUG("Removing " << mediaUuid.toString() << " at " << f);

                _frameNumMap.remove(f);
            }
        }

        _mediaMap.remove(mediaUuid);

        result = true;
        emit mediaRemoved(node, _uuid.toString(), startFrame, emitSyncAction);

    } else
    {

        RLP_LOG_ERROR("Media not found at frame " << frameNum);
    }

    return result;
}


int
DS_Track::mediaCount()
{
    return _mediaMap.size();
}


void
DS_Track::clear()
{
    RLP_LOG_DEBUG("");

    // Need to do this the long way for sync

    QVariantList nodeFrameList = getNodeFrameList();
    
    for (int i=0; i != nodeFrameList.size(); ++i)
    {
        QVariantMap ninfo = nodeFrameList.at(i).toMap();
        qlonglong nframe = ninfo.value("frame").toLongLong();
        removeNodeByFrame(nframe);
    }

    if ((_mediaMap.size() > 0) || (_frameNumMap.size() > 0))
    {
        RLP_LOG_WARN("removeNode didn't remove everything, forcing clear")

        _mediaMap.clear();
        _frameNumMap.clear();
    }

    emit trackCleared();
}


void
DS_Track::setName(QString name)
{
    RLP_LOG_DEBUG(name)

    QVariantMap changeInfo;
    changeInfo.insert("value.old", _name);
    changeInfo.insert("value.new", name);

    _name = name;

    emit trackDataChanged("set_name", _uuid.toString(), changeInfo);
}


void
DS_Track::setEnabled(bool isEnabled, bool emitDataChanged)
{
    // RLP_LOG_DEBUG(isEnabled);

    QVariantMap changeInfo;

    changeInfo.insert("value.old", _enabled);
    changeInfo.insert("value.new", isEnabled);

    _enabled = isEnabled;

    if (emitDataChanged)
    {
        emit trackDataChanged("set_enabled", _uuid.toString(), changeInfo);
    }
}


bool
DS_Track::setNodeEnabled(QString nodeUuidStr, bool isEnabled)
{
    QUuid nodeUuid(nodeUuidStr);
    if (_mediaMap.contains(nodeUuid))
    {
        // RLP_LOG_DEBUG(nodeUuid << ":" << isEnabled)

        _mediaMap[nodeUuid].enabled = isEnabled;

        emit mediaEnabledChanged(nodeUuid, isEnabled);

        return true;
    }

    return false;
}


void
DS_Track::resetNodeEnabled()
{
    RLP_LOG_DEBUG("")

    MediaMapIterator it;
    for (it = _mediaMap.begin(); it != _mediaMap.end(); ++it)
    {
        it->enabled = true;
        emit mediaEnabledChanged(it->node->graph()->uuid(), true);
    }
}


bool
DS_Track::isNodeEnabledByUuid(QUuid nodeUuid)
{
    if (_mediaMap.contains(nodeUuid))
    {
        return _mediaMap[nodeUuid].enabled;
    }

    RLP_LOG_WARN("No node at" << nodeUuid)
    return true;
}


bool
DS_Track::isNodeEnabledByFrame(qlonglong frameNum)
{
    DS_NodePtr node = getNodeByFrame(frameNum);
    if (node != nullptr)
    {
        QUuid guuid = node->graph()->uuid();
        if (_mediaMap.contains(guuid))
        {
            return _mediaMap[guuid].enabled;
        }
    }

    RLP_LOG_WARN("No node at" << frameNum)
    return true;
}


void
DS_Track::onMediaDataReady(QString dataType, QVariantMap data)
{
    // RLP_LOG_DEBUG(dataType)

    if (dataType == "video_source")
    {

        DS_Node* node = data.value("node").value<DS_Node*>();
        qlonglong frameCount = node->getPropertiesPtr()->value("media.frame_count").toLongLong();
        QUuid graphUuid = data.value("graph.uuid").toUuid();

        if (_mediaMap.contains(graphUuid))
        {
            NodeFrameInfo nri = _mediaMap.value(graphUuid);
            if (frameCount != nri.frameCount)
            {
                updateMediaIndex2(graphUuid, frameCount);
            } else
            {
                QVariantMap evtInfo;

                QVariantMap mInfo;
                mInfo.insert("node_uuid", graphUuid.toString());
                mInfo.insert("frame", nri.frame);
                mInfo.insert("frame_count", nri.frameCount);
                QVariantList mediaChangeList = {mInfo};

                evtInfo.insert("media", mediaChangeList);
                if (!_isLoading)
                {
                    emit trackDataChanged("media_updated", _uuid.toString(), evtInfo);
                }
                
            }
        }
    }

    // TODO FIXME: creating a signal with the data does not pass through
    // binding.. why!?!??!
    emit mediaDataReady(dataType);
}


int
DS_Track::load(QVariantMap trackData, DS_Factory* mediaFactory)
{
    RLP_LOG_DEBUG("");

    int numLoaded = 0;

    QVariantMap trackMedia = trackData.value("media").toMap();

    QStringList mediaUuids = trackMedia.keys();
    QStringList::iterator mit;

    int mediaCount = 0;
    int mediaTotal = mediaUuids.size();

    for (mit = mediaUuids.begin(); mit != mediaUuids.end(); ++mit)
    {

        QString mediaUuidIn = *mit;

        RLP_LOG_DEBUG("media UUID in: " << mediaUuidIn)

        QVariantMap mediaData = trackMedia.value(*mit).toMap();
        qlonglong frame = mediaData.value("frame").toLongLong();

        QVariantMap allprops = mediaData.value("properties").toMap();
        QVariantMap props;

        props.insert("graph.uuid", mediaUuidIn);
        // props.insert("graph.frame", frame);
        // props.insert("track.uuid", uuid().toString());

        // Seed some data for progress
        //
        props.insert("progress.media_total", mediaTotal);
        props.insert("progress.media_count", mediaCount);

        QStringList keyList = allprops.keys();
        QStringList::iterator it;

        // Transfer all media.* properties, leave everything else
        //
        for (it=keyList.begin(); it != keyList.end(); ++it) {
            QString keyName = (*it);

            // if (keyName == "media.video")
            // {
            //     RLP_LOG_DEBUG("SKIPPING" << keyName << "=" << allprops.value(keyName))
            //     continue;
            // }

            // if (keyName == "media.audio")
            // {
            //     RLP_LOG_DEBUG("SKIPPING" << keyName << "=" << allprops.value(keyName))
            //     continue;
            // }


            if (keyName.startsWith("media."))
            {
                if (keyName.indexOf("locator") == -1)
                {
                    props.insert(keyName, allprops.value(keyName));
                }

            }
        }

        QVariantMap iprops = mediaFactory->identifyMedia(props);

        DS_NodePtr node = mediaFactory->createMedia(iprops);
        if (node != nullptr)
        {
            insertNode(node, frame);
            numLoaded++;
        }

        mediaCount++;
    }

    return numLoaded;
}


QJsonObject
DS_Track::toJson(bool graph)
{
    QJsonObject result;
    QJsonObject trackMedia;

    result.insert("track_type", "Media");
    result.insert("idx", _idx);
    result.insert("name", _name);
    result.insert("owner", _owner);
    result.insert("uuid", _uuid.toString());

    MediaMapIterator it;

    int mediaCount = 0;

    for (it=_mediaMap.begin(); it != _mediaMap.end(); ++it)
    {

        NodeFrameInfo nfi = it.value();
        QUuid mediaUuid = it.key();

        // RLP_LOG_DEBUG(mediaUuid.toString());

        DS_NodePtr media = nfi.node;
        if (media == nullptr)
        {
            RLP_LOG_ERROR("SKIPPING, invalid media" );
            continue;
        }

        QJsonObject mediaInfo;

        mediaInfo.insert("frame", (qint64)nfi.frame);
        if (graph)
        {
            mediaInfo.insert("graph", media->toJson());
        }

        // Only serialize source info
        QVariantMap srcInfo;

        QVariantMap* properties = media->getPropertiesPtr();
        QJsonObject props;

        UTIL_Convert::toJsonObject(*properties, &props);

        mediaInfo.insert("properties", props);
        trackMedia.insert(mediaUuid.toString(), mediaInfo);

        mediaCount++;
    }

    result.insert("media", trackMedia);
    result.insert("media_count", mediaCount);

    return result;
}
