//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_STOR_SQL_STORAGEMANAGER_H
#define EDB_STOR_SQL_STORAGEMANAGER_H

#include "RlpEdb/DS/StorageManagerBase.h"

#include "RlpEdb/STOR_SQL/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/DataSource.h"
#include "RlpEdb/STOR_SQL/EntityType.h"
#include "RlpEdb/STOR_SQL/FieldType.h"
#include "RlpEdb/STOR_SQL/Entity.h"
#include "RlpEdb/STOR_SQL/FieldValue.h"
#include "RlpEdb/STOR_SQL/EntityCache.h"
#include "RlpEdb/STOR_SQL/Event.h"

class EDB_STOR_SQL_API EdbStorSql_StorageManager : public EdbDs_StorageManagerBase {

public:
    RLP_DEFINE_LOGGER
    
    EdbStorSql_StorageManager(QString fileName, QString dsName);
    EdbStorSql_StorageManager(EdbStorSql_Connection* conn);

    EdbStorSql_Connection* conn() { return _conn; }
    EdbStorSql_EntityCache* ecache() { return _ecache; }

    EdbDs_StorageHandleBase* makeStorageHandle(EdbDs_ObjectBase* eobj);
    
    QString fileName() { return _fileName; }

private:

    void initTables();

private:

    QString _fileName;
    EdbStorSql_Connection* _conn;
    EdbStorSql_EntityCache* _ecache;

};

// Q_DECLARE_METATYPE(DATA_SQL_DataSource);

#endif