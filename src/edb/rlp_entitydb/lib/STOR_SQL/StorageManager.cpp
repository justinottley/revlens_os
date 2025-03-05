
#include <iostream>

#include "RlpEdb/STOR_SQL/StorageManager.h"

RLP_SETUP_LOGGER(edb, EdbStorSql, StorageManager)

EdbStorSql_StorageManager::EdbStorSql_StorageManager(QString fileName, QString dsName):
    EdbDs_StorageManagerBase::EdbDs_StorageManagerBase()
{
    RLP_LOG_DEBUG(fileName);

    _conn = new EdbStorSql_Connection(fileName, dsName);
    _ecache = EdbStorSql_EntityCache::getCache(_conn);

    initTables();
}


EdbStorSql_StorageManager::EdbStorSql_StorageManager(EdbStorSql_Connection* conn):
    EdbDs_StorageManagerBase(),
    _conn(conn)
{
    _ecache = EdbStorSql_EntityCache::getCache(_conn);

    initTables();
}


void
EdbStorSql_StorageManager::initTables()
{
    EdbStorSql_DataSource::init(_conn);
    EdbStorSql_EntityType::init(_conn);
    EdbStorSql_FieldType::init(_conn);
    EdbStorSql_Entity::init(_conn);
    EdbStorSql_FieldValue::init(_conn);
    EdbStorSql_EventHandler::init(_conn);

}


EdbDs_StorageHandleBase*
EdbStorSql_StorageManager::makeStorageHandle(EdbDs_ObjectBase* edbObj)
{
    
    QString classNameIn(edbObj->metaObject()->className());
    
    // RLP_LOG_VERBOSE(classNameIn)

    EdbDs_StorageHandleBase* result = nullptr;
    
    if (classNameIn == "EdbDs_DataSource") {
        result = new EdbStorSql_DataSource(
            this,
            static_cast<EdbDs_DataSource*>(edbObj)
        );
        
    } else if (classNameIn == "EdbDs_EntityType") {
        result = new EdbStorSql_EntityType(
            this,
            static_cast<EdbDs_EntityType*>(edbObj)
        );
        
    } else if (classNameIn == "EdbDs_FieldType") {
        result = new EdbStorSql_FieldType(
            this,
            static_cast<EdbDs_FieldType*>(edbObj)
        );
        
    } else if (classNameIn == "EdbDs_Entity") {
        result = new EdbStorSql_Entity(
            this,
            static_cast<EdbDs_Entity*>(edbObj)
        );
        
    } else if (classNameIn == "EdbDs_FieldValue") {
        result = new EdbStorSql_FieldValue(
            this,
            static_cast<EdbDs_FieldValue*>(edbObj)
        );
    
    } else if (classNameIn == "EdbDs_Event") {

        result = new EdbStorSql_EventHandler(
            this,
            static_cast<EdbDs_Event*>(edbObj)
        );

    }
    
    if (result == nullptr)
    {
        RLP_LOG_WARN("NO RESULT!" << classNameIn)
    }
    // RLP_LOG_VERBOSE("result: " << result->metaObject()->className())

    return result;
    
    //return new EdbStorSql_DataSource(ds);
}