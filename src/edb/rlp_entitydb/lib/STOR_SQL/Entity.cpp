

#include "RlpEdb/STOR_SQL/Entity.h"
#include "RlpEdb/STOR_SQL/EntityCache.h"

#include "RlpEdb/DS/FieldValue.h"

RLP_SETUP_LOGGER(edb, EdbStorSql, Entity)

EdbStorSql_Entity::EdbStorSql_Entity(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_Entity* entity):
        EdbStorSql_StorageHandle::EdbStorSql_StorageHandle(
            mgr,
            entity,
            "edb_entity",
            "e_"),
        _entity(entity)
{
    _ecache = EdbStorSql_EntityCache::getCache(_conn);
}



void
EdbStorSql_Entity::init(EdbStorSql_Connection* conn)
{
    RLP_LOG_DEBUG("")
    
    QString tableName = "edb_entity";
    
    if (!(conn->tables().contains(tableName))) {
        
        RLP_LOG_DEBUG(tableName << " table not found, creating")
        
        QSqlQuery q(conn->db());
        bool result = q.exec("CREATE TABLE "
                  + tableName + " ("
                  "e_rowid INTEGER PRIMARY KEY ASC,"
                  "e_uuid TEXT UNIQUE,"
                  "e_name TEXT,"
                  "e_metadata TEXT,"
                  "e_et_uuid TEXT,"
                  "e_create_date INTEGER,"
                  "e_create_user TEXT,"
                  "e_update_date INTEGER,"
                  "e_update_user TEXT);");
                  
        RLP_LOG_VERBOSE("query result: " << result)
           
    } else {
        RLP_LOG_VERBOSE("EdbDs_Entity table found, skipping")
    }
}



EdbDs_StorageHandleBase::BindResult
EdbStorSql_Entity::bind()
{
    EdbDs_StorageHandleBase::BindResult bindResult = BIND_FAIL;
    
    if (_entity->etHandle() != nullptr)
    {
        RLP_LOG_DEBUG(_entity->etHandle()->name() << "autocreate:" << _entity->autoCreate())
        
    }
    else
    {
        RLP_LOG_DEBUG(_entity->et_uuid())
    }

    
    QString query(""
        "SELECT "
        "e_rowid,"
        "e_uuid,"
        "e_name,"
        "e_update_date,"
        "e_update_user"
        " FROM "
        + _tableName +
        " WHERE "
        " e_et_uuid = :et_uuid "
        " AND "
    );

    bool hasUuid = false;
    bool hasName = false;

    if (_entity->uuid() != "")
    {
        query += "e_uuid = :uuid";
        hasUuid = true;
    }

    else if (_entity->name() != "")
    {
        query += "e_name = :name";
        hasName = true;
    }

    
    QSqlQuery q(_conn->db());
    q.prepare(query);
    q.bindValue(":et_uuid", _entity->et_uuid());

    if (hasUuid)
    {
        q.bindValue(":uuid", _entity->uuid());
    }

    else if (hasName)
    {
        q.bindValue(":name", _entity->name());
    }



    if ((hasUuid) || (hasName))
    {
        QString etName = _ecache->getEntityTypeName(_entity->et_uuid());

        RLP_LOG_VERBOSE(etName << " " << _entity->name() << " " << q.executedQuery())
        
        if (!q.exec())
        {
            RLP_LOG_ERROR("SELECT failed:" << q.lastError().text())
            RLP_LOG_ERROR(q.executedQuery())
        }
        
        
        if (q.next())
        {
            int rowId = q.value(0).toInt();
            QString uuidStr(q.value(1).toString());
            int udate = q.value(4).toInt();
            QString uuser = q.value(5).toString();
            
            RLP_LOG_VERBOSE("rowid: " << rowId) // uuidStr << " udate: " << udate << " rowid " << rowId << " " << uuser;
            
            _entity->set_uuid(uuidStr, false);
            _rowid = rowId;
            bindResult = BIND_EXISTING;
        }
    }


    if ((bindResult != BIND_EXISTING) && (_entity->autoCreate()))
    {
        QString query(""
            "INSERT INTO "
            + _tableName +
            " ( "
            " e_rowid,"
            " e_uuid,"
            " e_name,"
            " e_et_uuid,"
            " e_metadata,"
            " e_create_date,"
            " e_create_user,"
            " e_update_date,"
            " e_update_user"
            " ) VALUES ( "
            " NULL,"
            ":uuid,"
            ":name,"
            ":et_uuid,"
            ":metadata,"
            ":create_date,"
            ":create_user,"
            ":update_date,"
            ":update_user"
            ")"
        );

        q.prepare(query);
        
        QString newUuid = QUuid::createUuid().toString();
        if (hasUuid)
        {
            newUuid = _entity->uuid();
        }

        QString entityName = _entity->name();
        if (entityName == "")
        {
            entityName = newUuid;
        }

        qint64 tnow = QDateTime::currentSecsSinceEpoch();


        q.bindValue(":uuid", newUuid);
        q.bindValue(":name", entityName);
        q.bindValue(":et_uuid", _entity->et_uuid());
        q.bindValue(":metadata", _entity->metadataString());
        q.bindValue(":create_date", tnow);
        q.bindValue(":create_user", _entity->createUser());
        q.bindValue(":update_date", tnow);
        q.bindValue(":update_user", _entity->updateUser());

        // RLP_LOG_VERBOSE(q.executedQuery() << " uuid=" << newUuid << " name=" <<  entityName << " entity: " << _entity->etHandle()->name()

        if (!q.exec())
        {
            QString etName = _ecache->getEntityTypeName(_entity->et_uuid());

            RLP_LOG_ERROR(q.executedQuery() << " uuid=" << newUuid << " name=" <<  entityName << " entity: " << etName)
            RLP_LOG_ERROR("INSERT failed: " << q.lastError().text())
            
        }
        else
        {   
            _entity->set_uuid(newUuid, false);
            _rowid = q.lastInsertId().toInt();
    
            bindResult = BIND_NEW;
            
            RLP_LOG_VERBOSE("INSERT success: " << _rowid)
        }
        
        
    }
    
    return bindResult;

}


bool
EdbStorSql_Entity::removeFields()
{
    RLP_LOG_DEBUG("")

    bool fresult = true;

    EdbStorSql_EntityCache* cache = EdbStorSql_EntityCache::getCache(_conn);

    QVariantMap fieldInfo = cache->getEntityTypeFields(_entity->typeName());

    QVariantMap::iterator it;
    for (it = fieldInfo.begin(); it != fieldInfo.end(); ++it)
    {
        EdbDs_FieldValue fv(
            _entity,
            it.key()
        );

        fresult &= fv.remove();
        
    }

    // RLP_LOG_DEBUG(fieldInfo)

    return fresult;
}


bool
EdbStorSql_Entity::removeEntity()
{
    RLP_LOG_DEBUG("")

    bool result = false;

    if (bound())
    {

        QString queryStr(""
            "DELETE FROM edb_entity WHERE "
            "e_uuid = :uuid"
        );

        QSqlQuery q(_conn->db());
        q.prepare(queryStr);
        q.bindValue(":uuid", _entity->uuid());

        RLP_LOG_VERBOSE(q.executedQuery() << " :uuid = " << _entity->uuid())

        if (!q.exec())
        {
            RLP_LOG_ERROR("DELETE failed:" << q.lastError().text())

        }
        else
        {
            RLP_LOG_VERBOSE("rows affected: " << q.numRowsAffected())
            result = true;
        }

    } else
    {
        RLP_LOG_ERROR("cannot remove - not bound")
    }

    return result;

}


bool
EdbStorSql_Entity::remove()
{
    bool result = true;

    if (!removeFields())
    {
        RLP_LOG_ERROR("Error removing fields")
        result = false;
    }

    if (!removeEntity())
    {
        RLP_LOG_ERROR("Error removing entity")
        result = false;
    }

    return result;

}