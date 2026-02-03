//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_FIELDVALUE_H
#define EDB_DS_FIELDVALUE_H

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/EdbObjectBase.h"

class EdbDs_Entity;

class EDB_DS_API EdbDs_FieldValue : public EdbDs_ObjectBase {
    Q_OBJECT

public:

    EdbDs_FieldValue(
        QString fvName,
        QString uuid="",
        QVariantMap metadata=QVariantMap(),
        QString entityUuid="",
        int createDate=0,
        QString createUser="",
        int updateDate=0,
        QString updateUser=""
    );

    EdbDs_FieldValue(
        EdbDs_Entity* entity,
        QString fName,
        QVariant fVal,
        QString typeName
    );

    EdbDs_FieldValue(
        EdbDs_Entity* entity,
        QString fName,
        bool autoCreate=false
    );

    EdbDs_Entity* entityHandle() { return _entityHandle; }
    QString type_name() { return _typeName; }
    QVariant val() { return _fVal; }


public slots:

    QString e_uuid();
    void set_e_uuid(QString entityUuid) { _entityUuid = entityUuid; }
    
    QString et_uuid();


private:

    QVariant _fVal;
    QString _typeName;
    
    QString _entityUuid;
    EdbDs_Entity* _entityHandle;
};


#endif