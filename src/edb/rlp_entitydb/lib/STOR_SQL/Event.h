
#ifndef EDB_STOR_SQL_EVENT_H
#define EDB_STOR_SQL_EVENT_H


#include "RlpEdb/STOR_SQL/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/StorageHandle.h"

#include "RlpEdb/DS/Event.h"


class EDB_STOR_SQL_API EdbStorSql_EventBase : public EdbStorSql_StorageHandle {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_EventBase(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_Event* event,
        QString tableName,
        QString colPrefix
    );


    static void init(EdbStorSql_Connection* conn, QString tableName, QString colPrefix);

};


// -------------


class EDB_STOR_SQL_API EdbStorSql_Event : public EdbStorSql_EventBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_Event(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_Event* event
    );

};


// -----------


class EDB_STOR_SQL_API EdbStorSql_EventQueue : public EdbStorSql_EventBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_EventQueue(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_Event* event
    );
};


// -----------


class EDB_STOR_SQL_API EdbStorSql_EventHandler : public EdbStorSql_StorageHandle {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_EventHandler(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_Event* event
    );

    BindResult bind();
    bool save();

    static void init(EdbStorSql_Connection* conn);

    int getMaxEventId(bool remote=true);
    bool updateEvent(QString evtId, QString field, QVariant val);

private:
    EdbStorSql_Event* _evtHandle;
    EdbStorSql_EventQueue* _evtQueueHandle;

    EdbDs_Event* _event;

};


#endif