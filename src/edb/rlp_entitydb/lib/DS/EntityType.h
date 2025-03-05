//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_ENTITYTYPE_H
#define EDB_DS_ENTITYTYPE_H


#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/EdbObjectBase.h"
#include "RlpEdb/DS/FieldType.h"

class EdbDs_DataSource;

class EDB_DS_API EdbDs_EntityType : public EdbDs_ObjectBase {
    Q_OBJECT
    
public:
    RLP_DEFINE_LOGGER

    EdbDs_EntityType(
        QString name,
        QVariantList fieldData={},
        QString uuid="",
        QString dtsUuid="",
        EdbDs_DataSource* dts=nullptr);

    EdbDs_EntityType(
        QString name,
        EdbDs_DataSource* dts,
        EdbDs_EntityType* parent=nullptr);

    EdbDs_EntityType();
    EdbDs_EntityType(const EdbDs_EntityType&);

    EdbDs_FieldType field(QString name);

    

public slots:
    
    QVariantMap field_data() { return _fieldData; }
    void set_field_data(QVariantMap fieldData) { _fieldData = fieldData; }

    QString dts_uuid();
    void set_dts_uuid(QString uuid) { _dtsUuid = uuid; if (_autoSave) { saveOne("dts_uuid", uuid); } }

    QString parent_uuid();
    void set_parent_uuid(QString parentUuid) { _parentUuid = parentUuid; if (_autoSave) { saveOne("parent_uuid", parentUuid); } }

    void setParent(EdbDs_EntityType* p) { _parentHandle = p; }


    EdbDs_FieldType* addField(
        QString fieldName,
        QString displayName,
        QString typeName,
        FtValType valType=FT_VAL_SINGLE
    );

    bool removeField(QString fieldName);

    EdbDs_DataSource* dataSource() { return _dtsHandle; }
    EdbDs_EntityType* parent() { return _parentHandle; }
    
    QString typeName();

private:

    QVariantMap _fieldData;
    
    QString _dtsUuid;
    QString _parentUuid; // TODO FIXME: DEPRECATE THIS, if parent uuid is available, convert to object
    
    EdbDs_DataSource* _dtsHandle;
    EdbDs_EntityType* _parentHandle;
    

};

Q_DECLARE_METATYPE(EdbDs_EntityType);

#endif