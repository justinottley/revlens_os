
#include <iostream>

#include "RlpEdb/STOR_SQL/Connection.h"

RLP_SETUP_LOGGER(edb, EdbStorSql, Connection)

EdbStorSql_Connection::EdbStorSql_Connection(
    QString dbName,
    QString connPrefix):
        EdbDs_StorageConnectionBase::EdbDs_StorageConnectionBase(),
        _dbName(dbName)
{
    QString connectionName = QString("%1%2").arg(dbName).arg((uint64_t)QThread::currentThread(), 16 );

    RLP_LOG_DEBUG("Attempting" << connectionName << QThread::currentThread())

    if (!QSqlDatabase::contains(connectionName))
    {
        RLP_LOG_INFO("Creating new connection: " << connectionName)

        _db = QSqlDatabase::addDatabase(dbType(), connectionName); // dbType(), connName());
        _db.setDatabaseName(dbName);
        _isValid = _db.open();
        _tables = _db.tables();

        if (_isValid)
        {
            RLP_LOG_DEBUG("OK: " << dbName)
        } else
        {
            RLP_LOG_ERROR("Connection failed:" << connectionName)
        }

        _connPrefix = connectionName;
    } else
    {
        _db = QSqlDatabase::database(connectionName);
        _isValid = true;
    }

    _tables = _db.tables();
}