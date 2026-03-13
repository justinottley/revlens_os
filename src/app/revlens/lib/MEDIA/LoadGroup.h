
#ifndef REVLENS_MEDIA_LOADGROUP_H
#define REVLENS_MEDIA_LOADGROUP_H

#include "RlpRevlens/MEDIA/Global.h"
#include "RlpRevlens/DS/ControllerBase.h"

// For tracking the status of an async load of mulitple pieces of media
class REVLENS_MEDIA_API MEDIA_LoadGroup : public QObject {
    Q_OBJECT

signals:
    void complete(QUuid uuid, QString dataType);

public:
    RLP_DEFINE_LOGGER

    MEDIA_LoadGroup(DS_ControllerPtr controller, DS_Track* track, int numItems);

public slots:
    void onMediaDataReady(QString dataType, QVariantMap data);

    QUuid uuid() { return _uuid; }

    int trackIdx() { return _trackIdx; }
    int numItems() { return _numItems; }
    int numCompleted() { return _completed; }

private:
    DS_ControllerPtr _controller;
    QUuid _uuid;
    DS_Track* _track;
    int _trackIdx;
    int _numItems;
    int _completed;
    int _thumbCompleted;
};

#endif
