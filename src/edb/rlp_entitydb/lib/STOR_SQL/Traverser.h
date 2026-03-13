//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_STOR_SQL_TRAVERSER_H
#define EDB_STOR_SQL_TRAVERSER_H

#include "RlpEdb/GRAPH/TraverserBase.h"
#include "RlpEdb/GRAPH/Node.h"

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"
#include "RlpEdb/STOR_SQL/EntityCache.h"

class EDB_STOR_SQL_API EdbStorSql_Traverser : public EdbGraph_TraverserBase {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    EdbStorSql_Traverser(EdbStorSql_Connection* conn);
    
    
public slots:

    bool update(
        QString etypeName,
        QString euuid,
        QVariantMap fields
    );

    bool deleteEntityType(
        QString etypeName
    );

    bool deleteEntity(
        QString etypeName,
        QString euuid
    );

    bool deleteEntityTypeField(
        QString etypeName,
        QString fieldName
    );

    bool deleteField(
        QString euuid,
        QString fieldName
    );

    QVariantList find(
        QString etypeName,
        QVariantList filters,
        QStringList returnFields,
        int limit=-1,
        QVariantList orderBy=QVariantList(),
        bool considerChildTypes=true
    );

    QVariantList findEvent(
        QString evtTypeName, // Event or EventQueue
        QVariantList filters,
        QStringList returnFields,
        int limit=-1
    );

    QVariantMap findByEntityUuid(QVariantList uuidList);

    int getMaxEntityId(QString etypeName);

    void traverse(EdbGraph_Path path);
    
    EdbStorSql_Connection* conn() { return _conn; }
    EdbStorSql_EntityCache* ecache() { return _ecache; }

private:

    void buildResultByFieldValue(
        QSqlQuery* qv,
        QVariantMap* result,
        QString retField
    );


    void buildResultByEntity(
        QString etypeUuid,
        QStringList returnFields,
        QStringList* uuids,
        QVariantMap* result
    );


    void buildResult(
        QString etypeUuid,
        QStringList returnFields,
        QStringList* uuids,
        QVariantMap* result
    );
    
    
    void prepFindByFieldValue(
        QString fName,
        QString op,
        QVariant val,
        QSqlQuery* query
    );
    
    
    void prepFindByEntity(
        QString fName,
        QString op,
        QVariant val,
        QSqlQuery* query
    );


    void findAllByEntityType(
        QString etypeName,
        QSqlQuery* query,
        QSet<QString>* eUuids
    );


    QVariantList findOneEType(
        QString etypeName,
        QVariantList filters,
        QStringList returnFields
    );


    void sortResult(
        QVariantMap* resultIn,
        QVariantList* resultOut
    );

private:

    EdbStorSql_Connection* _conn;
    EdbStorSql_EntityCache* _ecache;
    
};

#endif
