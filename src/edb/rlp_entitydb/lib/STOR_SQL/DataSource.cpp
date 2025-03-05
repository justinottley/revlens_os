
#include <iostream>
#include "RlpEdb/STOR_SQL/DataSource.h"


RLP_SETUP_LOGGER(edb, EdbStorSql, DataSource)

EdbStorSql_DataSource::EdbStorSql_DataSource(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_DataSource* ds):
    EdbStorSql_StorageHandle::EdbStorSql_StorageHandle(
        mgr,
        ds,
        "edb_data_source",
        "dts_"),
    _ds(ds)
{
}


void
EdbStorSql_DataSource::init(EdbStorSql_Connection* conn)
{
	RLP_LOG_DEBUG("")
	
	if (!(conn->tables().contains("edb_data_source"))) {

		RLP_LOG_DEBUG("table not found, creating")
		
		QSqlQuery q(conn->db());
		bool result = q.exec("CREATE TABLE edb_data_source ("
		          "dts_rowid INTEGER PRIMARY KEY ASC,"
		          "dts_uuid TEXT UNIQUE,"
		          "dts_name TEXT,"
		          "dts_type TEXT,"
                  "dts_metadata TEXT,"
                  "dts_schema TEXT,"
                  "dts_origin INTEGER,"
                  "dts_version INTEGER,"
                  "dts_create_date INTEGER,"
                  "dts_create_user TEXT,"
                  "dts_update_date INTEGER,"
                  "dts_update_user TEXT);");
                  
        // td::cout << "query result: " << result << std::endl;
           
	} else {
		// RLP_LOG_DEBUG("table found, skipping")
		
	}

}


EdbDs_StorageHandleBase::BindResult
EdbStorSql_DataSource::bind()
{
	EdbDs_StorageHandleBase::BindResult bindResult = BIND_FAIL;
	
    // RLP_LOG_DEBUG("")
	
	QString query(""
        "SELECT "
        "dts_rowid,"
        "dts_uuid,"
        "dts_name,"
        "dts_type,"
        "dts_update_date,"
        "dts_update_user"
	    " FROM edb_data_source;"
	);
	
	QSqlQuery q(_conn->db());
	
	if (!q.exec(query)) {
		
        RLP_LOG_WARN("SELECT failed:" << q.lastError().text())
        
    }
    
    if (!q.next()) {
        
        QString query(""
            "INSERT INTO edb_data_source ("
            " dts_rowid,"
            " dts_uuid,"
            " dts_name,"
            " dts_type,"
            " dts_metadata,"
            " dts_schema,"
            " dts_origin,"
            " dts_version,"
            " dts_create_date,"
            " dts_create_user,"
            " dts_update_date,"
            " dts_update_user"
            " ) VALUES ( "
            " NULL,"
            ":uuid,"
            ":name,"
            ":type,"
            ":metadata,"
            ":schema,"
            ":origin,"
            ":version,"
            ":create_date,"
            ":create_user,"
            ":update_date,"
            ":update_user"
            ");"
        );
        
        q.prepare(query);
        
        QString newUuid = QUuid::createUuid().toString();
        
        q.bindValue(":uuid", newUuid);
        q.bindValue(":name", _ds->name());
        q.bindValue(":type", _ds->type());
        q.bindValue(":metadata", _ds->metadataString());
        q.bindValue(":schema", "");
        q.bindValue(":origin", 0);
        q.bindValue(":version", 1);
        q.bindValue(":create_date", 0);
        q.bindValue(":create_user", "");
        q.bindValue(":update_date", 0);
        q.bindValue(":update_user", "");
        
        if (!q.exec()) {
            RLP_LOG_ERROR("INSERT failed:" << q.lastError().text())

        } else {
            
            _ds->set_uuid(newUuid, false);
            _rowid = q.lastInsertId().toInt();

            bindResult = BIND_NEW;
        }
    } else {
        
        int rowId = q.value(0).toInt();
        QString uuidStr(q.value(1).toString());
        int udate = q.value(4).toInt();
        QString uuser = q.value(5).toString();
        
        // RLP_LOG_DEBUG("rowid " << rowId) //  uuidStr << " udate: " << udate <<  << " " << uuser;
        
        _rowid = rowId;
        _ds->set_uuid(uuidStr, false);
        bindResult = BIND_EXISTING;
        
    }

	
	return bindResult;
}
