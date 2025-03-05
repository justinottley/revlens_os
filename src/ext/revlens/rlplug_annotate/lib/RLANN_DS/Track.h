//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_TRACK_H
#define EXTREVLENS_RLANN_DS_TRACK_H

#include "RlpExtrevlens/RLANN_DS/Global.h"

#include "RlpExtrevlens/RLANN_DS/BrushBase.h"
#include "RlpExtrevlens/RLANN_DS/Annotation.h"
#include "RlpRevlens/DS/Track.h"


class RLANN_DS_Track;
typedef QList<RLANN_DS_Track*> TrackList;
typedef QList<RLANN_DS_Track*>::iterator TrackListIterator;

// TODO FIXME: register track type -> track name?
//
static const int TRACK_TYPE_ANNOTATION = 1;

class EXTREVLENS_RLANN_DS_API RLANN_DS_Track : public DS_Track {
    Q_OBJECT
    
public:
    
    RLP_DEFINE_LOGGER

    RLANN_DS_Track(DS_Session* session, int idx, QUuid uuid, QString owner);
    ~RLANN_DS_Track();
    
    bool initCacheDir();

signals:
    void trackDataChanged(QString trackUuidStr, QVariantMap info);

public slots:

    // AnnotationFrameMap* getAnnotationMap() { return &_annFrameMap; }

    bool insertAnnotation(qlonglong frameNum, RLANN_DS_AnnotationPtr ann);
    bool hasAnnotation(qlonglong frameNum);
    bool updateAnnotationUuid(QUuid srcUuidStr, QUuid destUuidStr);
    bool clearAnnotationOnFrame(qlonglong frame);
    
    void clear();

    bool holdFrame(qlonglong srcFrame, qlonglong destFrame);
    bool releaseFrame(qlonglong frameNum);
    void setAnnotationFrameRange(RLANN_DS_AnnotationPtr ann);
    
    QImage* getImage(qlonglong frameNum);

    RLANN_DS_AnnotationPtr getAnnotation(qlonglong frameNum);
    RLANN_DS_Annotation* getAnnotationPtr(qlonglong frameNum);

    RLANN_DS_AnnotationPtr getAnnotationByUuid(QString uuidStr);

    QList<qlonglong> getAnnotatedFrames();
    QVariantList getAnnotatedFramesAsList();
    QList<QUuid> getAnnotationUuids();

    QVariantList getFrameList(QUuid annUuid);

    // same as getFrameList(), for better compatibility with calling from python (windows)
    //
    QVariantList getFrameListAsList(QUuid annUuid);

    int getGhostFrameCount() { return _ghostFrameCount; }
    void setGhostFrameCount(int gcount) { _ghostFrameCount = gcount; }

    RLANN_DS_BrushBase* brush() { return _brush; }
    void setBrush(RLANN_DS_BrushBase* brush);

    // Serialization / Deserialization
    //

    QString getCacheFilePath(QString filename);
    // QString getAnnotationCacheFilePath(long frameNum);
    QByteArray getBase64FileData(QString fileName);

    int load(QVariantMap trackData, DS_Factory* factory);
    QJsonObject toJson(bool data=true);

private:
    
    AnnotationMap _annMap;
    AnnotationFrameMap _annFrameMap;
    AnnotationUuidMap _annUuidMap;

    int _ghostFrameCount;

    // Brush Stuff
    RLANN_DS_BrushBase* _brush;
    
};

#endif