

#include "RlpEdb/DS/Entity.h"
#include "RlpEdb/STOR_SQL/Traverser.h"

RLP_SETUP_LOGGER(edb, EdbStorSql, Traverser)

EdbStorSql_Traverser::EdbStorSql_Traverser(
    EdbStorSql_Connection* conn):
        _conn(conn)
{
    _ecache = EdbStorSql_EntityCache::getCache(conn);
}


int
EdbStorSql_Traverser::getMaxEntityId(QString etypeName)
{
    RLP_LOG_DEBUG(etypeName)

    QSqlQuery q(_conn->db());

    if ((etypeName == "Event") || (etypeName == "EventQueue"))
    {
        QString tableName;
        QString colPrefix;

        if (etypeName == "Event")
        {
            tableName = "edb_event";
            colPrefix = "evt_";

        } else
        {
            tableName = "edb_event_queue";
            colPrefix = "evtq_";
        }

        QString qStr(""
            "SELECT max(" + colPrefix + "rowid)"
            " FROM "
            + tableName
        );

        q.prepare(qStr);

    } else
    {
        QString etypeUuid = _ecache->getEntityTypeUuid(etypeName);

        QString qStr(""
            "SELECT max(e_rowid) "
            "FROM edb_entity "
            "WHERE "
            "e_et_uuid = :et_uuid"
        );

        q.prepare(qStr);
        q.bindValue(":et_uuid", etypeUuid);

    }
    
    int result = -1;
    RLP_LOG_DEBUG(q.executedQuery())

    if (q.exec())
    {
        RLP_LOG_DEBUG("OK")

        if (q.first()) {
            
            RLP_LOG_DEBUG(q.value(0))

            result = q.value(0).toInt();

            RLP_LOG_DEBUG("GOT RESULT: " << result)
        }
    }

    return result;
}


void
EdbStorSql_Traverser::prepFindByEntity(
    QString fName,
    QString op,
    QVariant val,
    QSqlQuery* q)
{
    // RLP_LOG_DEBUG(fName << op << val)

	QString valStr = val.typeName();
	QString qOp;

    QString qStr;

    if (op == "is")
    {
        qOp = " = ";

        qStr = QString(""
            "SELECT e_uuid "
            "FROM edb_entity "
            "WHERE "
            "e_" + fName + " " + qOp + " :val"
            " AND "
            "e_et_uuid = :et_uuid"
        );

        q->prepare(qStr);
        q->bindValue(":val", val);


    } else if (op == "in")
    {
        qOp = " IN ";
        qStr = QString(""
            "SELECT e_uuid "
            "FROM edb_entity "
            "WHERE "
            "e_" + fName + " " + qOp + " "
        );

        QStringList valList;
        QVariantList inList = val.toList();

        for (int i=0; i != inList.size(); ++i)
        {
            valList.push_back("'" + inList.at(i).toString() + "'");
        }
        
        qStr += "(" + valList.join(",") + ")";

        q->prepare(qStr);
    }
    
	
}



void
EdbStorSql_Traverser::prepFindByFieldValue(
    QString fName,
    QString op,
    QVariant val,
    QSqlQuery* q)
{
    QString valStr = val.typeName();
    
    RLP_LOG_DEBUG("op:" << op << "valStr: " << valStr)
    
    QString qStr(""
        "SELECT fv_e_uuid "
        "FROM edb_field_value WHERE "
        "fv_name = :name AND "
        "fv_et_uuid = :et_uuid AND "
    );
    
    if (valStr == "QList<QUuid>") {
        
        qStr += "fv_v_e_uuid IN ";

        QList<QUuid> ulist = val.value<QList<QUuid> >();
        
        QList<QUuid>::iterator it;
        QStringList valResult;
        for (it = ulist.begin(); it != ulist.end(); ++it) {
            valResult.append("'" + it->toString() + "'");
        }
        
        qStr += "(" + valResult.join(",") + ")";
        
        q->prepare(qStr);
        
        
    } else if (valStr == "EdbDs_Entity") {
        
        EdbDs_Entity fv = val.value<EdbDs_Entity>();
        
        // qInfo() << "entity uuid: " << fv.uuid();
        
        qStr += "fv_v_e_uuid = :e_uuid ";
        
        // qStr += " AND "
        // qStr += "fv_v_et_uuid = :et_uuid "
        
        // qStr += "fv_v_dts_uuid = :dts_uuid; ";
        
        q->prepare(qStr);
        
        q->bindValue(":e_uuid", fv.uuid());
        // q.bindValue(":et_uuid", fv.value("et_uuid"));
        
        // q.bindValue(":dts_uuid", fv.value("dts_uuid"));
        
    } else if (valStr == "QUuid") {

        // RLP_LOG_DEBUG("found QUuid type")

        qStr += "fv_v_e_uuid = :e_uuid ";
        q->prepare(qStr);
        q->bindValue(":e_uuid", val);


    } else if (valStr == "QVariantMap") {

        QVariantMap valm = val.toMap();
        if (valm.contains("uuid"))
        {
            // RLP_LOG_DEBUG("FOUND uuid WRAPPED")
            QString uval = valm.value("uuid").toString();

            qStr += "fv_v_e_uuid = :e_uuid ";
            q->prepare(qStr);
            q->bindValue(":e_uuid", uval);

        }
    }  else if ((valStr == "QVariantList") ||
                (valStr == "QStringList"))
    {
        QVariantList valList = val.toList();

        // use the first entry in the list as the type for all entries
        QString listType = valList.at(0).typeName();
        QStringList valResult;

        if (listType == "QVariantMap")
        {
            for (int i = 0; i != valList.size(); ++i)
            {
                
                QVariantMap vm = valList.at(i).toMap();
                if (vm.contains("uuid"))
                {
                    QString uval = "'" + vm.value("uuid").toString() + "'";
                    valResult.append(uval);
                }

                // RLP_LOG_DEBUG("GOT:" << vm);
            }

            qStr += "fv_v_e_uuid IN (" + valResult.join(",") + ")";

        } else if ((listType == "qlonglong") ||
                   (listType == "int"))
        {
            for (int i=0; i != valList.size(); ++i)
            {
                QString sv("%1");
                sv = sv.arg(valList.at(i).toLongLong());
                valResult.append(sv);
            }

            qStr += "fv_v_int IN (" + valResult.join(",") + ")";

        } else if (listType == "QString")
        {
            for (int i=0; i != valList.size(); ++i)
            {
                QString sv("'%1'");
                sv = sv.arg(valList.at(i).toString());
                valResult.append(sv);
            }

            qStr += "fv_v_str IN (" + valResult.join(",") + ")";
        }

        RLP_LOG_VERBOSE(qStr)

        q->prepare(qStr);

    } else {
        
        QString qOp;
        if (op == "is") {
            qOp = " = ";
            
        } else if (op == "in") {
            qOp = " IN ";
            
        }
        QString colName = "";
            
        if (valStr == "QString") {
            colName += "fv_v_str";
            
        } else if (
            (valStr == "int") ||
            (valStr == "qlonglong"))
        {
            colName += "fv_v_int";
            
        } else if (valStr == "double") {
            colName += "fv_v_float";
            
        }
        
        qStr += colName + " " + qOp + " :val";
        
        q->prepare(qStr);
        q->bindValue(":val", val);
    }
    
}


void
EdbStorSql_Traverser::buildResultByFieldValue(
    QSqlQuery* qv,
    QVariantMap* result,
    QString retField)
{
    // RLP_LOG_DEBUG("")

    qv->bindValue(":retfield", retField);
        
    if (qv->exec()) {
        while (qv->next()) {
            
            QString retFieldName = qv->value(0).toString();
            QString typeStr = qv->value(3).toString();
            QString eUuid = qv->value(1).toString();
            // QString etUuid = qv->value(2).toString();

            // Add uuid and map to this entity if this is the first field value
            // of the entity that has been returned
            //
            if (!result->contains(eUuid))
            {
                
                QVariantMap eResult;
                eResult.insert("uuid", eUuid);
                result->insert(eUuid, eResult);
            }
            
            QVariantMap rr = result->value(eUuid).toMap();
            
            if (typeStr == "entity")
            {
                QUuid eUuid = QUuid(qv->value("fv_v_e_uuid").toString());
                QString etypeName = _ecache->getEntityTypeName(
                    qv->value("fv_v_et_uuid").toString());

                QString eName = _ecache->getEntityName(eUuid.toString());

                QVariantMap eresult;
                eresult.insert("uuid", eUuid);
                eresult.insert("et_name", etypeName);
                eresult.insert("name", eName);

                // RLP_LOG_INFO("!!!!!" << eresult)
                rr.insert(retFieldName, eresult);

            } else
            {

                QString retcol = "fv_v_" + typeStr;
                if (typeStr == "double")
                {
                    retcol = "fv_v_float";
                }

                // RLP_LOG_DEBUG("inserting " << retFieldName)

                rr.insert(
                    retFieldName,
                    qv->value(retcol)
                );
                
            }
            
            result->insert(eUuid, rr);
        }
    } else {
        
        qWarning() << qv->executedQuery();
        qWarning() << "result query failed: " << qv->lastError().text();
        
    }

}


void
EdbStorSql_Traverser::buildResultByEntity(
    QString etypeUuid,
    QStringList returnFields,
    QStringList* uuids,
    QVariantMap* result)
{
    QString qestr(""
        "SELECT "
        "e_uuid, "
        "e_name, "
        "e_metadata, "
        "e_create_date, "
        "e_create_user, "
        "e_update_date, "
        "e_update_user, "
        "e_rowid "
        " FROM edb_entity "
        " WHERE "
        " e_et_uuid = :et_uuid AND "
        " e_uuid IN "
    );

    // TODO FIXME: injection
    //
    QStringList nUuids;
    for (int i=0; i != uuids->size(); ++i) {
        nUuids.push_back("'" + uuids->at(i) + "'");
    }
    qestr += "(" + nUuids.join(",") + ")";

    // qInfo() << "buildResultByEntity(): ";
    // qInfo() << qestr;

    QSqlQuery qe(_conn->db());
    qe.prepare(qestr);
    qe.bindValue(":et_uuid", etypeUuid);
    

    // QStringList builtinFields = {"name", "metadata", "create_date", "create_user", "update_date", "update_user"};

    QStringList::iterator rit;

    if (qe.exec()) {
        while (qe.next()) {
            
            QVariantMap res;
            QString eUuid = qe.value(0).toString();

            if (result->contains(eUuid)) {
                res = result->value(eUuid).toMap();
            
            } else {
                res.insert("uuid", eUuid);

            }

            for (rit = returnFields.begin(); rit != returnFields.end(); ++rit) {

                QString rFieldName = *rit;

                
                bool fFound = true;

                if (rFieldName == "name") {
                    res.insert("name", qe.value(1).toString());

                } else if (rFieldName == "metadata") {
                    // TODO: JSON deserialize
                    res.insert("metadata", qe.value(2).toString());

                } else if (rFieldName == "create_date") {
                    res.insert("create_date", qe.value(3).toInt());

                } else if (rFieldName == "create_user") {
                    res.insert("create_user", qe.value(4).toString());

                } else if (rFieldName == "update_date") {
                    res.insert("update_date", qe.value(5).toInt());

                } else if (rFieldName == "update_user") {
                    res.insert("update_user", qe.value(6).toString());

                } else if (rFieldName == "rowid") {
                    res.insert("rowid", qe.value(7).toInt());

                } else {
                    fFound = false;
                }

                if (fFound) {
                    RLP_LOG_DEBUG("inserting " << rFieldName)
                
                }
            }

            result->insert(eUuid, res);

        }
    }
            
    
}


void
EdbStorSql_Traverser::buildResult(
    QString etypeUuid,
    QStringList returnFields,
    QStringList* uuids,
    QVariantMap* result)
{
    QString qvstr(""
        "SELECT "
        "fv_name,"
        "fv_e_uuid,"
        "fv_et_uuid," // Return et_uuid for later use in parent / child / subtype cases
        "fv_type_name,"
        "fv_v_int,"
        "fv_v_str,"
        "fv_v_float,"
        "fv_v_json,"
        "fv_v_binary,"
        "fv_v_e_uuid,"
        "fv_v_et_uuid,"
        "fv_v_dts_uuid"
        " FROM edb_field_value WHERE "
        " fv_et_uuid = :et_uuid AND "
        " fv_name = :retfield AND "
        " fv_e_uuid IN "
        
    );

    // qInfo() << "buildResult() et_uuid = " << etypeUuid;
    // qInfo() << "  retfields = " << returnFields;
    // qInfo() << "";

    // TODO FIXME: injection
    //
    QStringList nUuids;
    for (int i=0; i != uuids->size(); ++i) {
        nUuids.push_back("'" + uuids->at(i) + "'");
    }
    qvstr += "(" + nUuids.join(",") + ")";

    QSqlQuery qv(_conn->db());

    qv.prepare(qvstr);
    qv.bindValue(":et_uuid", etypeUuid);
    
    // qInfo() << "buildResult(): ";
    //  qInfo() << qvstr;
    
    QStringList::iterator rit;

    bool doEntityQuery = false;

    for (rit = returnFields.begin(); rit != returnFields.end(); ++rit)
    {
        QString returnFieldName = *rit;

        if ((returnFieldName == "name") ||
            (returnFieldName == "metadata") ||
            (returnFieldName == "create_date") ||
            (returnFieldName == "create_user") ||
            (returnFieldName == "update_date") ||
            (returnFieldName == "update_user") ||
            (returnFieldName == "rowid")
        ) {
            doEntityQuery = true;
            break;
        }
    }

    if (doEntityQuery) {
        buildResultByEntity(etypeUuid, returnFields, uuids, result);
    }

    for (rit = returnFields.begin(); rit != returnFields.end(); ++rit)
    {
        buildResultByFieldValue(&qv, result, *rit);
    }
}


void
EdbStorSql_Traverser::sortResult(
    QVariantMap* resultIn,
    QVariantList* resultOut)
{

    // TODO FIXME: no implementation yet, just return values

    QVariantMap::iterator it;
    for (it = resultIn->begin(); it != resultIn->end(); ++it) {
        resultOut->append(it.value());
    }
}



void
EdbStorSql_Traverser::findAllByEntityType(
    QString etypeName,
    QSqlQuery* query,
    QSet<QString>* eUuids)
{
    QString etypeUuid = _ecache->getEntityTypeUuid(etypeName);

    QString qStr(""
        "SELECT e_uuid FROM edb_entity "
        "WHERE e_et_uuid = :et_uuid");

    query->prepare(qStr);
    query->bindValue(":et_uuid", etypeUuid);

    if (query->exec()) {
        while(query->next()) {
            eUuids->insert(query->value(0).toString());
        }
    }

}


QVariantMap
EdbStorSql_Traverser::findByEntityUuid(QVariantList euuidList)
{
    QString qStr(""
        "SELECT "
        "e_uuid, "
        "e_name, "
        "e_metadata, "
        "e_et_uuid "
        "FROM edb_entity WHERE "
        "e_uuid IN "
    );

    QStringList valList;
    for (int i=0; i != euuidList.size(); ++i) {
        valList.push_back("'" + euuidList.at(i).toString() + "'");
    }
    
    qStr += "(" + valList.join(",") + ")";

    QSqlQuery q(_conn->db());
    q.prepare(qStr);
    
    QVariantMap result;

    if (q.exec())
    {

        while(q.next())
        {
            QVariantMap rr;
            QString eUuid = q.value(0).toString();
            QString etUuid = q.value(3).toString();
            QString etypeName = _ecache->getEntityTypeName(etUuid);

            rr.insert("name", q.value(1).toString());
            rr.insert("et_name", etypeName);
            rr.insert("uuid", eUuid);

            result.insert(eUuid, rr);
        }
    } else {

        qInfo() << q.lastError().text();
    }

    return result;
}


bool
EdbStorSql_Traverser::update(
    QString etypeName,
    QString euuid,
    QVariantMap fields)
{
    RLP_LOG_ERROR("NO IMPLEMENTATION, see EdbCntrl_Query::update")

    return false;
}


QVariantList
EdbStorSql_Traverser::findEvent(
    QString evtTypeName,
    QVariantList filters,
    QStringList returnFields,
    int limit)
{
    RLP_LOG_VERBOSE(evtTypeName)
    RLP_LOG_VERBOSE("__filters: " << filters)
    RLP_LOG_VERBOSE("__returnFields: " << returnFields)
    RLP_LOG_VERBOSE("__limit: " << limit)
    
    QString tableName;
    QString colPrefix;

    if (evtTypeName == "Event")
    {
        tableName = "edb_event";
        colPrefix = "evt_";

    } else if (evtTypeName == "EventQueue")
    {
        tableName = "edb_event_queue";
        colPrefix = "evtq_";
    }


    QString filterStr = " WHERE ";

    for (int fi=0; fi !=filters.size(); ++fi)
    {
        if (fi > 0)
        {
            filterStr += " AND ";
        }

        QVariantList filterOne;

        //
        // Support wrapping the filter in a map to work around a crash
        // bug on windows for filters using QVariantList
        // this seems to crash on Windows:
        // filters.append(EdbUtil_Filter::makeFilter(QVariantList({"label", "is", label})));
        //

        QString filterTypeName = filters.at(fi).typeName();
        if (filterTypeName == "QVariantMap")
        {
            QVariantMap fWrapper = filters.at(fi).value<QVariantMap>();
            filterOne.append(fWrapper.value("field").toString());
            filterOne.append(fWrapper.value("op").toString());
            filterOne.append(fWrapper.value("val"));
        }
        else
        {
            filterOne = filters.at(fi).toList();
        }



        QString valType = filterOne.at(2).typeName();

        RLP_LOG_DEBUG(filterOne << valType)

        if ((valType == "int") ||
            (valType == "double") ||
            (valType == "qlonglong"))
        {
            filterStr += QString("%1%2 %3 %4")
            .arg(colPrefix)
            .arg(filterOne.at(0).toString())
            .arg(filterOne.at(1).toString())
            .arg(filterOne.at(2).toLongLong());

        } else if (valType == "QString")
        {
            filterStr += QString("%1%2 %3 '%4'")
                .arg(colPrefix)
                .arg(filterOne.at(0).toString())
                .arg(filterOne.at(1).toString())
                .arg(filterOne.at(2).toString());
            
        }
        
    }

    QString returnStr = "";
    for (int ri=0; ri != returnFields.size(); ++ri)
    {
        if (ri > 0)
        {
            returnStr += " , ";
        }
    
        returnStr += colPrefix;
        returnStr += returnFields.at(ri);
    }

    QString qStr(""
        "SELECT "
        + returnStr +
        " FROM "
        + tableName
    );

    if (filters.size() > 0)
    {
        qStr += filterStr;
    }

    if (limit > 0)
    {
        qStr += QString(" LIMIT %1").arg(limit);
    }

    QSqlQuery q(_conn->db());
    q.prepare(qStr);

    RLP_LOG_DEBUG(q.executedQuery());

    QVariantList rresult;

    if (q.exec())
    {
        RLP_LOG_DEBUG("OK")

        while (q.next())
        {
            QVariantMap rrmap;

            for (int rri = 0; rri != returnFields.size(); ++rri)
            {
                rrmap.insert(
                    returnFields.at(rri),
                    q.value(rri)
                );

            }
            rresult.append(rrmap);
        }
        
    } else
    {
        RLP_LOG_ERROR("Event Query failed:" << q.lastError().text())
    }

    return rresult;
}


QVariantList
EdbStorSql_Traverser::findOneEType(
    QString etypeName,
    QVariantList filters,
    QStringList returnFields)
{
    // registerEntityType(etypeName);

	
	QString etypeUuid = _ecache->getEntityTypeUuid(etypeName);
	
    RLP_LOG_VERBOSE(etypeName << " uuid: " << etypeUuid)
    RLP_LOG_VERBOSE("__filters: " << filters)
    RLP_LOG_VERBOSE("__returnFields: " << returnFields)
    
    QSqlQuery q(_conn->db());
    
    QSet<QString> eUuids;
    
    if (filters.size() == 0) {
        findAllByEntityType(etypeName, &q, &eUuids);
    }

    for (int i=0; i != filters.size(); ++i) {

        // QVariantList f;
        //
        //
        // Support wrapping the filter in a map to work around a crash
        // bug on windows for filters using QVariantList
        // this seems to crash on Windows:
        // filters.append(EdbUtil_Filter::makeFilter(QVariantList({"label", "is", label})));
        //

        // QString filterTypeName = filters.at(i).typeName();
        // if (filterTypeName == "QVariantMap")
        // {
        //     QVariantMap fWrapper = filters.at(i).value<QVariantMap>();
        //     f.append(fWrapper.value("field").toString());
        //     f.append(fWrapper.value("op").toString());
        //     f.append(fWrapper.value("val"));
        // }
        // else
        // {
        //
        // }

        QVariantList f = filters.at(i).value<QVariantList>();


        QString fName = f.at(0).toString();
        
        if ((fName == "name") ||
            (fName == "uuid") ||
            (fName == "rowid"))
        {

            prepFindByEntity(
                fName,
                f.at(1).toString(),
                f.at(2),
                &q
            );

        } else
        {

            prepFindByFieldValue(
                fName,
                f.at(1).toString(),
                f.at(2),
                &q
            );
            
        }
            
        q.bindValue(":name", fName);
        q.bindValue(":et_uuid", etypeUuid);

        // qInfo() << ":name = " << fName << " et_uuid = " << etypeUuid;
        // qInfo() << q.executedQuery();

        if (q.exec()) {

            QSet<QString> subUuids;

            while (q.next()) {

                QString erUuid = q.value(0).toString();
                subUuids.insert(erUuid);

            }
            
            if (i == 0) {
                eUuids = subUuids;
                
            } else {
                eUuids.intersect(subUuids);
            }
            
            
        } else {
            
            RLP_LOG_ERROR("subquery failed: " << q.lastError().text())
            qInfo() << q.executedQuery();
        }
        
        
        
    }
        
    QStringList uuidResultList = eUuids.values();
    
    
    // qInfo() << "eUuids: " << eUuids;
    // qInfo() << "uuidresultlist: " << uuidResultList;
    
    QVariantMap result;
    QSet<QString>::iterator eit;
    for (eit = eUuids.begin(); eit != eUuids.end(); ++eit) {
        QVariantMap r;
        
        r.insert("uuid", *eit);
        // r.insert("et_uuid", etypeUuid);
        r.insert("et_name", etypeName);

        result.insert(*eit, r);
    }


    // RLP_LOG_VERBOSE("Raw Result:")
    // RLP_LOG_VERBOSE(result)

    buildResult(
        etypeUuid,
        returnFields,
        &uuidResultList,
        &result
    );
    
    QVariantList sortedResult;
    sortResult(&result, &sortedResult);

    // RLP_LOG_VERBOSE("Result:")
    // RLP_LOG_VERBOSE(result)
    // RLP_LOG_VERBOSE("")


    return sortedResult;
}


QVariantList
EdbStorSql_Traverser::find(
    QString etypeName,
    QVariantList filters,
    QStringList returnFields,
    int limit,
    QVariantList orderBy,
    bool considerChildTypes)
{

    if ((etypeName == "Event") || (etypeName == "EventQueue"))
    {
        return findEvent(etypeName, filters, returnFields, limit);
    }


    QStringList childTypes;
    if (considerChildTypes)
    {
        childTypes = _ecache->childEntityTypes(etypeName);
    }

    RLP_LOG_DEBUG(etypeName << " child types: " << childTypes);

    childTypes.insert(0, etypeName);

    QVariantList result;

    for (int i=0; i !=childTypes.size(); ++i) {
        result.append(
            findOneEType(
                childTypes.at(i),
                filters,
                returnFields
            )
        );
    }

    return result;
}



void
EdbStorSql_Traverser::traverse(EdbGraph_Path path)
{
	qInfo() << "traverse() " << path.size();
	
	for (int i=0; i != path.size(); ++i) {
		
		EdbGraph_NodePtr n = path.value(i);
		
		if (n->hasEdge()) {
			
			qInfo() << "node has edge, building query";
			
			EdbGraph_NodePtr srcNode = path.value(n->edgeLabel());
			QList<QUuid> resultUuids = srcNode->returnResultUuids();
			
			QVariant ulist;
			ulist.setValue(resultUuids);
			QVariantList edgeFilter = {n->edgeField(), "in", ulist};
			QVariantList filterList;
			filterList.append(QVariant(edgeFilter));
			
			// qInfo() << edgeFilter;
			
			QVariantList nResult = find(
			    n->entityType(),
			    filterList,
			    n->returnFields()
			);
			
			qInfo() << nResult;
			
		} else {
			
			qInfo() << "direct query detected";
			
			
			QVariantList nResult = find(
			    n->entityType(),
			    n->filters(),
			    n->returnFields()
			);
			
			n->setReturnResult(nResult);
			
			qInfo() << nResult;
			
		}
			
		
	}
	
}


bool
EdbStorSql_Traverser::deleteEntityType(QString etypeName)
{
    RLP_LOG_DEBUG(etypeName)

    bool result = true;


    // Get all fields of this type, delete all field typesQ
    //
    QVariantMap fieldInfo = _ecache->getEntityTypeFields(etypeName);

    QVariantMap::iterator it;

    bool fresult = true;
    for (it = fieldInfo.begin(); it != fieldInfo.end(); ++it) {
        QString fieldName = it.key();
        bool ftresult = deleteEntityTypeField(etypeName, fieldName);
        fresult = fresult && ftresult;
    }

    RLP_LOG_DEBUG("Field delete result:" << fresult)

    QString queryStr(""
        "DELETE from edb_entity_type"
        " WHERE et_name = :et_name"
    );

    QSqlQuery q(_conn->db());
    q.prepare(queryStr);
    q.bindValue(":et_name", etypeName);

    if (q.exec())
    {
        int numRows = q.numRowsAffected();

        RLP_LOG_DEBUG("Delete Entity Type - num rows affected:" << numRows)

        if (numRows != 1)
        {
            result = false;
        }
    }

    return result;

}


bool
EdbStorSql_Traverser::deleteEntityTypeField(QString etypeName, QString fieldName)
{
    RLP_LOG_DEBUG(etypeName << " " << fieldName)

    bool result = true;

    QString etypeUuid = _ecache->getEntityTypeUuid(etypeName);

    QString queryStr(""
        "DELETE from edb_field_type"
        " WHERE ft_et_uuid = :et_uuid"
        " AND ft_name = :f_name"
    );

    QSqlQuery q(_conn->db());
    q.prepare(queryStr);
    q.bindValue(":et_uuid", etypeUuid);
    q.bindValue(":f_name", fieldName);

    if (q.exec())
    {
        int numRows = q.numRowsAffected();

        RLP_LOG_DEBUG("Delete Entity Type Field: num rows affected:" << numRows)

        if (numRows != 1)
        {
            result = false;
        }
    }

    return result;
}


bool
EdbStorSql_Traverser::deleteEntity(QString etypeName, QString euuid)
{
    RLP_LOG_DEBUG(etypeName << " " << euuid)

    bool result = true;

    QString etypeUuid = _ecache->getEntityTypeUuid(etypeName);

    // Get all fields of this type, delete all fields
    //
    QVariantMap fieldInfo = _ecache->getEntityTypeFields(etypeName);

    QVariantMap::iterator it;

    for (it = fieldInfo.begin(); it != fieldInfo.end(); ++it) {
        QString fieldName = it.key();

        if (!deleteField(euuid, fieldName)) {
            result = false;
        }
    }


    QString queryStr(""
        "DELETE FROM edb_entity "
        " WHERE e_uuid = :uuid"
    );

    QSqlQuery q(_conn->db());
    q.prepare(queryStr);
    q.bindValue(":uuid", euuid);

    if (q.exec()) {
        
        int numRows = q.numRowsAffected();
        RLP_LOG_DEBUG("Deleted Entity " << euuid << " Num rows affected: " << numRows)

        if (numRows != 1) {
            result = false;
        }
    }

    return result;
}


bool
EdbStorSql_Traverser::deleteField(QString euuid, QString fieldName)
{
    RLP_LOG_DEBUG(fieldName << euuid)

    bool result = false;
    
    QString queryStr(""
        "DELETE FROM edb_field_value WHERE "
        "fv_e_uuid = :e_uuid AND "
        "fv_name = :name"
    );

    QSqlQuery q(_conn->db());
    q.prepare(queryStr);
    q.bindValue(":e_uuid", euuid);
    q.bindValue(":name", fieldName);

    RLP_LOG_VERBOSE(q.executedQuery());

    if (q.exec()) {

        int numAffected = q.numRowsAffected();
        RLP_LOG_DEBUG("Deleted " << numAffected << " rows")

        result = true;

    } else {

        RLP_LOG_ERROR("DELETE failed: " << q.lastError().text())

    }

    return result;
}

