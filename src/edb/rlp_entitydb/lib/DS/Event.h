

#ifndef EDB_DS_EVENT_H
#define EDB_DS_EVENT_H


#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/EntityType.h"
#include "RlpEdb/DS/EdbObjectBase.h"


class EDB_DS_API EdbDs_Event : public EdbDs_ObjectBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbDs_Event(
        EdbDs_StorageManagerBase* storage,
        QString name,
        QVariantMap data=QVariantMap(),
        QVariantMap opts=QVariantMap()
    );

    void setLastEventId(int evtId);

    bool isSynced();


public slots:

    int evtId() { return _evtId; }
    int remoteEvtId() { return _remoteEvtId; }
    QString name() { return _name; }


private:

    int _evtId;
    int _remoteEvtId;

    int _lastEvtId;


};

#endif