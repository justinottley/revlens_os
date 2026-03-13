

#include "RlpEdb/STOR_SQL/FieldType.h"

RLP_SETUP_LOGGER(edb, EdbStorSql, FieldType)

EdbStorSql_FieldType::EdbStorSql_FieldType(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_FieldType* ft):
        EdbStorSql_StorageHandle::EdbStorSql_StorageHandle(
            mgr,
            ft,
            "edb_field_type",
            "ft_"),
        _ft(ft)
{
}



void
EdbStorSql_FieldType::init(EdbStorSql_Connection* conn)
{
    RLP_LOG_DEBUG("")
    
    QString tableName = "edb_field_type";
    
    if (!(conn->tables().contains(tableName))) {
        
        RLP_LOG_INFO(tableName << " table not found, creating")
        
        QSqlQuery q(conn->db());
        bool result = q.exec("CREATE TABLE "
                  + tableName + " ("
                  "ft_rowid INTEGER PRIMARY KEY ASC,"
                  "ft_uuid TEXT UNIQUE,"
                  "ft_name TEXT,"
                  "ft_display_name TEXT,"
                  "ft_type_name TEXT,"
                  "ft_valtype INTEGER,"
                  "ft_et_uuid TEXT,"
                  "ft_metadata TEXT,"
                  "ft_create_date INTEGER,"
                  "ft_create_user TEXT,"
                  "ft_update_date INTEGER,"
                  "ft_update_user TEXT);");
                  
        // qInfo() << "query result: " << result;
           
    } else {
        // qInfo()  << "EdbDs_FieldType table found, skipping";
        
    }
}



EdbDs_StorageHandleBase::BindResult
EdbStorSql_FieldType::bind()
{
    EdbDs_StorageHandleBase::BindResult bindResult = BIND_FAIL;
    
    QString query(""
        "SELECT "
        "ft_rowid,"
        "ft_uuid,"
        "ft_name,"
        "ft_update_date,"
        "ft_update_user"
        " FROM "
        + _tableName +
        " WHERE "
        " ft_et_uuid = :et_uuid "
        " AND "
        " ft_name = :name "
    );
    
    QSqlQuery q(_conn->db());
    q.prepare(query);
    q.bindValue(":et_uuid", _ft->etype_uuid());
    q.bindValue(":name", _ft->name());
    
    
    // qInfo() << "et uuid: " << _ft->etype_uuid() << " name: " << _ft->name() << " " << q.executedQuery();
    
    if (!q.exec()) {
        
        RLP_LOG_ERROR("SELECT failed:" << q.lastError().text())
        
    }
    
    
    
    if (!q.next()) {
    
         QString query(""
            "INSERT INTO "
            + _tableName +
            " ( "
            " ft_rowid,"
            " ft_uuid,"
            " ft_name,"
            " ft_display_name,"
            " ft_type_name,"
            " ft_valtype,"
            " ft_et_uuid,"
            " ft_metadata,"
            " ft_create_date,"
            " ft_create_user,"
            " ft_update_date,"
            " ft_update_user"
            " ) VALUES ( "
            " NULL,"
            ":uuid,"
            ":name,"
            ":display_name,"
            ":type_name,"
            ":valtype,"
            ":et_uuid,"
            ":metadata,"
            ":create_date,"
            ":create_user,"
            ":update_date,"
            ":update_user"
            ")"
        );
        
        QSqlQuery iq(_conn->db());
        iq.prepare(query);
        
        QString newUuid = QUuid::createUuid().toString();
        
        iq.bindValue(":uuid", newUuid);
        iq.bindValue(":name", _ft->name());
        iq.bindValue(":display_name", _ft->display_name());
        iq.bindValue(":type_name", _ft->type_name());
        iq.bindValue(":valtype", (int)_ft->valtype());
        iq.bindValue(":et_uuid", _ft->etype_uuid());
        iq.bindValue(":metadata", _ft->metadataString());
        iq.bindValue(":create_date", 0);
        iq.bindValue(":create_user", "");
        iq.bindValue(":update_date", 0);
        iq.bindValue(":update_user", "");
        
        
        // qInfo() << iq.executedQuery();
        
        if (!iq.exec()) {
            RLP_LOG_ERROR("INSERT failed:" << iq.lastError().text())
            
        } else {
            
            _ft->set_uuid(newUuid, false);
            _rowid = q.lastInsertId().toInt();

            /*

            // Pull rowid..
            // Q: should we use already available uuid instead?
            // motivation for rowid is that we may have an entity
            // that doesnt exist in the table but has a uuid

            QString bindQuery(""
                "SELECT ft_rowid FROM "
                + _tableName +
                " WHERE ft_uuid = :uuid"
            );

            iq.prepare(bindQuery);
            iq.bindValue(":uuid", newUuid);

            if (iq.exec()) {
                if (iq.next()) {

                    int rowid = iq.value(0).toInt();

                    // qInfo() << "EdbStorSql_FieldType::bind(): bound to rowid " << rowid;

                    _rowid = rowid;
                }
            }
            */


            bindResult = BIND_NEW;

            RLP_LOG_DEBUG("success: " << _rowid)

        }
        
    } else {
        
        int rowId = q.value(0).toInt();
        QString uuidStr(q.value(1).toString());
        int udate = q.value(4).toInt();
        QString uuser = q.value(5).toString();
        
        // RLP_LOG_DEBUG(_ft->name() << " rowid: " << rowId) // uuidStr << " udate: " << udate << " rowid " << rowId << " " << uuser;
        
        _ft->set_uuid(uuidStr, false);
        _rowid = rowId;
        bindResult = BIND_EXISTING;
    }
    
    // qInfo() << _ft->uuid();
    
    return bindResult;

}


bool
EdbStorSql_FieldType::remove()
{
    RLP_LOG_DEBUG("")

    bool result = false;


    if (_rowid < 0)
    {
        RLP_LOG_ERROR("cannot remove, invalid rowid, not bound?")
    }

    QString query(""
        "DELETE FROM "
        + _tableName + 
        " WHERE "
        "ft_name = :name"
        " AND "
        "ft_rowid = :rowid"
    );

    QSqlQuery q(_conn->db());
    q.prepare(query);
    q.bindValue(":name", _ft->name());
    q.bindValue(":rowid", _rowid);

    RLP_LOG_DEBUG(q.executedQuery())

    if (!q.exec())
    {
        RLP_LOG_ERROR("DELETE failed:" << q.lastError().text())

    }
    else
    {
        RLP_LOG_VERBOSE("rows affected: " << q.numRowsAffected())
        result = true;
    }
    
    return result;
}