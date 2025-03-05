
#include "RlpEdb/STOR_SQL/EntityCache.h"

RLP_SETUP_LOGGER(edb, EdbStorSql, EntityCache)

QMap<QString, EdbStorSql_EntityCache*> EdbStorSql_EntityCache::_cacheMap;
QMutex* EdbStorSql_EntityCache::_CMUTEX =  new QMutex();

EdbStorSql_EntityCache::EdbStorSql_EntityCache(
    EdbStorSql_Connection* conn):
        _conn(conn)
{
}


void
EdbStorSql_EntityCache::setDataSource(EdbDs_DataSource* dts)
{
    RLP_LOG_DEBUG(dts->type());

    _dts = dts;

    registerAllEntityTypes();
    initBuiltinFields();
}


void
EdbStorSql_EntityCache::initBuiltinFields()
{
    RLP_LOG_VERBOSE("")
    
    BUILTIN_FIELDS = {"name", "metadata", "create_date", "create_user", "update_date", "update_user"};
}


EdbStorSql_EntityCache*
EdbStorSql_EntityCache::getCache(EdbStorSql_Connection* conn)
{
    QMutexLocker l(EdbStorSql_EntityCache::_CMUTEX);

    QString cacheKey = conn->connName();

    if (!_cacheMap.contains(cacheKey)) {
        EdbStorSql_EntityCache* cache = new EdbStorSql_EntityCache(conn);

        RLP_LOG_VERBOSE("inserting into cacheMap: " << cacheKey)

        EdbStorSql_EntityCache::_cacheMap.insert(cacheKey, cache);

    }
    
    return _cacheMap.value(cacheKey);
    
}


QString
EdbStorSql_EntityCache::getEntityTypeUuid(QString etName)
{
    if (!_etUuidMap.contains(etName)) {
        registerEntityByName(etName);
    }

    return _etUuidMap.value(etName);
}


QString
EdbStorSql_EntityCache::getEntityTypeName(QString etUuid)
{
    return _etNameMap.value(etUuid);
}


EdbDs_EntityType*
EdbStorSql_EntityCache::getEntityTypeObj(QString etName)
{
    return _etObjMap.value(etName);
}


QVariantMap
EdbStorSql_EntityCache::getEntityTypeFields(QString etName, bool parents)
{
    RLP_LOG_DEBUG(etName << " parents: " << parents)

    if (!_etUuidMap.contains(etName)) {
        registerEntityByName(etName);
    }

    QVariantMap result;

    QStringList etypes;
    etypes.insert(0, etName);

    if (parents)
    {
        etypes.append(parentEntityTypes(etName));
    }
    
    RLP_LOG_VERBOSE("All Types:" << etypes)

    for (int i=0; i != etypes.size(); ++i) {

        QVariantMap oneTypeResult = getEntityTypeFieldsOneEType(etypes.at(i));

        // RLP_LOG_VERBOSE(oneTypeResult)

        QVariantMap::iterator it;
        for (it=oneTypeResult.begin(); it != oneTypeResult.end(); it++)
        {
            result.insert(it.key(), it.value());
        }

    }

    return result;
}


QVariantMap
EdbStorSql_EntityCache::getEntityTypeFieldsOneEType(QString etName)
{
    RLP_LOG_VERBOSE(etName)

    // TODO FIXME: cache into this class?
    //

    QString qstr(""
        "SELECT "
        "ft_rowid, "
        "ft_uuid, "
        "ft_name, "
        "ft_display_name, "
        "ft_type_name, "
        "ft_valtype, "
        "ft_metadata, "
        "ft_et_uuid, "
        "ft_create_date, "
        "ft_create_user, "
        "ft_update_date, "
        "ft_update_user "
        "FROM edb_field_type "
        "WHERE ft_et_uuid = :et_uuid "
    );

    QString etUuid = getEntityTypeUuid(etName);

    QSqlQuery q(_conn->db());
    q.prepare(qstr);
    q.bindValue(":et_uuid", etUuid);

    // RLP_LOG_VERBOSE(q.executedQuery())

    QVariantMap result;

    if (q.exec()) {

        while (q.next()) {

            QVariantMap fInfo;
            
            // RLP_LOG_VERBOSE(etName << " " << q.value(2).toString())

            fInfo.insert("name", q.value(2).toString());
            fInfo.insert("display_name", q.value(3).toString());
            fInfo.insert("type_name", q.value(4).toString());
            fInfo.insert("valtype", q.value(5).toString());

            // TODO: JSON deserialize
            fInfo.insert("metadata", q.value(6).toString());

            fInfo.insert("et_uuid", q.value(7).toString());
            fInfo.insert("create_date", q.value(8).toInt());
            fInfo.insert("create_user", q.value(9).toString());
            fInfo.insert("update_date", q.value(10).toInt());
            fInfo.insert("update_user", q.value(11).toString());

            result.insert(q.value(2).toString(), fInfo);
        }
    } else {
        
        RLP_LOG_ERROR("query failed: " << q.lastError().text())
    }

    return result;
    
}


QStringList
EdbStorSql_EntityCache::childEntityTypes(QString etName)
{
    RLP_LOG_VERBOSE(etName)

    QStringList result;

    if (!_etUuidMap.contains(etName)) {
        registerEntityByName(etName);
    }

    QString etUuid = _etUuidMap.value(etName);

    // RLP_LOG_DEBUG(etName << " uuid: " << etUuid)
    // RLP_LOG_DEBUG(_etUuidMap)

    if (etUuid != "") {
        QStringList uuidList = _etChildMap.value(etUuid);
        for (int i=0; i!= uuidList.size(); ++i)
        {
            QString directChildType = _etNameMap.value(uuidList.at(i));
            result.append(directChildType);
            result.append(childEntityTypes(directChildType));
        }
        

    } else {

        RLP_LOG_DEBUG(etUuid << " Not found in uuid cache")
    }

    return result;
}


QStringList
EdbStorSql_EntityCache::parentEntityTypes(QString etName)
{
    RLP_LOG_DEBUG(etName)

    QStringList result;
    QString etUuid = _etUuidMap.value(etName);
    if (_etParentMap.contains(etUuid)) {
        QString parentUuid = _etParentMap.value(etUuid);
        QString parentName = _etNameMap.value(parentUuid);
        result.append(parentName);

        result.append(parentEntityTypes(parentName));
    }

    return result;
}


QVariantList
EdbStorSql_EntityCache::getEntityTypes()
{
    QVariantList result;
    
    QMap<QString, QVariantMap>::iterator it;

    for (it=_etypeMap.begin(); it != _etypeMap.end(); ++it) {
        
        result.append(it.value());
    }

    return result;
}


void
EdbStorSql_EntityCache::registerAllEntityTypes()
{
    _etypeMap.clear();

    QString qstr(""
        "SELECT "
        "et_uuid, "
        "et_name, "
        "et_metadata, "
        "et_dts_uuid, "
        "et_parent_uuid, "
        "et_create_date, "
        "et_create_user, "
        "et_update_date, "
        "et_update_user "
        " FROM edb_entity_type "
    );


    QSqlQuery q(_conn->db());
    q.prepare(qstr);

    QVariantList result;

    if (q.exec()) {
        while (q.next()) {

            QVariantMap res;

            QString etName = q.value(1).toString();
            registerEntityType(etName);

            res.insert("uuid", q.value(0).toString());
            res.insert("name", q.value(1).toString());

            // TODO: JSON deserialize
            res.insert("metadata", q.value(2).toString());

            res.insert("dts_uuid", q.value(3).toString());
            res.insert("parent_uuid", q.value(4).toString());
            res.insert("create_date", q.value(5).toInt());
            res.insert("create_user", q.value(6).toString());
            res.insert("update_date", q.value(7).toInt());
            res.insert("update_user", q.value(8).toString());

            // result.push_back(res);

            _etypeMap.insert(etName, res);

            RLP_LOG_VERBOSE(res)
        }
    }

    
}


void
EdbStorSql_EntityCache::registerEntityType(QString etName)
{
    if (_etUuidMap.contains(etName)) {

        RLP_LOG_VERBOSE(etName << " already registered, skipping")
        return;
    } else 
    {
        RLP_LOG_VERBOSE(etName)
    }

    registerEntityByName(etName);
    
}



void
EdbStorSql_EntityCache::registerEntityByName(QString etName)
{
    // RLP_LOG_VERBOSE(etName)

    QString query(""
        "SELECT "
        "et_uuid, "
        "et_parent_uuid "
        " FROM "
	    " edb_entity_type "
	    " WHERE "
	    " et_name = :et_name "
	);

    QSqlQuery q(_conn->db());
	q.prepare(query);
	q.bindValue(":et_name", etName);

    if (q.exec() && q.next()) {

        QString entityUuid = q.value(0).toString();
        QString parentUuid = q.value(1).toString();

        RLP_LOG_VERBOSE(entityUuid << " parent: " << parentUuid)

        _etUuidMap.insert(etName, entityUuid);
        _etNameMap.insert(entityUuid, etName);

        // RLP_LOG_VERBOSE("INSERTED INTO etUuidMap and etNameMap")
        // registerFields(entityUuid);

        EdbDs_EntityType* parentObj = nullptr;

        if (parentUuid != "")
        {

            _etParentMap.insert(entityUuid, parentUuid);
            
            QStringList etChildren = _etChildMap.value(parentUuid);
            etChildren.append(entityUuid);
            _etChildMap.insert(parentUuid, etChildren);

            // RLP_LOG_VERBOSE(_etChildMap)

            // get parent entity name for object construction
            //
            QString parentEtName = _etNameMap.value(parentUuid);

            if (_etObjMap.contains(parentEtName))
            {
                parentObj = _etObjMap.value(parentEtName);
            }

        }

        if (_dts)
        {
            RLP_LOG_VERBOSE("attempting to create entity type object using dts: " << _dts)

            EdbDs_EntityType* etObj = new EdbDs_EntityType(etName, _dts, parentObj);
            _etObjMap.insert(etName, etObj);

            RLP_LOG_VERBOSE("Entity type object cached into map")
        }


    } else {

        RLP_LOG_ERROR(q.lastError().text())
    }
}


void
EdbStorSql_EntityCache::registerEntityByParentUuid(QString parentUuid)
{
    RLP_LOG_VERBOSE(parentUuid)

    
    QString query(""
        "SELECT "
        "et_name, "
        "et_uuid "
        " FROM "
	    " edb_entity_type "
	    " WHERE "
	    " et_parent_uuid = :et_uuid "
	);
    
    QSqlQuery q(_conn->db());
	q.prepare(query);
	q.bindValue(":et_uuid", parentUuid);

    if (q.exec())
    {
        if (q.next())
        {

            QString entityName = q.value(0).toString();
            QString etUuid = q.value(1).toString();

            RLP_LOG_DEBUG(entityName << " parent: " << parentUuid)

            _etUuidMap.insert(entityName, etUuid);
            _etNameMap.insert(etUuid, entityName);
            _etParentMap.insert(etUuid, parentUuid);

            QStringList etChildren = _etChildMap.value(parentUuid);
            etChildren.append(etUuid);
            _etChildMap.insert(parentUuid, etChildren);
        }
    } else {
        qWarning() << q.executedQuery();
        qWarning() << "result query failed: " << q.lastError().text();
    }

}


QString
EdbStorSql_EntityCache::getEntityName(QString eUuid)
{
    if (_eNameMap.contains(eUuid))
    {
        return _eNameMap.value(eUuid);
    }

    RLP_LOG_INFO(eUuid)

    QString result("");

    /*
    QString query(""
        "SELECT "
        "fv_v_str "
        "FROM "
        "edb_field_value "
        "WHERE "
        "fv_name = :name AND "
        "fv_e_uuid = :e_uuid"
    );
    */

   QString query(""
        "SELECT "
        "e_name "
        "FROM "
        "edb_entity "
        "WHERE "
        "e_uuid = :e_uuid"
    );

    QSqlQuery q(_conn->db());
    q.prepare(query);
    q.bindValue(":e_uuid", eUuid);
    if (q.exec() && q.next())
    {
        result = q.value(0).toString();
        _eNameMap.insert(eUuid, result);
    } else
    {
        RLP_LOG_WARN("No entity name found from" << eUuid)
    }

    return result;
}