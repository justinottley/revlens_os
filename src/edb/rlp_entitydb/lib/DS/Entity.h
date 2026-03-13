//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_ENTITY_H
#define EDB_DS_ENTITY_H

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/EdbObjectBase.h"
#include "RlpEdb/DS/EntityType.h"

class EDB_DS_API EdbDs_Entity : public EdbDs_ObjectBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbDs_Entity(
        QString etypeUuid,
        QString entityName="",
        QString uuid="",
        QVariantMap metadata=QVariantMap(),
        int createDate=0,
        QString createUser="",
        int updateDate=0,
        QString updateUser=""
    );

    EdbDs_Entity(
        EdbDs_EntityType* etype,
        QString entityName,
        QString entityUuid,
        bool autoCreate=true
    );

    EdbDs_Entity(
        EdbDs_EntityType* etype,
        QString entityName="",
        bool autoCreate=true
    );

    EdbDs_Entity();
    
    EdbDs_Entity(const EdbDs_Entity& entity);
    
    EdbDs_EntityType* etHandle() { return _etHandle; }


public slots:

    QString typeName();
    
    QString et_uuid();
    void set_et_uuid(QString etUuid) { _etUuid = etUuid; }
    
    bool setFieldValue(QString fName, QString fVal);
    bool setFieldValue(QString fName, int fVal);
    bool setFieldValue(QString fName, qlonglong fVal);
    bool setFieldValue(QString fName, double val);
    bool setFieldValue(QString fName, QUuid eUuid);
    bool setFieldValue(QString fName, QByteArray fVal);
    bool setFieldValue(QString fName, EdbDs_Entity* fVal);

    bool setFieldValue(QString fName, QVariant val);


private:

    QString _etUuid;

    EdbDs_EntityType* _etHandle;
};


Q_DECLARE_METATYPE(EdbDs_Entity);

#endif