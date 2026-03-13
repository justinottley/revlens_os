

#include "RlpEdb/STOR_SQL/FieldValue.h"

#include "RlpEdb/STOR_SQL/Entity.h"


RLP_SETUP_LOGGER(edb, EdbStorSql, FieldValue)

EdbStorSql_FieldValue::EdbStorSql_FieldValue(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_FieldValue* fv):
        EdbStorSql_StorageHandle::EdbStorSql_StorageHandle(
            mgr,
            fv,
            "edb_field_value",
            "fv_"),
        _fv(fv)
{
}



void
EdbStorSql_FieldValue::init(EdbStorSql_Connection* conn)
{
	
	QString tableName = "edb_field_value";
	
	if (!(conn->tables().contains(tableName))) {

		RLP_LOG_VERBOSE(tableName << " table not found, creating")
		
		QSqlQuery q(conn->db());
		bool result = q.exec("CREATE TABLE "
                  + tableName + " ("
		          "fv_rowid INTEGER PRIMARY KEY ASC,"
		          "fv_uuid TEXT UNIQUE,"
		          "fv_name TEXT,"
                  "fv_metadata TEXT,"
                  "fv_history TEXT,"
                  "fv_e_uuid TEXT,"
                  "fv_et_uuid TEXT,"
                  "fv_f_uuid TEXT,"
                  "fv_create_date INTEGER,"
                  "fv_create_user TEXT,"
                  "fv_update_date INTEGER,"
                  "fv_update_user TEXT,"
                  "fv_type_name TEXT,"
                  "fv_v_int INTEGER,"
                  "fv_v_str TEXT,"
                  "fv_v_float REAL,"
                  "fv_v_json TEXT,"
                  "fv_v_binary BLOB,"
                  "fv_v_e_uuid TEXT,"
                  "fv_v_et_uuid TEXT,"
                  "fv_v_dts_uuid TEXT"
                  ");");
                  
        RLP_LOG_DEBUG("query result: " << result)
           
	} else {
		RLP_LOG_VERBOSE("EdbDs_FieldValue table found, skipping")
		
	}
}


bool
EdbStorSql_FieldValue::runInsert()
{
    RLP_LOG_DEBUG(_fv->name() << _fv->type_name())

    QString typeName = _fv->type_name();
    QString valCol = QString(" fv_v_%1,").arg(typeName);
    if (typeName == "entity")
    {
        valCol = " fv_v_int,";
    }

    else if (typeName == "double")
    {
        valCol = " fv_v_float,";
    }

        QString query(""
        "INSERT INTO "
        + _tableName +
        " ( "
        " fv_rowid,"
        " fv_uuid,"
        " fv_name,"
        " fv_metadata,"
        " fv_e_uuid,"
        " fv_et_uuid,"
        " fv_f_uuid,"
        " fv_type_name,"
        + valCol +
        " fv_v_e_uuid,"
        " fv_v_et_uuid,"
        " fv_v_dts_uuid,"
        " fv_create_date,"
        " fv_create_user,"
        " fv_update_date,"
        " fv_update_user"
        " ) VALUES ( "
        " NULL,"
        ":uuid,"
        ":name,"
        ":metadata,"
        ":e_uuid,"
        ":et_uuid,"
        ":f_uuid,"
        ":type_name,"
        ":val,"
        ":val_e_uuid,"
        ":val_et_uuid,"
        ":val_dts_uuid,"
        ":create_date,"
        ":create_user,"
        ":update_date,"
        ":update_user"
        ")"
    );
    
    QSqlQuery iq(_conn->db());
    iq.prepare(query);
    
    QString newUuid = QUuid::createUuid().toString();

    QString userUuid = "";

    qint64 tnow = QDateTime::currentSecsSinceEpoch();

    iq.bindValue(":uuid", newUuid);
    iq.bindValue(":name", _fv->name());
    iq.bindValue(":metadata", _fv->metadataString());
    iq.bindValue(":e_uuid", _fv->e_uuid());
    iq.bindValue(":et_uuid", _fv->et_uuid());
    iq.bindValue(":f_uuid", "");
    iq.bindValue(":type_name", _fv->type_name());
    iq.bindValue(":create_date", tnow);
    iq.bindValue(":create_user", userUuid);
    iq.bindValue(":update_date", tnow);
    iq.bindValue(":update_user", userUuid);
    
    prepareSaveQuery(&iq);
    
    RLP_LOG_VERBOSE(_fv->name() << iq.executedQuery())

    bool result = false;
    if (!iq.exec()) {
        RLP_LOG_ERROR("INSERT failed:" << iq.lastError().text())
        RLP_LOG_ERROR(iq.boundValues())

    } else {
        
        result = true;
        _fv->set_uuid(newUuid, false);
        _rowid = iq.lastInsertId().toInt();

    }

    return result;
}


EdbDs_StorageHandleBase::BindResult
EdbStorSql_FieldValue::bind()
{
    // RLP_LOG_VERBOSE("")

	EdbDs_StorageHandleBase::BindResult bindResult = BIND_FAIL;
	
	QString query(""
        "SELECT "
        "fv_rowid,"
        "fv_uuid,"
        "fv_name,"
        "fv_update_date,"
        "fv_update_user"
	    " FROM "
	    + _tableName +
	    " WHERE "
	    " fv_e_uuid = :e_uuid "
	    " AND "
	    " fv_name = :name "
	);
	
	QSqlQuery q(_conn->db());
	q.prepare(query);
	q.bindValue(":name", _fv->name());
	q.bindValue(":e_uuid", _fv->e_uuid());
	
	RLP_LOG_VERBOSE(_fv->name() << q.executedQuery())
	
	if (!q.exec()) {
        RLP_LOG_ERROR("SELECT failed:" << q.lastError().text())
        return BIND_FAIL;
    }
    
    
    if (q.size() > 1)
    {
        RLP_LOG_ERROR("SELECT for bind returned multuple results - got " << q.size())
        return BIND_FAIL;
    }

    if (q.next()) {
	    
        int rowId = q.value(0).toInt();
        QString uuidStr(q.value(1).toString());
        int udate = q.value(4).toInt();
        QString uuser = q.value(5).toString();
        
        RLP_LOG_VERBOSE(_fv->name() << " " << uuidStr << " rowid: " << rowId) //  " udate: " << udate  << " " << uuser;
        
        _fv->set_uuid(uuidStr, false);
        _rowid = rowId;
        
        bindResult = BIND_EXISTING;

        
    } else if (_fv->autoCreate()) {
        
        if (runInsert())
        {
            bindResult = BIND_NEW;
        }

    }
    
	return bindResult;

}


void
EdbStorSql_FieldValue::prepareSaveQuery(QSqlQuery* iq)
{
	// Not implemented yet
	//
	iq->bindValue(":val_dts_uuid", "");
        
    QString edbStr = _fv->type_name();
    QVariant val = _fv->val();
    QString valTypeStr = val.typeName();
    
    // RLP_LOG_DEBUG(valTypeStr)
    
    if (edbStr == "entity") {
        
        if (valTypeStr == "QUuid") { // need to lookup the entity type uuid
            
            QString etypeQuery(""
                "SELECT e_et_uuid FROM edb_entity WHERE e_uuid = :euuid"
            );

            QString eUuid = val.toUuid().toString();
            QString etUuid;
            
            RLP_LOG_WARN(_fv->name() << "no entity type uuid, looking up from " << eUuid)

            QSqlQuery eq(_conn->db());
            eq.prepare(etypeQuery);
            eq.bindValue(":euuid", eUuid);


            if (eq.exec()) {
                if (eq.next()) {
                    etUuid = eq.value(0).toString();

                    RLP_LOG_DEBUG("GOT et_uuid: " << etUuid)

                } else {
                    RLP_LOG_ERROR(_fv->name() << "COULD NOT OBTAIN entity type uuid")
                }
            } else {

                RLP_LOG_ERROR(eq.lastError().text())
            }

            Q_ASSERT(etUuid != "");

            iq->bindValue(":val_e_uuid", eUuid);
            iq->bindValue(":val_et_uuid", etUuid);

        } else {

            EdbDs_Entity* fv = val.value<EdbDs_Entity*>();
            
            iq->bindValue(":val_e_uuid", fv->uuid());
            iq->bindValue(":val_et_uuid", fv->et_uuid());

        }

        
        iq->bindValue(":val", 0);
        
    } else {
        
        iq->bindValue(":val_e_uuid", "");
        iq->bindValue(":val_et_uuid", "");
        
        if (valTypeStr == "json") {
            
            QJsonDocument doc(val.toJsonObject());
            QString docStr = doc.toJson(QJsonDocument::Compact);
            
            iq->bindValue(":val", docStr);
            
        } else {
            
            iq->bindValue(":val", val);
            
        }
    }
	
}


bool
EdbStorSql_FieldValue::save()
{
    RLP_LOG_DEBUG(_fv->name())
	
	if (bound()) {
		
        QString typeName = _fv->type_name();
		QString valCol = " fv_v_" + typeName;
	    if (typeName == "entity")
        {
		    valCol = " fv_v_int ";
		}
        else if (typeName == "double")
        {
            valCol = " fv_v_float ";
        }

		
		QSqlQuery q(_conn->db());
		
		QString queryStr = ""
		    "UPDATE " + _tableName + " SET  "
		    "fv_v_e_uuid = :val_e_uuid , "
		    "fv_v_et_uuid = :val_et_uuid , "
		    "fv_v_dts_uuid = :val_dts_uuid , "
		    + valCol + " = :val "
		    " WHERE "
		    "fv_name = :name AND "
		    "fv_e_uuid = :e_uuid";
		
		q.prepare(queryStr);
		
		q.bindValue(":name", _fv->name());
		q.bindValue(":e_uuid", _fv->e_uuid());
		
	    prepareSaveQuery(&q);
		
		RLP_LOG_VERBOSE("bound values: " << q.boundValues())
		RLP_LOG_VERBOSE(q.executedQuery())
		
		if (q.exec()) {
			RLP_LOG_VERBOSE("rows affected: " << q.numRowsAffected())
			
		} else {
			RLP_LOG_ERROR(q.lastError().text())
			
		}
		
	} else {
		
		RLP_LOG_WARN("not bound, running bind()")
		
		runInsert();
		
	}
	return false;
}


bool
EdbStorSql_FieldValue::remove()
{
    RLP_LOG_VERBOSE(_fv->name())

    bool result = false;

    if (bound()) {
        QString queryStr(""
            "DELETE FROM "
            + _tableName + 
            " WHERE fv_uuid = :uuid");

        QSqlQuery q(_conn->db());
        q.prepare(queryStr);
        q.bindValue(":uuid", _fv->uuid());

        RLP_LOG_VERBOSE(_fv->name() << q.executedQuery())

        if (q.exec()) {

            result = true;
            _rowid = -1; // "un-bind" now that the record has been deleted

            RLP_LOG_DEBUG(_fv->name() << " Delete successful")

        } else {

            RLP_LOG_ERROR("DELETE failed: " << q.lastError().text())

        }
    } else {

        // if not bound, mark as successful
        //
        result = true;
    }

    return result;
}