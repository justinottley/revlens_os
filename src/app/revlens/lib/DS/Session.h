//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_SESSION_H
#define REVLENS_DS_SESSION_H

#include <memory>

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Track.h"
#include "RlpRevlens/DS/TrackFactoryPlugin.h"

class DS_Session;
typedef std::shared_ptr<DS_Session> DS_SessionPtr;


class REVLENS_DS_API DS_Session : public QObject {
    Q_OBJECT

    static const int VERSION;

signals:

    void frameCountChanged(qlonglong frameCount);
    void inFrameChanged(qlonglong frame);
    void outFrameChanged(qlonglong frame);
    
    // re-emitted from track
    void mediaAdded(DS_NodePtr node, QString uuidStr, qlonglong frameNum, qlonglong mediaFrameCount);

    void mediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame);
    
    void trackCleared(QString trackUuidStr);
    void trackAdded(QString trackUuidStr, QString trackName, QString trackType);
    void trackDeleted(QString trackUuidStr);
    void trackVisibilityChanged(int trackIdx, bool isEnabled);
    void trackDataChanged(QString eventName, QString trackUuidStr, QVariantMap trackData);

    void sessionCleared();
    
    void syncAction(QString action, QVariantMap kwargs);


public:
    RLP_DEFINE_LOGGER

    // when a new track is created, how does the new track get its index?
    // 
    enum TrackIndexOrderPolicy {
        TRACK_IDX_AS_IS, // assign index as tracks are added - serially incrementing no matter the type
        TRACK_IDX_BY_TYPE // assign index by track using track type precedence
    };

public:
    
    DS_Session(QString name, QUuid uuid);
    DS_Session(QString name);

    bool initCacheDir();

public slots:
    
    QString name() { return _name; }
    QUuid uuid() { return _uuid; }


    QString cacheDir(QString subdir = "");
    QString siteCacheDir();

    QVariantMap properties() { return _properties; }
    void updateProperties(QVariantMap props) { _properties.insert(props); }

    bool registerTrackFactoryPlugin(QString trackType, DS_TrackFactoryPlugin* plugin);

    // Order that tracks get assigned a track number (index)
    // allows grouping tracks by type
    //
    bool setTrackTypeIndexOrder(QVariantList indexOrder);
    QVariantList trackTypeIndexOrder() { return _trackTypeIndexOrder; }

    void setTrackIndexOrderPolicy(TrackIndexOrderPolicy policy);
    TrackIndexOrderPolicy trackIndexOrderPolicy() { return _trackIndexOrderPolicy; }

    // next track index consideirng track index policy
    int getNextTrackIndex(int trackType);

    //
    // Track
    //
    

    DS_TrackPtr addTrack(QString name = "", QString trackType = "Media", QUuid uuid = QUuid::createUuid(), QString owner = "", bool emitSyncAction=true);

    // Separate name for easier python binding
    DS_TrackPtr addTrackAllDefaults()
    {
        return addTrack("", "Media", QUuid::createUuid(), "", true);
    }

    //Separate name for easier python binding
    DS_TrackPtr addTrackByType(QString trackType)
    {
        return addTrack("", trackType, QUuid::createUuid(), "", true);
    }


    DS_TrackPtr addTrackNoSync(QString name = "", QString trackType = "Media")
    {
        return addTrack(name, trackType, QUuid::createUuid(), "", false);
    }

    DS_TrackPtr getTrackByIndex(int idx);
    DS_TrackPtr getTrackByUuid(QString uuidStr);
    
    bool clearTrack(int idx);
    bool deleteTrack(QString trackUuidStr, bool emitSyncAction=true);

    bool setTrackEnabled(int idx, bool isEnabled);
    int numTracks();
    QList<DS_TrackPtr> trackList() { return _trackList; }

    //
    // Node Accessors
    //
    
    DS_NodePtr getNodeByFrame(qlonglong frameNum);
    QList<DS_NodePtr> getAllNodesByFrame(qlonglong frameNum);

    qlonglong getNodeFrameIndex(DS_NodePtr node);
    DS_NodePtr getNodeByUuid(QString uuid);
    /*
     * Get a map from frame -> track idx for the session of the input frames
     * QVariantMap for ease of transport into python
     *
     * NOTE: unused
     */
    // QVariantMap getFrameTrackMap(QVariantList inFrames);
    
    // 
    //
    void onMediaAdded(DS_NodePtr node, QString trackUuidStr, qlonglong frameNum, qlonglong mediaFrameNum);
    void onMediaRemoved(DS_NodePtr node, QString trackUuidStr, qlonglong atFrame);
    void onTrackDataChanged(QString eventName, QString trackUuidStr, QVariantMap info);

    //
    //
    qlonglong startFrame() { return 1; }
    qlonglong frameCount() { return _frameCount; }
    qlonglong inFrame() { return _inFrame; }
    qlonglong outFrame() { return _outFrame; }
    
    void setFrameCount(qlonglong frameCount);
    void setInFrame(qlonglong inFrame);
    void setOutFrame(qlonglong outFrame);
    
    bool isFrameValid(qlonglong frameNum);
    
    void clear(QString uuidStr="");
    
    QJsonObject toJson(bool data=false);
    
    void emitTrackDataChanged(QString eventName, QString trackUuidStr, QVariantMap trackData) {
        emit trackDataChanged(eventName, trackUuidStr, trackData);
    }


protected:

    QString _name;
    QUuid _uuid;

    QDir _cacheDir;
    QDir _siteCacheDir;

    QVariantMap _properties;

    qlonglong _frameCount;
    qlonglong _inFrame;
    qlonglong _outFrame;
    
    // track type -> tracks
    QHash<int, QList<DS_TrackPtr> >_trackMap;

    // raw track index -> track
    QList<DS_TrackPtr> _trackList;

    QMap<QString, DS_TrackPtr> _trackUuidMap;
    QMap<QUuid, DS_NodePtr> _nodeUuidMap;
    QMap<QString, DS_TrackFactoryPlugin*> _trackPlugins;

    // precedence order search path / order for how to assign / set a track index when creating a new track
    // should be list of ints
    QVariantList _trackTypeIndexOrder;
    TrackIndexOrderPolicy _trackIndexOrderPolicy;

    
};

Q_DECLARE_METATYPE(DS_SessionPtr)


#endif
