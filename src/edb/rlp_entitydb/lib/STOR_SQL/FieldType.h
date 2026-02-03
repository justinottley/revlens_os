
#ifndef EDB_STOR_SQL_FIELDTYPE_H
#define EDB_STOR_SQL_FIELDTYPE_H


#include "RlpEdb/STOR_SQL/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/StorageHandle.h"

#include "RlpEdb/DS/FieldType.h"

class EDB_STOR_SQL_API EdbStorSql_FieldType : public EdbStorSql_StorageHandle {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_FieldType(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_FieldType* ft);

    static void init(EdbStorSql_Connection* conn);
    
    BindResult bind();

    bool remove();

private:

    EdbDs_FieldType* _ft;

};

#endif