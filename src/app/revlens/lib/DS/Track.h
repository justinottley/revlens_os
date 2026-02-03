//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_TRACK_H
#define REVLENS_DS_TRACK_H

#include <memory>


#include "RlpCore/LOG/Logging.h"
#include "RlpRevlens/DS/Node.h"
#include "RlpRevlens/DS/Factory.h"
#include "RlpRevlens/DS/Global.h"


class DS_Session;

class DS_Track;
typedef std::shared_ptr<DS_Track> DS_TrackPtr;

typedef QPair<DS_NodePtr, qlonglong> NodeFramePair;

struct NodeFrameInfo {
    DS_NodePtr node;
    qlonglong frame;
    qlonglong frameCount;
    bool enabled;
};


typedef QHash<qlonglong, DS_NodePtr>::iterator FrameNumMapIterator;
typedef QHash<QUuid, NodeFrameInfo>::iterator MediaMapIterator;

static const int TRACK_TYPE_MEDIA = 0;

class REVLENS_DS_API DS_Track : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    DS_Track(DS_Session* session, int idx, int trackType=TRACK_TYPE_MEDIA, QUuid uuid = QUuid::createUuid(), QString owner = "");
    ~DS_Track();

signals:

    void mediaAdded(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount);
    void mediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame, bool emitSyncAction);
    void mediaDataReady(QString dataType);
    void mediaEnabledChanged(QUuid nodeUuid, bool isEnabled);

    void trackDataChanged(QString eventName, QString trackUuidStr, QVariantMap info);
    void trackCleared();
    

public slots:
    
    DS_Session* session() { return _session; }
    int index() { return _idx; }
    void setTrackIndex(int idx) { _idx = idx; }

    int trackType() { return _trackType; }

    QUuid uuid() { return _uuid; }

    QString name() { return _name; }
    void setName(QString name);

    QString trackTypeName() { return _trackTypeName; }
    void setTrackTypeName(QString typeName) { _trackTypeName = typeName; }

    QVariantMap metadata() { return _metadata; }
    void setMetadata(QString key, QVariant value) { _metadata.insert(key, value); }
    bool hasMetadataValue(QString key) { return _metadata.contains(key); }
    QVariant metadataValue(QString key) { return _metadata.value(key); }

    QString owner() { return _owner; }
    void setOwner(QString owner) { _owner = owner; }

    bool isEditable();

    QDir cacheDir() { return _cacheDir; }

    DS_NodePtr getNodeByFrame(qlonglong frameNum);


    qlonglong getNodeFrameIndex(DS_NodePtr node);
    qlonglong getNodeFrameByUuid(QString uuidStr);
    qlonglong getMediaFrameNum(qlonglong frameNum);
    // qlonglong getFrameOffsetByIndex(int index);
    
    bool hasNode(qlonglong frameNum);
    QVariantList getFrames();
    qlonglong getMaxFrameNum();

    // Get a map of node uuid -> start frame for each node in the track
    QVariantMap getNodeInfo();

    // Get a sorted list of node/frame info for all the nodes in this track
    QVariantList getNodeFrameList();

    bool insertNode(DS_NodePtr media, qlonglong frameNum);

    bool moveNodeToFrame(DS_NodePtr media, qlonglong frameNum);
    bool removeNodeByFrame(qlonglong frameNum, bool emitSyncAction=true);

    virtual void clear();

    int mediaCount();

    bool isEnabled() { return _enabled; }
    void setEnabled(bool enabled, bool emitDataChanged=true);

    bool isNodeEnabledByUuid(QUuid nodeUuid);
    bool isNodeEnabledByFrame(qlonglong frameNum);
    bool setNodeEnabled(QString nodeUuidStr, bool isEnabled);
    void resetNodeEnabled();

    void onMediaDataReady(QString dataType, QVariantMap data);

    void setLoading(bool isLoading) { _isLoading = isLoading; }
    bool isLoading() { return _isLoading; }

    // Serialize / Deserialize
    //
    virtual int load(QVariantMap trackData, DS_Factory* mediaFactory);
    virtual QJsonObject toJson(bool graph=false);

protected:
    virtual bool initCacheDir();

private:
    void updateMediaIndex(DS_NodePtr node, qlonglong frameNum, bool reset=false);
    void updateMediaIndex2(QUuid atNodeUuid, qlonglong newLength);


protected:

    DS_Session* _session;
    int _idx;
    int _trackType;
    QString _trackTypeName;
    QUuid _uuid;
    QString _owner;
    QString _name;
    QDir _cacheDir;

    bool _enabled;
    bool _isLoading;

    // node -> startframe
    QHash<QUuid, NodeFrameInfo> _mediaMap;

    // frame number -> node
    QHash<qlonglong, DS_NodePtr> _frameNumMap;

    QHash<qlonglong, bool> _enabledFrameMap;

    QVariantMap _metadata;
};

#endif
