//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_STOR_SQL_STORAGEHANDLE_H
#define EDB_STOR_SQL_STORAGEHANDLE_H

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"

#include "RlpEdb/DS/StorageHandleBase.h"
#include "RlpEdb/DS/StorageManagerBase.h"
#include "RlpEdb/DS/EdbObjectBase.h"


class EDB_STOR_SQL_API EdbStorSql_StorageHandle : public EdbDs_StorageHandleBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_StorageHandle(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_ObjectBase* eobj,
        QString tableName="",
        QString colPrefix="",
        int rowid=-1);
    
    virtual QString tableName() { return _tableName; }
    virtual QString colPrefix() { return _colPrefix; }
    
    virtual void saveOne(QString methodName, int val);
    virtual void saveOne(QString methodName, QString val);
    virtual void saveOne(QString methodName, QJsonObject val);
    

    void buildSaveQuery(QString methodName, QSqlQuery* query);
    void runQuery(QSqlQuery* query);
    
    virtual bool bound() { return _rowid != -1; }
    
    int rid() { return _rowid; }

protected:

    EdbStorSql_Connection* _conn;
    QString _tableName;
    QString _colPrefix;
    int _rowid;

};

// Q_DECLARE_METATYPE(DATA_SQL_STORAGEHANDLE);

#endif