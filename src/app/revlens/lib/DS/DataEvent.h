//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_DATAEVENT_H
#define REVLENS_DS_DATAEVENT_H

#include "RlpRevlens/DS/Global.h"


static const int DS_DataEventType = QEvent::User + 13;

class DS_DataEvent : public QEvent {

public:
    DS_DataEvent(QVariantMap info):
        QEvent(QEvent::Type(DS_DataEventType)),
        _info(info)
    {
    }

    QVariantMap info() { return _info; }

protected:
    QVariantMap _info;

};

#endif