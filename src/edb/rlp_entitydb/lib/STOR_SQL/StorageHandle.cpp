

#include "RlpEdb/STOR_SQL/StorageHandle.h"

RLP_SETUP_LOGGER(edb, EdbStorSql, StorageHandle)

EdbStorSql_StorageHandle::EdbStorSql_StorageHandle(
    EdbDs_StorageManagerBase* mgr,
    EdbDs_ObjectBase* obj,
    QString tableName,
    QString colPrefix,
    int rowid):
        EdbDs_StorageHandleBase::EdbDs_StorageHandleBase(
            obj,
            mgr
        ),              
        _conn(static_cast<EdbStorSql_Connection*>(mgr->conn())),
        _tableName(tableName),
        _colPrefix(colPrefix),
        _rowid(rowid)
{
}


void
EdbStorSql_StorageHandle::buildSaveQuery(QString methodName, QSqlQuery* query)
{
	RLP_LOG_DEBUG(methodName)
	
	QString queryStr(""
	    "UPDATE " + _tableName + " SET "
	    + _colPrefix + methodName +
        " = :val "
        " WHERE "
        + _colPrefix + "rowid = :rowid"
        // + _colPrefix + "uuid = :uuid"
    );
    
    
	// QSqlQuery q(_conn->db());
	query->prepare(queryStr);
	query->bindValue(":rowid", _rowid);
	
}

void
EdbStorSql_StorageHandle::runQuery(QSqlQuery* query)
{
	if (!query->exec()) {
        RLP_LOG_WARN("UPDATE failed: " << query->lastError().text())
    }
	
	RLP_LOG_VERBOSE(query->executedQuery())
	RLP_LOG_VERBOSE("bound values: " << query->boundValues())
}


void
EdbStorSql_StorageHandle::saveOne(QString methodName, int val)
{
	QSqlQuery q(_conn->db());
	buildSaveQuery(methodName, &q);
	
	q.bindValue(":val", val);
	
	
	runQuery(&q);
}


void
EdbStorSql_StorageHandle::saveOne(QString methodName, QString val)
{
	QSqlQuery q(_conn->db());
	buildSaveQuery(methodName, &q);
	
	q.bindValue(":val", val);
	
	// qInfo() << "saveOne(): rowId: " << _rowid << " " << methodName << " " << val;

	runQuery(&q);
}


void
EdbStorSql_StorageHandle::saveOne(QString methodName, QJsonObject val)
{
	// NIY
	
	/*
	
	QSqlQuery q(_conn->db());
	buildSaveQuery(methodName, &q);
	
	q.bindValue(":val", val);
	
	runQuery(&q);
	
	*/
}




