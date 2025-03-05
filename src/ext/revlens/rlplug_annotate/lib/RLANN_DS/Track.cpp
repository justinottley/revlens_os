//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpExtrevlens/RLANN_DS/Track.h"

#include "RlpRevlens/DS/Session.h"


RLP_SETUP_EXT_LOGGER(RLANN_DS, Track)

RLANN_DS_Track::RLANN_DS_Track(DS_Session* session, int idx, QUuid uuid, QString owner):
    DS_Track(session, idx, TRACK_TYPE_ANNOTATION, uuid, owner),
    _ghostFrameCount(0)
{
    _annMap.clear();
    _annFrameMap.clear();
    _annUuidMap.clear();
    
    initCacheDir();   
}


RLANN_DS_Track::~RLANN_DS_Track()
{
    RLP_LOG_DEBUG(""); 

    _annFrameMap.clear();
    _annUuidMap.clear();
}


bool
RLANN_DS_Track::initCacheDir()
{
    QDir annotationCacheDir = QDir(_cacheDir.path() + "/annotation");

    bool result = annotationCacheDir.mkpath(annotationCacheDir.path());
    if (result) {
        _cacheDir = annotationCacheDir;
    }

    RLP_LOG_DEBUG(annotationCacheDir.path() );


    return result;
}


void
RLANN_DS_Track::setBrush(RLANN_DS_BrushBase* brush)
{
    RLP_LOG_DEBUG(_name << " - " << brush->name());

    _brush = brush;
}


bool
RLANN_DS_Track::insertAnnotation(qlonglong frameNum, RLANN_DS_AnnotationPtr ann)
{
    RLP_LOG_DEBUG(frameNum );

    _annFrameMap.insert(frameNum, ann);

    if (!_annMap.contains(ann->uuid())) {
        _annMap.insert(ann->uuid(), ann);
    }
    
    if (!_annUuidMap.contains(ann->uuid())) {

        // QList<qlonglong> frameList;
        
        QSet<qint64> frameSet;
        frameSet.insert(frameNum);
        // frameList.push_back(frameNum);
        _annUuidMap.insert(ann->uuid(), frameSet);

    } else {

        _annUuidMap[ann->uuid()].insert(frameNum);
    }
    

    return true;
}


bool
RLANN_DS_Track::updateAnnotationUuid(QUuid srcUuid, QUuid destUuid)
{
    // RLP_LOG_DEBUG(srcUuid << " -> " << destUuid);

    // Update _annMap
    if (_annMap.contains(srcUuid)) {

        RLP_LOG_DEBUG("Updating annMap");

        _annMap[destUuid] = _annMap[srcUuid];
        _annMap.remove(srcUuid);
    }

    if (_annUuidMap.contains(srcUuid)) {

        RLP_LOG_DEBUG("Updating annUuidMap (uuid -> frame list");

        _annUuidMap[destUuid] = _annUuidMap[srcUuid];
        _annUuidMap.remove(srcUuid);
    }
    return true;
}


bool
RLANN_DS_Track::hasAnnotation(qlonglong frameNum)
{
    return _annFrameMap.contains(frameNum);
}


bool
RLANN_DS_Track::holdFrame(qlonglong srcFrame, qlonglong destFrame)
{
    RLP_LOG_DEBUG("RLANN_DS_Track " << srcFrame << " " << destFrame);

    bool result = false;

    if (_annFrameMap.contains(srcFrame)) {

        RLANN_DS_AnnotationPtr ann = _annFrameMap.value(srcFrame);
        if (ann != nullptr) {
            _annFrameMap.insert(destFrame, ann);

            if (_annUuidMap.contains(ann->uuid())) {
                _annUuidMap[ann->uuid()].insert(destFrame);
                result = true;
                
            }
        } else {
            RLP_LOG_ERROR("no annotation found on source frame " << srcFrame);
        }
        
        
    } else {
        RLP_LOG_ERROR("no annotation in map on source frame " << srcFrame);
    }

    return result;
}


bool
RLANN_DS_Track::releaseFrame(qlonglong destFrame)
{
    bool result = false;

    if (_annFrameMap.contains(destFrame)) {

        RLANN_DS_AnnotationPtr ann = _annFrameMap.value(destFrame);
        if (_annUuidMap.contains(ann->uuid())) {
            result = _annUuidMap[ann->uuid()].remove(destFrame);
        }

        _annFrameMap.remove(destFrame);
    }

    return result;
}


void
RLANN_DS_Track::setAnnotationFrameRange(RLANN_DS_AnnotationPtr ann)
{
    RLP_LOG_DEBUG("");

    QVariantList frameList = getFrameList(ann->uuid());

    RLP_LOG_DEBUG("frameList size: " << frameList.size());

    if (frameList.size() == 0) {
        RLP_LOG_ERROR("No frames in frame list!");
        return;
    }

    qlonglong startFrame = frameList[0].toLongLong();
    qlonglong endFrame = frameList[0].toLongLong();

    for (int fx=0; fx != frameList.size(); ++fx)
    {
        qlonglong lfx = frameList[fx].toLongLong();

        if (lfx < startFrame) {
            startFrame = lfx;
        }

        if (lfx > endFrame) {
            endFrame = lfx;
        }
    }

    ann->setFrameRange(startFrame, endFrame);

    //RLP_LOG_DEBUG(frameList.size());

}


bool
RLANN_DS_Track::clearAnnotationOnFrame(qlonglong frame)
{
    RLP_LOG_DEBUG(frame);

    bool result = false;
    RLANN_DS_AnnotationPtr ann = getAnnotation(frame);

    if (ann != nullptr) {

        RLP_LOG_DEBUG("clearing annotation " << ann->uuid().toString());

        result = true;

        QSet<qlonglong> frameList = _annUuidMap.value(ann->uuid());
        QSet<qlonglong>::iterator it;

        for (it = frameList.begin(); it != frameList.end(); ++it) {

            RLP_LOG_DEBUG("clearing annotation - frame " << *it);
            _annFrameMap.remove(*it);
        }

        _annUuidMap.remove(ann->uuid());
    }

    return result;
}



QVariantList
RLANN_DS_Track::getFrameList(QUuid annUuid)
{
    RLP_LOG_DEBUG(annUuid)

    QVariantList result;
    
    result.clear();

    
    if (_annUuidMap.contains(annUuid)) {
        QSet<qlonglong> flist = _annUuidMap.value(annUuid);
        QSet<qlonglong>::iterator it;
        for (it = flist.begin(); it != flist.end(); ++it) {
            result.append((qlonglong)(*it));
        }
    }

    return result;
}


QVariantList
RLANN_DS_Track::getFrameListAsList(QUuid annUuid)
{
    QVariantList flist = getFrameList(annUuid);
    QVariantList result;

    for (int i=0; i != flist.size(); ++i) {
        result.append(flist.at(i).toLongLong());
    }

    return result;
}


QString
RLANN_DS_Track::getCacheFilePath(QString filename)
{
    return _cacheDir.path() + "/" + filename;
}


/*
QString
RLANN_DS_Track::getAnnotationCacheFilePath(qlonglong frame)
{
    QString filename = QString("%1_combined.png").arg(frame);
    return getCacheFilePath(filename);
}
*/


QByteArray
RLANN_DS_Track::getBase64FileData(QString fileName)
{
    QByteArray result;
    QString filePath = getCacheFilePath(fileName);

    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray b = f.readAll();
        result = b.toBase64();
    }

    return result;
}


RLANN_DS_AnnotationPtr
RLANN_DS_Track::getAnnotation(qlonglong frameNum)
{
    if (_annFrameMap.contains(frameNum)) {
        return _annFrameMap.value(frameNum);
    }

    // RLP_LOG_WARN("Not found:" << frameNum)
    return nullptr;
}


RLANN_DS_Annotation*
RLANN_DS_Track::getAnnotationPtr(qlonglong frameNum)
{
    RLANN_DS_AnnotationPtr ann = getAnnotation(frameNum);
    if (ann) {
        return ann.get();
    }

    return nullptr;
}


RLANN_DS_AnnotationPtr
RLANN_DS_Track::getAnnotationByUuid(QString uuidStr)
{
    QUuid uuid(uuidStr);
    if (_annMap.contains(uuid)) {
        return _annMap.value(uuid);
    }

    return nullptr;
}


QList<qlonglong>
RLANN_DS_Track::getAnnotatedFrames()
{
    QList<qlonglong> result;
    result.clear();

    QList<qlonglong> allFrames = _annFrameMap.keys();
    QList<qlonglong>::iterator it;
    for (it= allFrames.begin(); it != allFrames.end(); ++it) {
        if (_annFrameMap[*it]->isValid()) {
            result.push_back(*it);
        }
    }

    return result;
}


QVariantList
RLANN_DS_Track::getAnnotatedFramesAsList()
{
    QList<qlonglong> flist = getAnnotatedFrames();
    QVariantList l;
    for (int i=0; i != flist.size(); ++i)
    {
        l.append(flist.at(i));
    }

    return l;
}


QList<QUuid>
RLANN_DS_Track::getAnnotationUuids()
{
    RLP_LOG_DEBUG("")

    return _annUuidMap.keys();

}


QImage*
RLANN_DS_Track::getImage(qlonglong frameNum)
{
    if (_annFrameMap.contains(frameNum)) {
        return _annFrameMap.value(frameNum)->getQImage();
    }
    
    return nullptr;
}


int
RLANN_DS_Track::load(QVariantMap trackData, DS_Factory* factory)
{
    RLP_LOG_DEBUG("");
    // LOG_Logging::pprint(trackData);

    int numLoaded = 0;

    QVariantMap annotations = trackData.value("annotation").toMap();
    QVariantMap::iterator it;
    for (it = annotations.begin(); it != annotations.end(); ++it) {
        QString uuidStr = it.key();
        QVariantMap frameData = it.value().toMap();

        RLP_LOG_DEBUG("Loading annotation from UUID " << uuidStr );

        int width = frameData.value("width").toInt();
        int height = frameData.value("height").toInt();
        QString annUuidStr = frameData.value("uuid").toString();

        RLANN_DS_AnnotationPtr ann = RLANN_DS_AnnotationPtr(new RLANN_DS_Annotation(width, height, annUuidStr));
        ann->loadFromSessionData(frameData);
        ann->setValid();

        QVariantList frameList = frameData.value("frame_list").value<QVariantList>();

        RLP_LOG_DEBUG("frameList: " << frameList.size());

        if (frameList.size() > 0)
        {
            // Reload annotations
            //
            insertAnnotation(frameList[0].toLongLong(), ann);
            for (int x=1; x != frameList.size(); ++x) {
                holdFrame(frameList[0].toLongLong(), frameList[x].toLongLong());
            }

            QVariantMap annEvtData;
            annEvtData.insert("frame_list", frameList);

            _session->emitTrackDataChanged("add_annotation", uuid().toString(), annEvtData);

        } else
        {
            RLP_LOG_ERROR("frameList length 0! This should not happen!");
        }

        numLoaded += 1;
    }

    return numLoaded;
}


void
RLANN_DS_Track::clear()
{
    RLP_LOG_DEBUG("")

    _annUuidMap.clear();
    _annFrameMap.clear();

    DS_Track::clear();
}


QJsonObject
RLANN_DS_Track::toJson(bool data)
{
    RLP_LOG_DEBUG(_name);

    QJsonObject result;

    result.insert("track_type", "Annotation");
    result.insert("idx", _idx);
    result.insert("name", _name);
    result.insert("owner", _owner);
    result.insert("uuid", _uuid.toString());

    QJsonObject annotationInfo;

    RLP_LOG_DEBUG(_annUuidMap.size());

    AnnotationUuidMapIterator it;
    for (it = _annUuidMap.begin(); it != _annUuidMap.end(); ++it) {

        RLANN_DS_AnnotationPtr ann = _annMap[it.key()];

        if (ann->isValid()) {

            QSet<qlonglong> flist = it.value();

            QJsonObject annFrameInfo = ann->toJson(data);

            QJsonArray frameList;
            QSet<qlonglong>::iterator flit;

            for (flit = flist.begin(); flit != flist.end(); ++flit) {
                frameList.push_back((qlonglong)(*flit));
            }
            
            annFrameInfo.insert("frame_list", frameList);
            annotationInfo.insert(it.key().toString(), annFrameInfo);
            
        }
        
    }

    result.insert("annotation", annotationInfo);

    return result;
}