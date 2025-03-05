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
#include "RlpRevlens/DS/PlaybackMode.h"

struct TrackResult {
    bool result;
    QString message;
};

class DS_Session;

class DS_Track;
typedef std::shared_ptr<DS_Track> DS_TrackPtr;

typedef QPair<DS_NodePtr, qlonglong> NodeFramePair;

typedef QHash<qlonglong, DS_NodePtr>::iterator FrameNumMapIterator;
typedef QHash<QUuid, NodeFramePair>::iterator MediaMapIterator;

static const int TRACK_TYPE_MEDIA = 0;

class REVLENS_DS_API DS_Track : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    DS_Track(DS_Session* session, int idx, int trackType=TRACK_TYPE_MEDIA, QUuid uuid = QUuid::createUuid(), QString owner = "");
    ~DS_Track();

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

    QString owner() { return _owner; }
    void setOwner(QString owner) { _owner = owner; }

    bool isEditable();

    QDir cacheDir() { return _cacheDir; }

    DS_NodePtr getNodeByFrame(qlonglong frameNum);
    
    qlonglong getNodeFrameIndex(DS_NodePtr node);
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
    bool removeNodeByFrame(qlonglong frameNum);

    virtual void clear();

    int mediaCount();

    bool isEnabled() { return _enabled; }
    void setEnabled(bool enabled, bool emitDataChanged=true);

    // void emitDataReady(QVariantMap info) { emit dataReady(info); }
    void onNodeDataReadyReceived(QVariantMap info);

    // Serialize / Deserialize
    //
    virtual int load(QVariantMap trackData, DS_Factory* mediaFactory);
    virtual QJsonObject toJson(bool graph=false);

protected:
    virtual bool initCacheDir();

private:
    void updateMediaIndex(DS_NodePtr node, qlonglong frameNum, bool reset=false);

signals:
    
    void mediaAdded(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameCount);
    void mediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame);
    // void trackUpdated(QList<DS_NodePtr>* mediaList, qlonglong totalFrames);
    void trackDataChanged(QString eventName, QString trackUuidStr, QVariantMap info);
    void trackCleared();
    void dataReady(QVariantMap info);

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

    // node -> startframe
    QHash<QUuid, NodeFramePair> _mediaMap;

    // frame number -> node
    QHash<qlonglong, DS_NodePtr> _frameNumMap;
};

#endif
