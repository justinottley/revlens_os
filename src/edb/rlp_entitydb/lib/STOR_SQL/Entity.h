
#ifndef EDB_STOR_SQL_ENTITY_H
#define EDB_STOR_SQL_ENTITY_H


#include "RlpEdb/STOR_SQL/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/StorageHandle.h"

#include "RlpEdb/DS/Entity.h"

class EdbStorSql_EntityCache;

class EDB_STOR_SQL_API EdbStorSql_Entity : public EdbStorSql_StorageHandle {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_Entity(
        EdbDs_StorageManagerBase* mgr,
        EdbDs_Entity* entity);

    static void init(EdbStorSql_Connection* conn);
    
    BindResult bind();

    bool remove();
    
    /*
     * Remove fields of this entity (do not remove entity record itself)
     */
    bool removeFields();

    /*
     * Remove entity record only
     */
    bool removeEntity();


private:

    EdbDs_Entity* _entity;
    EdbStorSql_EntityCache* _ecache;

};

#endif