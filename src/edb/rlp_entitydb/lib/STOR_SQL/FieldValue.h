
#ifndef EDB_STOR_SQL_FIELDVALUE_H
#define EDB_STOR_SQL_FIELDVALUE_H


#include "RlpEdb/DS/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/StorageHandle.h"

#include "RlpEdb/DS/FieldValue.h"

class EDB_STOR_SQL_API EdbStorSql_FieldValue : public EdbStorSql_StorageHandle {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_FieldValue(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_FieldValue* entity);

    static void init(EdbStorSql_Connection* conn);
    
    BindResult bind();
    bool save();
    bool remove();

private:

    void prepareSaveQuery(QSqlQuery* query);
    bool runInsert();
    
private:

    EdbDs_FieldValue* _fv;

};

#endif