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


/*
bool
DS_Track::event(QEvent* event)
{
    // if (event->type() == MEDIA_DataReadEventType) {
    //   RLP_LOG_DEBUG("GOT DATA READ EVENT";
    // }

    return QObject::event(event);
}
*/


DS_NodePtr
DS_Track::getNodeByFrame(qlonglong frameNum)
{
    // RLP_LOG_DEBUG("DS_Track::getNodeByFrame(): " << frameNum);

    // return getNodeByIndex(getNodeIndexByFrame(frameNum));

    DS_NodePtr result = nullptr;

    if (_frameNumMap.contains(frameNum)) {
        result = _frameNumMap[frameNum];
    }

    return result;
}


qlonglong
DS_Track::getNodeFrameIndex(DS_NodePtr node)
{
    QUuid nodeUuid = node->getProperty<QUuid>("graph.uuid");
    return _mediaMap[nodeUuid].second;
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
    for (it=_mediaMap.begin(); it != _mediaMap.end(); ++it) {

        result.insert(it->first->graph()->uuid().toString(), it->second);
    }

    return result;
}


QVariantList
DS_Track::getNodeFrameList()
{
    QVariantList result;

    QMap<qlonglong, QString> frameReverseMap;

    MediaMapIterator it;
    for (it=_mediaMap.begin(); it != _mediaMap.end(); ++it)
    {
        if (it->first.get() == nullptr)
        {
            RLP_LOG_WARN(_name << _uuid << " - Invalid node at" << it->second << ", skipping")
            continue;
        }

        frameReverseMap.insert(it->second, it->first->graph()->uuid().toString());
    }

    QList<qlonglong> frameList = frameReverseMap.keys();
    std::sort(frameList.begin(), frameList.end());

    QList<qlonglong>::iterator fit;

    for (fit = frameList.begin(); fit != frameList.end(); ++fit) {
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

    QUuid mediaUuid = media->getProperty<QUuid>("graph.uuid");
    qlonglong frameCount = media->getProperty<qlonglong>("media.frame_count");

    RLP_LOG_DEBUG(frameNum);

    if ((reset) && (_mediaMap.contains(mediaUuid))) {

        RLP_LOG_DEBUG("Resetting media " << mediaUuid.toString());

        qlonglong currFrame = media->getProperty<qlonglong>("session.frame");
        for (qlonglong y=currFrame; y != (currFrame+ frameCount); ++y) {
            _frameNumMap.remove(y);
        }

        _mediaMap.remove(mediaUuid);

    }

    _mediaMap[mediaUuid] = QPair<DS_NodePtr, qlonglong>(media, frameNum);

    for (qlonglong y=frameNum; y != (frameNum + frameCount); ++y) {
        _frameNumMap[y] = media;
    }

    media->setProperty<qlonglong>("session.frame", frameNum);

}


bool
DS_Track::insertNode(DS_NodePtr media, qlonglong frameNum)
{
    RLP_LOG_DEBUG(frameNum);

    qlonglong frameCount = media->getProperty<qlonglong>("media.frame_count");
    QUuid mediaUuid = media->getProperty<QUuid>("graph.uuid");

    RLP_LOG_DEBUG(_name << " on frame " << frameNum << " Media UUID: "
              << mediaUuid.toString() << " "
              << "frame count: "
              << frameCount);


    RLP_LOG_DEBUG("media map size: " << _mediaMap.size());

    // check that no other node occupies the same frame range as the incoming node

    // TrackResult result{false, "unknown"};

    bool result = false;

    if (_mediaMap.contains(mediaUuid))
    {

        RLP_LOG_ERROR("Cannot insert node - node already present in track");

    } else
    {

        RLP_LOG_DEBUG("media UUID not found, proceeding to framenum map");

        bool mediaFound = false;
        for (qlonglong i=frameNum; i != (frameNum + frameCount); ++i)
        {

            // RLP_LOG_DEBUG("Checking framenum map for " << i);

            if (_frameNumMap.contains(i))
            {

                mediaFound = true;
                QString message = QString("Cannot insert node - another node present at frame %1").arg(i);
                // result.message = message;

                RLP_LOG_ERROR(message);

                break;
            }
        }

        if (!mediaFound)
        {

            RLP_LOG_DEBUG("adding at " << frameNum);

            //_mediaMap[frameNum] = media;

            QVariant trackWrapper;
            trackWrapper.setValue(this);

            media->setProperty<QVariant>("session.track", trackWrapper); // _uuid.toString());
            media->setProperty<qlonglong>("session.frame", frameNum);

            QVariant sessionWrapper;
            sessionWrapper.setValue(DS_SessionPtr(_session));

            media->setProperty<QVariant>("session", sessionWrapper);

            updateMediaIndex(media, frameNum);



            // QVariantMap dinfo;
            // dinfo.insert("data_type", "track");

            // TODO FIXME: Not sure if this data event is necessary yet
            //
            // media->emitDataReadyToGraph(dinfo);

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

    if (_mediaMap.contains(mediaUuid)) {
        updateMediaIndex(node, frameNum, true);

        emit mediaAdded(node, _uuid.toString(), frameNum, frameCount);

        return true;

    } else {

        RLP_LOG_ERROR("media not present in this track - " << mediaUuid.toString());

        return false;
    }

}


bool
DS_Track::removeNodeByFrame(qlonglong frameNum)
{
    bool result = false;

    // TrackResult result{false, "unknown"};

    if (_frameNumMap.contains(frameNum)) {

        DS_NodePtr node = _frameNumMap[frameNum];

        QUuid mediaUuid = node->getProperty<QUuid>("graph.uuid");
        qlonglong startFrame = node->getProperty<qlonglong>("session.frame");
        qlonglong frameCount = node->getProperty<qlonglong>("media.frame_count");

        for (qlonglong f=startFrame; f != (startFrame + frameCount); ++ f) {
            if (_frameNumMap.value(f) == node) {

                RLP_LOG_DEBUG("Removing " << mediaUuid.toString() << " at " << f);

                _frameNumMap.remove(f);
            }
        }

        _mediaMap.remove(mediaUuid);

        result = true;
        emit mediaRemoved(node, _uuid.toString(), startFrame);

    } else {

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

    _mediaMap.clear();
    _frameNumMap.clear();

    emit trackCleared();
    // RLP_LOG_DEBUG("DS_Track::clear() : running reset");
    // reset();

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
DS_Track::onNodeDataReadyReceived(QVariantMap info)
{
    RLP_LOG_DEBUG("");
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

        NodeFramePair nfp = it.value();
        QUuid mediaUuid = it.key();

        RLP_LOG_DEBUG(mediaUuid.toString());

        DS_NodePtr media = nfp.first;
        if (media == nullptr) {
            RLP_LOG_ERROR("SKIPPING, invalid media" );
            continue;
        }

        QJsonObject mediaInfo;

        mediaInfo.insert("frame", (qint64)nfp.second);
        if (graph) {
            mediaInfo.insert("graph", media->toJson());
        }

        QVariantMap* properties = media->getPropertiesPtr();
        QJsonObject props;

        UTIL_Convert::toJsonObject(*properties, &props);

        /*
        QMap<QString, QVariant>::iterator pit;

        for (pit=properties->begin(); pit != properties->end(); ++pit) {

            // Convert the primitive types based on typename
            QString typeName = QString(pit.value().typeName());

            if (typeName == "int") {
                props.insert(pit.key(), pit.value().toInt()); // .toJsonValue());

            } else if (typeName == "QString") {
                props.insert(pit.key(), pit.value().toString());

            } else if (typeName == "double") {
                props.insert(pit.key(), pit.value().toDouble());

            } else if (typeName == "qulonglong") {
                props.insert(pit.key(), pit.value().toLongLong());

            } else if (typeName == "QUuid") {
                props.insert(pit.key(), pit.value().toString());

            } else if (pit.key().startsWith("media.")) {

                if (typeName == "QVariantMap") {
                    QJsonObject res = pit.value().toJsonObject();
                    props.insert(pit.key(), res);

                } else if (typeName == "QVariantList") {

                    QVariantList sl = pit.value().toList();
                    QJsonArray res;
                    for (int sli = 0; sli != sl.size(); sli++) {

                        QVariant lval = sl.at(sli);
                        QString ltypeName = QString(lval.typeName());
                        if (ltypeName == "QString")
                        {
                            res.push_back(lval.toString());

                        } else if (ltypeName =="QVariantMap")
                        {
                            QJsonObject lres = lval.toJsonObject();
                            res.push_back(lres);
                        } else {
                            RLP_LOG_ERROR("NOT SUPPORTED!" << ltypeName)
                            Q_ASSERT(false);
                        }

                    }

                    props.insert(pit.key(), res);

                    // StringList();
                    // QStringList::iterator it;
                    // QJsonArray res;
                    // for (it = sl.begin(); it != sl.end(); ++it) {
                    //     res.push_back((*it));
                    // }

                    // props.insert(pit.key(), res);

                }

            } else {

                // Omitted: frame_info - QVariantList
                RLP_LOG_DEBUG("skipping "
                          << pit.key()
                          << " type "
                          << typeName);
            }

            // RLP_LOG_DEBUG("DS_Track::toJsonObject(): "  << pit.key() << " " << pit.value().typeName() );
        }
        */

        // trackMedia.insert(mediaUuid.toString(), props);
        mediaInfo.insert("properties", props);
        trackMedia.insert(mediaUuid.toString(), mediaInfo);

        mediaCount++;
    }

    result.insert("media", trackMedia);
    result.insert("media_count", mediaCount);

    return result;
}
