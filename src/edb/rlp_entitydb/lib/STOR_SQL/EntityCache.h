
#ifndef EDB_STOR_SQL_ENTITY_CACHE_H
#define EDB_STOR_SQL_ENTITY_CACHE_H

#include "RlpEdb/DS/DataSource.h"
#include "RlpEdb/DS/EntityType.h"

#include "RlpEdb/STOR_SQL/Global.h"
#include "RlpEdb/STOR_SQL/Connection.h"

class EDB_STOR_SQL_API EdbStorSql_EntityCache : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbStorSql_EntityCache(EdbStorSql_Connection* conn);

    void setDataSource(EdbDs_DataSource* dts);

    void registerAllEntityTypes();


public slots:

    static EdbStorSql_EntityCache* getCache(EdbStorSql_Connection* conn);

    //
    // Entity Type
    //

    void registerEntityType(QString etName);

    QStringList childEntityTypes(QString etName);
    QStringList parentEntityTypes(QString etName);

    QString getEntityTypeUuid(QString etName);
    QString getEntityTypeName(QString etUuid);
    EdbDs_EntityType* getEntityTypeObj(QString etName);

    QVariantMap getEntityTypeFields(QString etName, bool parents=true);

    QVariantList getEntityTypes();
    
    //
    // Entity
    //
    QString getEntityName(QString eUuid);

private:

    QVariantMap getEntityTypeFieldsOneEType(QString etName);

    void initBuiltinFields();
    
    void registerEntityByName(QString etName);
    void registerEntityByParentUuid(QString etUuid);

    // void registerFields(QString etUuid);

private:
    EdbStorSql_Connection* _conn;
    EdbDs_DataSource* _dts; // used for making / caching EdbDs_EntityType objects

    QMap<QString, QString> _etUuidMap; // EntityType name -> uuid
    QMap<QString, QString> _etNameMap; // EntityType uuid -> name
    QMap<QString, QStringList> _etChildMap; // EntityType uuid -> child uuid
    QMap<QString, QString> _etParentMap; // EntityType uuid -> parent uuid

    QMap<QString, QVariantMap> _etypeMap; // EntityType name -> all DB info

    QMap<QString, EdbDs_EntityType*> _etObjMap; // EntityType name -> EdbDs_EntityType object

    QMap<QString, QString> _eNameMap; // Entity uuid -> name

    // QMap<QString, QSet<QString> > _etFieldMap; // EntityType uuid -> fields

    QSet<QString> BUILTIN_FIELDS;

    static QMap<QString, EdbStorSql_EntityCache*> _cacheMap;
    static QMutex* _CMUTEX;

};

#endif
