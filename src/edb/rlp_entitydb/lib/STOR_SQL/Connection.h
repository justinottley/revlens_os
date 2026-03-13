//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_STOR_SQL_CONNECTION_H
#define EDB_STOR_SQL_CONNECTION_H

#include "RlpEdb/DS/StorageConnectionBase.h"
#include "RlpEdb/STOR_SQL/Global.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class EDB_STOR_SQL_API EdbStorSql_Connection : public EdbDs_StorageConnectionBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_Connection(QString dbName, QString connPrefix);

    QSqlDatabase db() { return _db; }

public slots:

    QString dbType() { return "QSQLITE"; }
    QString dbName() { return _dbName; }
    QString connName() { return _connPrefix; }
    QStringList tables() { return _tables; }
    
    bool isValid() { return _isValid; }

private:

    QString _dbName;
    QString _connPrefix;
    QStringList _tables;
    QSqlDatabase _db;
    bool _isValid;

};

#endif
