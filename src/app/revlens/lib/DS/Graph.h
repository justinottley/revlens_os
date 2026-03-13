//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_GRAPH_H
#define REVLENS_DS_GRAPH_H

#include "RlpRevlens/DS/Global.h"
#include "RlpCore/DS/Flags.h"
#include "RlpRevlens/DS/FrameBuffer.h"

class REVLENS_DS_API DS_Graph : public QObject {
    Q_OBJECT

signals:
    void dataReady(QString dataType, QVariantMap data);
    void flagStateChanged(QString nodeUuidStr, QString flagName, bool flagVal);
    void noticeEvent(QString evtName, QVariantMap evtInfo);

public:
    RLP_DEFINE_LOGGER

    DS_Graph(QVariantMap* properties, QString uuidStr = QString());

    void emitDataReady(QString dataType, QVariantMap data=QVariantMap());
    void emitNoticeEvent(QString evtName, QVariantMap evtInfo=QVariantMap());

    QVariantMap* propertiesPtr() { return _properties; }
    void setProperties(QVariantMap* properties) { _properties = properties; }

public slots:
    QUuid uuid() { return _uuid; }
    void onGraphDataReady(QString dataType, QVariantMap data);

    CoreDs_Flags* flags() { return _flags; }
    void setFlags(CoreDs_Flags* flags) { _flags = flags; }

private slots:
    void onInternalFlagStateChanged(QString flagName, bool val);

private:
    QUuid _uuid;
    QVariantMap* _properties;

    CoreDs_Flags* _flags;

};


#endif
