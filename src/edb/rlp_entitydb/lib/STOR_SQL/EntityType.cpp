

#include "RlpEdb/STOR_SQL/EntityType.h"
#include "RlpEdb/STOR_SQL/StorageManager.h"
#include "RlpEdb/STOR_SQL/EntityCache.h"

RLP_SETUP_LOGGER(edb, EdbStorSql, EntityType)

EdbStorSql_EntityType::EdbStorSql_EntityType(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_EntityType* et):
        EdbStorSql_StorageHandle::EdbStorSql_StorageHandle(
            mgr,
            et,
            "edb_entity_type",
            "et_"),
        _et(et)
{
}



void
EdbStorSql_EntityType::init(EdbStorSql_Connection* conn)
{
	
	RLP_LOG_DEBUG("init entity type table")
	
	QString tableName = "edb_entity_type";
	
	if (!(conn->tables().contains(tableName))) {
		
        RLP_LOG_DEBUG(tableName << " table not found, creating")
		
		QSqlQuery q(conn->db());
		bool result = q.exec("CREATE TABLE "
                  + tableName + " ("
		          "et_rowid INTEGER PRIMARY KEY ASC,"
		          "et_uuid TEXT UNIQUE,"
		          "et_name TEXT,"
                  "et_metadata TEXT,"
                  "et_dts_uuid TEXT,"
                  "et_parent_uuid TEXT,"
                  "et_create_date INTEGER,"
                  "et_create_user TEXT,"
                  "et_update_date INTEGER,"
                  "et_update_user TEXT);");
                  
        
           
	} else {

        RLP_LOG_VERBOSE(tableName << " table found, skipping")
		
	}
}



EdbDs_StorageHandleBase::BindResult
EdbStorSql_EntityType::bind()
{
	EdbDs_StorageHandleBase::BindResult bindResult = BIND_FAIL;
	
	QString query(""
        "SELECT "
        "et_rowid,"
        "et_uuid,"
        "et_name,"
        "et_parent_uuid,"
        "et_update_date,"
        "et_update_user"
	    " FROM "
	    + _tableName +
	    " WHERE "
	    " et_name = :name "
	);
	
	QSqlQuery q(_conn->db());
	q.prepare(query);
	q.bindValue(":name", _et->name());
	
    RLP_LOG_VERBOSE(_et->name() << " " << q.executedQuery())
	
	if (!q.exec()) {
        RLP_LOG_ERROR("SELECT failed:" << q.lastError().text())
    }
    
    
    if (!q.next()) {
	
         QString query(""
            "INSERT INTO "
            + _tableName +
            " ( "
            " et_rowid,"
            " et_uuid,"
            " et_name,"
            " et_dts_uuid,"
            " et_parent_uuid,"
            " et_metadata,"
            " et_create_date,"
            " et_create_user,"
            " et_update_date,"
            " et_update_user"
            " ) VALUES ( "
            " NULL,"
            ":uuid,"
            ":name,"
            ":dts_uuid,"
            ":parent_uuid,"
            ":metadata,"
            ":create_date,"
            ":create_user,"
            ":update_date,"
            ":update_user"
            ");"
        );
        
        q.prepare(query);
        
        QString newUuid = QUuid::createUuid().toString();
        
        q.bindValue(":uuid", newUuid);
        q.bindValue(":name", _et->name());
        q.bindValue(":dts_uuid", _et->dts_uuid());
        q.bindValue(":parent_uuid", _et->parent_uuid());
        q.bindValue(":metadata", _et->metadataString());
        q.bindValue(":create_date", 0);
        q.bindValue(":create_user", "");
        q.bindValue(":update_date", 0);
        q.bindValue(":update_user", "");
        
        RLP_LOG_VERBOSE(q.executedQuery())
        
        if (!q.exec()) {
            RLP_LOG_ERROR("INSERT failed: " << q.lastError().text())
            
        } else {
            
            _et->set_uuid(newUuid, false);
            _rowid = q.lastInsertId().toInt();

            bindResult = BIND_NEW;

            static_cast<EdbStorSql_StorageManager*>(_mgr)->ecache()->registerEntityType(_et->name());
        }
        
    } else {
        
        
        int rowId = q.value(0).toInt();
        QString uuidStr(q.value(1).toString());
        QString parentUuidStr(q.value(3).toString());
        int udate = q.value(4).toInt();
        QString uuser = q.value(5).toString();
        
        // RLP_LOG_DEBUG("rowid: " << rowId) //uidStr << " udate: " << udate << " " << uuser;
        
        _et->set_uuid(uuidStr, false);
        _rowid = rowId;

        // TODO FIXME: Not sure how this happened in previous testing,
        // but should this be moved to a "health check" tool?
        //
        if ((parentUuidStr != "") && (_et->parent_uuid() != "") && (parentUuidStr != _et->parent_uuid())) {
            RLP_LOG_ERROR(_et->name() << " got db parent uuid: " << parentUuidStr << " incoming uuid: " << _et->parent_uuid())
            RLP_LOG_WARN("Resetting parent uuid to " << _et->parent_uuid())
            _et->set_parent_uuid(_et->parent_uuid());
        }

        
        bindResult = BIND_EXISTING;
        
        static_cast<EdbStorSql_StorageManager*>(_mgr)->ecache()->registerEntityType(_et->name());
    }
	
	return bindResult;

}

