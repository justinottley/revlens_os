//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_GRAPH_H
#define REVLENS_DS_GRAPH_H

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/FrameBuffer.h"

class REVLENS_DS_API DS_Graph : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    DS_Graph(QVariantMap* properties, QString uuidStr = QString());

    void emitDataReady(QVariantMap data);

    QVariantMap* propertiesPtr() { return _properties; }

public slots:
    QUuid uuid() { return _uuid; }

signals:
    void dataReady(QVariantMap data);

private:
    QUuid _uuid;
    QVariantMap* _properties;
};


#endif
