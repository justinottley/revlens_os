//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_EDB_CNTRL_QUERY_H
#define CORE_EDB_CNTRL_QUERY_H

#include "RlpCore/CNTRL/ControllerBase.h"

#include "RlpEdb/DS/EntityType.h"

#include "RlpEdb/STOR_SQL/Traverser.h"
#include "RlpEdb/STOR_SQL/StorageManager.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/EntityCache.h"
#include "RlpEdb/STOR_SQL/Event.h"

#include "RlpEdb/CNTRL/Global.h"


class EDB_CNTRL_API EdbCntrl_Query : public CoreCntrl_ControllerBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    // TODO FIXME: passing datasource name and type are temporary for now
    //
    EdbCntrl_Query(QString fileName, QString dsName, QString dsType);

    EdbStorSql_StorageManager* mgr() { return _mgr; }
    EdbStorSql_Connection* conn() { return _conn; }
    EdbStorSql_EntityCache* ecache() { return _ecache; }

    QVariantList findSimple(
        QString etypeName,
        QString uuid,
        QStringList returnList
    );

    QVariantList findSimple(
        QString etypeName,
        QString uuid,
        QString retField
    );


public slots:

    QString client_type() { return "native"; }

    QVariantList find(
        QString etypeName,
        QVariantList filters,
        QStringList returnFields,
        int limit=-1,
        QVariantList orderBy=QVariantList(),
        bool considerChildTypes=true
    );

    QVariantMap findByEntityUuid(
        QVariantList uuidList
    );

    QString create(
        QString etypeName,
        QString eName,
        QVariantMap fields,
        QString uuid=""
    );

    bool update(
        QString etypeName,
        QString euuid,
        QVariantMap fields
    );

    bool deleteEntity(
        QString etypeName,
        QString euuid
    );


    bool deleteField(
        QString euuid,
        QString fieldName
    );


    bool deleteEntityType(
        QString etypeName
    );


    bool deleteEntityTypeField(
        QString etypeName,
        QString fieldName
    );


    bool createEntityType(QString entityType, QString parentEntityTypeName="");

    bool createEntityTypeField(
        QString entityType,
        QString fieldName,
        QString displayName,
        QString typeName,
        int valType=0, // FT_VAL_SINGLE,
        QVariantMap metadata=QVariantMap()
    );


    QVariantList getEntityTypes();
    
    QVariantMap getEntityTypeFields(
        QString entityType,
        bool parents=true
    );

    int getMaxEntityId(QString etypeName);
    int getMaxEventId();
    bool updateEvent(QString evtId, QString field, QVariant val);


    void refreshCache() { _ecache->registerAllEntityTypes(); }

    void handleRemoteCall(QVariantMap msgObj);

private:

    EdbStorSql_Connection* _conn;
    EdbStorSql_StorageManager* _mgr;
    EdbStorSql_Traverser* _traverser;
    EdbStorSql_EntityCache* _ecache;

    EdbDs_DataSource* _dataSource;

    EdbDs_Event* _nevent;

};


#endif
