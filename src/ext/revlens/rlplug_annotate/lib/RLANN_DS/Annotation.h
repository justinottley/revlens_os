//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_ANNOTATION_H
#define EXTREVLENS_RLANN_DS_ANNOTATION_H

#include "RlpExtrevlens/RLANN_DS/Global.h"

#include "RlpRevlens/DS/Track.h"

#include "RlpExtrevlens/RLANN_DS/QImageFrameBuffer.h"


class RLANN_DS_Annotation;
typedef std::shared_ptr<RLANN_DS_Annotation> RLANN_DS_AnnotationPtr;

typedef QMap<qlonglong, RLANN_DS_AnnotationPtr> AnnotationFrameMap;
typedef QMap<qlonglong, RLANN_DS_AnnotationPtr>::iterator AnnotationFrameMapIterator;

typedef QMap<QUuid, QSet<qlonglong> > AnnotationUuidMap;
typedef QMap<QUuid, QSet<qlonglong> >::iterator AnnotationUuidMapIterator;

typedef QMap<QUuid, RLANN_DS_AnnotationPtr> AnnotationMap;
typedef QMap<QUuid, RLANN_DS_AnnotationPtr>::iterator AnnotationMapIterator;


class EXTREVLENS_RLANN_DS_API RLANN_DS_AnnotationDisplayWindow : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_DS_AnnotationDisplayWindow(QString displayUuid, int width, int height);

    void update(int width, int height, float displayZoom);

    QPointF mapToImage(const QPointF& inPoint, QPointF& panPercent);


protected:

    QString _displayUuidStr;
    int _width;
    int _height;

    // Coordinate system / rescaling
    //
    
    int _windowWidth;
    int _windowHeight;

    float _displayZoom;

    // zoom adjusted
    //
    float _nDestWidth;
    float _nDestHeight;
    float _nTop;
    float _nLeft;
};


class EXTREVLENS_RLANN_DS_API RLANN_DS_Annotation : public QObject, public RLANN_DS_QImageFrameBuffer {
    Q_OBJECT

signals:
    void strokeEnd();
    void annotationImageChanged(RLANN_DS_Annotation* ann);

public:
    RLP_DEFINE_LOGGER

    RLANN_DS_Annotation(int width, int height, QString uuidStr = "", float opacity=1.0, QColor fillColor=Qt::transparent);
    // ~RLANN_DS_Annotation();

    void reallocate();

    void setWindow(QString displayUuidStr, int width, int height, float displayZoom);
    QPointF mapToImage(QString displayUuidStr, const QPointF& inPoint, QPointF& panPercent);

    void setTrack(DS_TrackPtr track) { _track = track; }

    QString fileName() { return _uuid.toString() + "_combined.png"; }

    static QString getFileName(QString uuidStr) { return uuidStr + "_combined.png"; }
    QByteArray getBase64Data(QImage* img=nullptr);

    bool save(QIODevice* device, const char* format = "PNG", int quality = -1);

    // save annotation data to disk
    bool offload();

    // load annotation data from disk
    bool reload();

    QString diskCachePath();

    QJsonObject toJson(bool data=true);

    void emitStrokeEnd() { emit strokeEnd(); }

public slots:

    // -----

    void setFrameRange(qlonglong startFrame, qlonglong endFrame);
    void setOpacity(float opacity);

    QUuid uuid() { return _uuid; }
    void setUuid(QString uuidStr);

    void setValid() { _valid = true; }
    bool isValid() { return _valid; }

    float getOpacity() { return _opacity; }
    
    qlonglong startFrame() { return _startFrame; }
    qlonglong endFrame() { return _endFrame; }

    bool save(QString path);
    void load(QString imagePath);

    void setImageFromHistory(int idx);
    void saveToHistory();
    int currentHistoryIndex() { return _undoIndex; }
    int historySize() { return _history.size(); }

    bool undoStroke();
    bool redoStroke();

    void loadFromSessionData(QVariantMap annData);

private:

    QUuid _uuid;
    bool _valid; // true if user actually draws / writes data into this annotation
    
    QMap<QString, RLANN_DS_AnnotationDisplayWindow*> _windowMap;

    QColor _fillColor;
    float _opacity;
    
    qlonglong _startFrame;
    qlonglong _endFrame;

    // QList<RLANN_DS_StrokeBase*> _strokes;

    DS_TrackPtr _track;

    QList<QImage> _history;
    int _undoIndex;
};


// -------------

class EXTREVLENS_RLANN_DS_API RLANN_DS_AnnotationPtrDecorator : public QObject {
    Q_OBJECT

public slots:

    QUuid
    uuid(RLANN_DS_AnnotationPtr* ann)
    {
        return (*ann)->uuid();
    }

    
    bool
    isValid(RLANN_DS_AnnotationPtr* ann)
    {
        return ((*ann) != nullptr);
    }


    bool
    isDirty(RLANN_DS_AnnotationPtr* ann)
    {
        return (*ann)->isDirty();
    }


    qlonglong
    startFrame(RLANN_DS_AnnotationPtr* ann)
    {
        return (*ann)->startFrame();
    }


    qlonglong
    endFrame(RLANN_DS_AnnotationPtr* ann)
    {
        return (*ann)->endFrame();
    }

};

#endif
