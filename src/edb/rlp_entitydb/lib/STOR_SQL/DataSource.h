//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_STOR_SQL_DATASOURCE_H
#define EDB_STOR_SQL_DATASOURCE_H

#include "RlpEdb/DS/DataSource.h"

#include "RlpEdb/STOR_SQL/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/StorageHandle.h"


class EdbStorSql_StorageManager;

class EDB_STOR_SQL_API EdbStorSql_DataSource : public EdbStorSql_StorageHandle {

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_DataSource(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_DataSource* ds);
    
    static void init(EdbStorSql_Connection* conn);
    
    
    BindResult bind();
    // void saveOne(QString methodName, int val);
    
private:

    EdbDs_DataSource* _ds;

};

// Q_DECLARE_METATYPE(DATA_SQL_DataSource);

#endif