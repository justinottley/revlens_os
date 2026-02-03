
#ifndef EDB_STOR_SQL_ENTITYTYPE_H
#define EDB_STOR_SQL_ENTITYTYPE_H


#include "RlpEdb/DS/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/StorageHandle.h"

#include "RlpEdb/DS/EntityType.h"

class EDB_STOR_SQL_API EdbStorSql_EntityType : public EdbStorSql_StorageHandle {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_EntityType(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_EntityType* et);

    static void init(EdbStorSql_Connection* conn);
    
    BindResult bind();


private:

    EdbDs_EntityType* _et;

};

#endif