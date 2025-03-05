//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_ANNOTATION_EVENT_H
#define EXTREVLENS_RLANN_DS_ANNOTATION_EVENT_H

#include "RlpExtrevlens/RLANN_DS/Global.h"

static const int RLANN_DS_AnnotationOffloadEventType = QEvent::User + 20;

class RLANN_DS_AnnotationOffloadEvent : public QEvent {

public:
    RLANN_DS_AnnotationOffloadEvent(int trackIdx, QString annUuidStr):
        QEvent(QEvent::Type(RLANN_DS_AnnotationOffloadEventType)),
        _trackIdx(trackIdx),
        _annUuidStr(annUuidStr)
    {
    }

    int trackIdx() { return _trackIdx; }
    QString annUuidStr() { return _annUuidStr; }

protected:

    int _trackIdx;
    QString _annUuidStr;
};

#endif