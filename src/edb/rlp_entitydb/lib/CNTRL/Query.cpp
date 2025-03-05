
#include "RlpEdb/CNTRL/Query.h"

RLP_SETUP_LOGGER(edb, EdbCntrl, Query)

EdbCntrl_Query::EdbCntrl_Query(
    QString fsdbPath,
    QString dsName,
    QString dsType):
        CoreCntrl_ControllerBase()
{
    QFileInfo finfo(fsdbPath);
    _conn = new EdbStorSql_Connection(finfo.absoluteFilePath(), dsName);
    
    _mgr = new EdbStorSql_StorageManager(_conn);
    _traverser = new EdbStorSql_Traverser(_conn);

    _dataSource = new EdbDs_DataSource(dsName, dsType);
    _dataSource->bindStorage(_mgr);

    _ecache = EdbStorSql_EntityCache::getCache(_conn);
    _ecache->setDataSource(_dataSource);

    _nevent = new EdbDs_Event(_mgr, "init");
}


QVariantList
EdbCntrl_Query::getEntityTypes()
{
    return _ecache->getEntityTypes();
}


QVariantMap
EdbCntrl_Query::getEntityTypeFields(QString entityType, bool parents)
{
    return _ecache->getEntityTypeFields(entityType, parents);
}


int
EdbCntrl_Query::getMaxEntityId(QString etypeName)
{
    return _traverser->getMaxEntityId(etypeName);
}


int
EdbCntrl_Query::getMaxEventId()
{
    RLP_LOG_DEBUG("")

    EdbStorSql_EventHandler* eh = qobject_cast<EdbStorSql_EventHandler*>(_nevent->storageHandle());
    return eh->getMaxEventId();
}


bool
EdbCntrl_Query::updateEvent(QString evtId, QString field, QVariant val)
{
    RLP_LOG_DEBUG(evtId << field << "=" << val)

    EdbStorSql_EventHandler* eh = qobject_cast<EdbStorSql_EventHandler*>(_nevent->storageHandle());
    return eh->updateEvent(evtId, field, val);
}


QVariantList
EdbCntrl_Query::find(
    QString etypeName,
    QVariantList filters,
    QStringList returnFields,
    int limit,
    QVariantList orderBy,
    bool considerChildTypes)
{
    return _traverser->find(etypeName, filters, returnFields, limit, orderBy, considerChildTypes);
}


QVariantList
EdbCntrl_Query::findSimple(
    QString etypeName,
    QString uuid,
    QStringList returnFields)
{
    QVariantList filters;
    filters.append(QVariant(QVariantList({"uuid", "is", uuid})));

    return find(etypeName, filters, returnFields);
}


QVariantList
EdbCntrl_Query::findSimple(
    QString etypeName,
    QString uuid,
    QString returnField)
{
    QStringList retFields = {returnField};
    return findSimple(etypeName, uuid, retFields);
}


QVariantMap
EdbCntrl_Query::findByEntityUuid(
    QVariantList uuidList)
{
    RLP_LOG_DEBUG(uuidList)

    return _traverser->findByEntityUuid(uuidList);
}


QString
EdbCntrl_Query::create(
    QString etypeName,
    QString name,
    QVariantMap fields,
    QString uuid)
{
    // RLP_LOG_DEBUG(etypeName)

    EdbDs_EntityType et(etypeName, _dataSource);
    EdbDs_Entity entity(&et, name, uuid);

    QVariantMap::iterator fit;
    for (fit = fields.begin(); fit != fields.end(); ++fit) {

        // QUuid is used to represent an entity.. but cannot be seralized to JSON.
        // We expect an entity UUID as a wrapped dictionary. Convert to QUuid
        // here for compatibility
        //
        // TODO FIXME: revisit?
        //
        QString fieldName = fit.key();
        QVariant fval = fit.value();
        QString fvalTypeName = fval.typeName();

        // RLP_LOG_DEBUG(fieldName << fvalTypeName)

        if (fvalTypeName == "QVariantMap")
        {
            QVariantMap valDict = fval.toMap();
            QUuid fuuid = QUuid(valDict.value("uuid").toString());

            RLP_LOG_DEBUG("Converted to QUuid: " << fieldName << fuuid)

            fval.setValue(fuuid);
        }

        if (fieldName == "name")
        {
            entity.setName(fval.toString());

        } else if (fieldName == "metadata")
        {
            entity.setMetadata(fval.toMap());

        } else
        {
            entity.setFieldValue(fieldName, fval);
        }
    }

    return entity.uuid();
}


bool
EdbCntrl_Query::createEntityType(QString entityType, QString parentEntityTypeName)
{

    EdbDs_EntityType* parent = nullptr;
    if (parentEntityTypeName != "")
    {
        parent = _ecache->getEntityTypeObj(parentEntityTypeName);

        Q_ASSERT(parent != nullptr);
    }


    EdbDs_EntityType* et = new EdbDs_EntityType(entityType, _dataSource, parent);

    // refresh entity cache be re-reading everything
    //
    _ecache->registerAllEntityTypes();

    return true;
}


bool
EdbCntrl_Query::createEntityTypeField(
    QString entityType,
    QString fieldName,
    QString displayName,
    QString typeName,
    int valType, // FT_VAL_SINGLE
    QVariantMap metadata)
{

    RLP_LOG_DEBUG(entityType << "Name:" <<  fieldName << "Display Name:" << displayName << "Type Name:" << typeName << "Metadata:" << metadata)

    // allocate EntityType object on stack for mem free at end of function scope
    //
    EdbDs_EntityType et(entityType, _dataSource);

    EdbDs_FieldType* result = et.addField(
        fieldName,
        displayName,
        typeName,
        (FtValType)valType
    );

    result->setMetadata(metadata);

    // refresh entity cache be re-reading everything
    //
    _ecache->registerAllEntityTypes();

    // TODO FIXME: MEM LEAK for result?
    return true;
}


bool
EdbCntrl_Query::deleteEntityTypeField(
        QString entityType,
        QString fieldName)
{
    RLP_LOG_DEBUG(entityType << "Name:" <<  fieldName)

    // allocate EntityType object on stack for mem free at end of function scope
    //
    EdbDs_EntityType et(entityType, _dataSource);

    bool result = et.removeField(fieldName);

    return result;
}


bool
EdbCntrl_Query::update(
    QString etypeName,
    QString euuid,
    QVariantMap fields)
{
    RLP_LOG_DEBUG(etypeName << " " << euuid)

    EdbDs_EntityType et(etypeName, _dataSource);
    EdbDs_Entity entity(&et, "", euuid);

    bool result = true;

    QVariantMap::iterator fit;
    for (fit = fields.begin(); fit != fields.end(); ++fit) {
        bool fresult = entity.setFieldValue(fit.key(), fit.value());

        result = result && fresult;

        RLP_LOG_DEBUG(fit.key() << " : " << fresult)
    }

    return result;
}


bool
EdbCntrl_Query::deleteEntity(
    QString etypeName,
    QString euuid)
{
    return _traverser->deleteEntity(etypeName, euuid);    
}


bool
EdbCntrl_Query::deleteField(
    QString euuid,
    QString fieldName)
{
    return _traverser->deleteField(euuid, fieldName);
}


bool
EdbCntrl_Query::deleteEntityType(
    QString etypeName)
{
    return _traverser->deleteEntityType(etypeName);
}


/*
bool
EdbCntrl_Query::deleteEntityTypeField(
    QString etypeName,
    QString fieldName)
{
    return _traverser->deleteEntityTypeField(etypeName, fieldName);
}
*/

void
EdbCntrl_Query::handleRemoteCall(QVariantMap msgObj)
{
    LOG_Logging::pprint(msgObj);

    QVariantMap result;
    result.insert("ident", msgObj.value("ident"));

    if (msgObj.contains("session"))
    {
        QVariantMap sessionObj = msgObj.value("session").toMap();
        if (sessionObj.contains("run_id"))
        {
            result.insert("run_id", sessionObj.value("run_id"));
        }
    }

    QString methodName = msgObj.value("method").toString();
    QVariantList args = msgObj.value("args").toList();

    if (methodName == "find")
    {
        if (args.size() >= 3)
        {
            int limit = -1;
            QVariantList orderBy;

            if (args.size() > 3)
            {
                limit = args[3].toInt();
            }

            bool considerChildTypes = true;
            if (args.size() > 4)
            {
                considerChildTypes = args[4].toBool();
            }


            QVariantList findResult = find(
                args[0].toString(),
                args[1].toList(),
                args[2].value<QStringList>(),
                limit,
                orderBy,
                considerChildTypes
            );
            
            result.insert("status", "OK");
            result.insert("result", findResult);
        }
        else
        {
            result.insert("status", "ERR");
            result.insert("err_msg", "Invalid input args, expect 5");
        }
        
    }

    else if (methodName == "findByEntityUuid")
    {
        QVariantMap findResult = findByEntityUuid(
            args[0].toList()
        );
            
        result.insert("status", "OK");
        result.insert("result", findResult);
    }

    else if (methodName == "getEntityTypes")
    {
        QVariantList etListResult = getEntityTypes();

        // RLP_LOG_DEBUG(etListResult)

        result.insert("status", "OK");
        result.insert("result", etListResult);
    }

    else if (methodName == "create")
    {
        QString create_result = create(
            args[0].toString(),
            args[1].toString(),
            args[2].toMap(),
            args[3].toString()
        );

        result.insert("status", "OK");
        result.insert("result", create_result);
    }

    else if (methodName == "update")
    {
        bool update_result = update(
            args[0].toString(),
            args[1].toString(),
            args[2].toMap()
        );

        result.insert("status", "OK");
        result.insert("result", update_result);
    }

    else if (methodName == "deleteEntity")
    {
        bool delete_result = deleteEntity(
            args[0].toString(),
            args[1].toString()
        );

        result.insert("status", "OK");
        result.insert("result", delete_result);
    }

    else if (methodName == "createEntityType")
    {
        bool create_entity_type_result = createEntityType(
            args[0].toString(),
            args[1].toString()
        );

        result.insert("status", "OK");
        result.insert("result", create_entity_type_result);
    }

    else if (methodName == "getEntityTypeFields")
    {
        QString etypeName = args[0].toString();
        bool parents = args[1].toBool();

        QVariantMap fieldInfoResult = getEntityTypeFields(etypeName, parents);

        result.insert("status", "OK");
        result.insert("result", fieldInfoResult);
    }

    else if (methodName == "createEntityTypeField")
    {
        bool create_field_result = createEntityTypeField(
            args[0].toString(),
            args[1].toString(),
            args[2].toString(),
            args[3].toString(),
            args[4].toInt(),
            args[5].toMap()
        );

        result.insert("status", "OK");
        result.insert("result", create_field_result);
    }

    else if (methodName == "deleteEntityTypeField")
    {
        bool deleteResult = deleteEntityTypeField(
            args[0].toString(),
            args[1].toString()
        );

        result.insert("status", "OK");
        result.insert("result", deleteResult);
    }
    
    else if (methodName == "deleteEntityType")
    {
        bool deleteResult = deleteEntityType(
            args[0].toString()
        );

        result.insert("status", "OK");
        result.insert("result", deleteResult);
    }


    else if (methodName == "refreshCache")
    {
        refreshCache();

        result.insert("status", "OK");
    }


    else
    {
        QString errMsg = "Method \"";
        errMsg += methodName;
        errMsg += "\" not found";

        result.insert("status", "ERR");
        result.insert("err_msg", errMsg);
    }

    emit dataReady(result);
}