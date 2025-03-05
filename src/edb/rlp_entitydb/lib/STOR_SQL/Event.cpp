

#include "RlpEdb/STOR_SQL/Event.h"


RLP_SETUP_LOGGER(edb, EdbStorSql, EventBase)
RLP_SETUP_LOGGER(edb, EdbStorSql, Event)
RLP_SETUP_LOGGER(edb, EdbStorSql, EventQueue)
RLP_SETUP_LOGGER(edb, EdbStorSql, EventHandler)

EdbStorSql_EventBase::EdbStorSql_EventBase(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_Event* event,
    QString tableName,
    QString colPrefix):
        EdbStorSql_StorageHandle::EdbStorSql_StorageHandle(
            mgr,
            event,
            tableName,
            colPrefix
        )
{
}


/* static */
void
EdbStorSql_EventBase::init(EdbStorSql_Connection* conn, QString tableName, QString colPrefix)
{
    RLP_LOG_DEBUG("")

    if (!(conn->tables().contains(tableName))) {

        RLP_LOG_DEBUG(tableName << " table not found, creating")

        QSqlQuery q(conn->db());
        bool result = q.exec("CREATE TABLE "
            + tableName + " ("
            + colPrefix + "_rowid INTEGER PRIMARY KEY ASC,"
            + colPrefix + "_r_rowid INTEGER,"
            + colPrefix + "_uuid TEXT UNIQUE,"
            + colPrefix + "_name TEXT,"
            + colPrefix + "_e_uuid TEXT,"
            + colPrefix + "_status TEXT,"
            + colPrefix + "_data TEXT,"
            + colPrefix + "_create_date INTEGER,"
            + colPrefix + "_create_user TEXT,"
            + colPrefix + "_update_date INTEGER,"
            + colPrefix + "_update_user TEXT);");
                
        RLP_LOG_VERBOSE("query result: " << result)

    } else {
        RLP_LOG_VERBOSE("EdbDs_Entity table found, skipping")
    }
}


// ----------


EdbStorSql_Event::EdbStorSql_Event(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_Event* event):
        EdbStorSql_EventBase(
            mgr,
            event,
            "edb_event",
            "evt"
        )
{
}



// --------------------


EdbStorSql_EventQueue::EdbStorSql_EventQueue(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_Event* event):
        EdbStorSql_EventBase(
            mgr,
            event,
            "edb_event_queue",
            "evtq"
        )
{
}


// -----------------


EdbStorSql_EventHandler::EdbStorSql_EventHandler(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_Event* event):
        EdbStorSql_StorageHandle(mgr, event),
        _event(event)
{
    _evtHandle = new EdbStorSql_Event(mgr, event);
    _evtQueueHandle = new EdbStorSql_EventQueue(mgr, event);
}


/* static */
void
EdbStorSql_EventHandler::init(EdbStorSql_Connection* conn)
{
    RLP_LOG_VERBOSE("")

    EdbStorSql_EventBase::init(conn, "edb_event", "evt");
    EdbStorSql_EventBase::init(conn, "edb_event_queue", "evtq");
}


EdbDs_StorageHandleBase::BindResult
EdbStorSql_EventHandler::bind()
{
    // RLP_LOG_VERBOSE("!!!!" << _event->name())
    // RLP_LOG_VERBOSE(_event->metadataString())
    // RLP_LOG_VERBOSE(_event->metadata())

    EdbDs_StorageHandleBase::BindResult bindResult = BIND_FAIL;

    if (_event->name() == "init")
    {
        return BIND_EXISTING;
    }

    QString tableName = "edb_event";
    QString colPrefix = "evt";

    QSqlQuery q(_conn->db());
    QString qStr(""
        "INSERT INTO "
        + tableName + 
        " ( "
        + colPrefix + "_rowid, "
        + colPrefix + "_uuid, "
        + colPrefix + "_name, "
        + colPrefix + "_data, "
        + colPrefix + "_r_rowid, "
        + colPrefix + "_create_date, "
        + colPrefix + "_create_user, "
        + colPrefix + "_update_date, "
        + colPrefix + "_update_user"
        " ) VALUES ( "
        " NULL, "
        " :uuid, " 
        " :name, "
        " :data, "
        " :r_rowid, "
        " :create_date, "
        " :create_user, "
        " :update_date, "
        " :update_user"
        ")"
    );

    qint64 tnow = QDateTime::currentSecsSinceEpoch();

    QString tuuid = QUuid::createUuid().toString();
    if (_event->uuid() != "")
    {
        tuuid = _event->uuid();
    }

    q.prepare(qStr);

    q.bindValue(":uuid", tuuid);
    q.bindValue(":name", _event->name()),
    q.bindValue(":data", _event->metadataString());
    q.bindValue(":r_rowid", _event->remoteEvtId());
    q.bindValue(":create_date", tnow);
    q.bindValue(":create_user", "");
    q.bindValue(":update_date", tnow);
    q.bindValue(":update_user", "");

    if (q.exec())
    {
        _event->set_uuid(tuuid, false);
        _rowid = q.lastInsertId().toInt();

        bindResult = BIND_NEW;

    } else
    {
        RLP_LOG_ERROR(q.lastError().text())
    }

    return bindResult;
}


bool
EdbStorSql_EventHandler::save()
{
    RLP_LOG_DEBUG("")

    return false;
}


int
EdbStorSql_EventHandler::getMaxEventId(bool remote)
{
    RLP_LOG_DEBUG("remote=" << remote)

    QString colName = "evt_rowid";
    if (remote)
    {
        colName = "evt_r_rowid";
    }

    QSqlQuery q(_conn->db());
    QString qStr(""
        "SELECT max(" + colName + ") "
        "FROM edb_event"
    );

    int result = -1;

    q.prepare(qStr);
    if (q.exec())
    {
        RLP_LOG_DEBUG("OK")
        RLP_LOG_DEBUG(q.executedQuery())

        if (q.first())
        {
            result = q.value(0).toInt();

            RLP_LOG_DEBUG("Got result: " << result)
        } else {
            
        }
    } else 
    {
        RLP_LOG_ERROR(q.lastError().text())
    }

    return result;
}


bool
EdbStorSql_EventHandler::updateEvent(QString evtUuid, QString field, QVariant val)
{
    RLP_LOG_DEBUG(evtUuid << field << "=" << val)

    QSqlQuery q(_conn->db());
    QString qStr(""
        "UPDATE edb_event SET "
        "evt_" + field + "="
        ":val WHERE "
        "evt_uuid=:evt_uuid"
    );

    q.prepare(qStr);
    q.bindValue(":val", val);
    q.bindValue(":evt_uuid", evtUuid);

    if (q.exec())
    {
        int rows = q.numRowsAffected();
        RLP_LOG_VERBOSE("rows affected: " << rows)

        return (rows == 1);
    }

    return false;

}