//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_FIELDTYPE_H
#define EDB_DS_FIELDTYPE_H

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/EdbObjectBase.h"

class EdbDs_EntityType;

enum FtValType {
    FT_VAL_SINGLE,
    FT_VAL_LIST
};

class EDB_DS_API EdbDs_FieldType : public EdbDs_ObjectBase {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    EdbDs_FieldType(
        QString name,
        QString displayName="",
        QString typeName="",
        FtValType valType=FT_VAL_SINGLE,
        QString uuid="",
        QString etypeUuid="",
        // QString fieldUuid="",
        EdbDs_EntityType* et=nullptr
    );
    EdbDs_FieldType(QString name, EdbDs_EntityType* et);
    EdbDs_FieldType();
    EdbDs_FieldType(const EdbDs_FieldType&);

public slots:

    QString display_name() { return _displayName; }
    void set_display_name(QString displayName) { _displayName = displayName; }

    // QStringList allowed_types() { return _allowedTypes; }
    // void set_allowed_types(QStringList allowedTypes) { _allowedTypes = allowedTypes; }

    FtValType valtype() { return _valType; }
    void set_valtype(FtValType valType) {}
    
    QString type_name() { return _typeName; }
    void set_type_name(QString typeName) { _typeName = typeName; }
    
    QString etype_uuid();
    void set_etype_uuid(QString euuid) { _etypeUuid = euuid; }
    
    // QString field_uuid() { return _fieldUuid; }
    // void set_field_uuid(QString fuuid) { _fieldUuid = fuuid; }
    
    QJsonObject toJson();


    EdbDs_EntityType* entityType() { return _etHandle; }
    
    bool removeFieldType();


private:

    QString _displayName;
    // QStringList _allowedTypes;
    FtValType _valType;
    QString _typeName;

    QString _etypeUuid;

    // QString _fieldUuid;
    
    QJsonObject _metadata;

    EdbDs_EntityType* _etHandle;

};

Q_DECLARE_METATYPE(EdbDs_FieldType);

#endif