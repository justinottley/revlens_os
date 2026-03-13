//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#include "RlpCore/UTIL/Convert.h"

#include "RlpEdb/DS/FieldType.h"
#include "RlpEdb/DS/EntityType.h"
#include "RlpEdb/DS/Global.h"


RLP_SETUP_LOGGER(edb, EdbDs, FieldType)

EdbDs_FieldType::EdbDs_FieldType(
    QString name,
    QString displayName,
    QString typeName,
    FtValType valType,
    QString uuid,
    QString entityUuid,
    // QString fieldUuid,
    EdbDs_EntityType* et
):
   EdbDs_ObjectBase::EdbDs_ObjectBase(
        name,
        uuid
    ),
    _displayName(displayName),
    _typeName(typeName),
    _valType(valType),
    _etypeUuid(entityUuid),
    // _fieldUuid(fieldUuid),
    _etHandle(et)
{
    if (_displayName == "")
    {
        _displayName = _name;
    }
}


EdbDs_FieldType::EdbDs_FieldType(
    QString name,
    EdbDs_EntityType* et):
        EdbDs_ObjectBase::EdbDs_ObjectBase(
            name,
            ""
        ),
        _displayName(""),
        _typeName(""),
        _valType(FT_VAL_SINGLE),
        _etypeUuid(""),
        // _fieldUuid(""),
        _etHandle(et)
{
}


EdbDs_FieldType::EdbDs_FieldType():
    EdbDs_ObjectBase::EdbDs_ObjectBase(),
    _displayName(""),
    _typeName(""),
    _valType(FT_VAL_SINGLE),
    _etypeUuid(""),
    // _fieldUuid(""),
    _etHandle(nullptr)
    
{
}


EdbDs_FieldType::EdbDs_FieldType(const EdbDs_FieldType& ftIn):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        const_cast<EdbDs_FieldType*>(&ftIn)->name(),
        const_cast<EdbDs_FieldType*>(&ftIn)->uuid()
    ),
    _displayName(const_cast<EdbDs_FieldType*>(&ftIn)->display_name()),
    _typeName(const_cast<EdbDs_FieldType*>(&ftIn)->type_name()),
    _valType(const_cast<EdbDs_FieldType*>(&ftIn)->valtype()),
    _etypeUuid(const_cast<EdbDs_FieldType*>(&ftIn)->etype_uuid()),
    // _fieldUuid(const_cast<EdbDs_FieldType*>(&ftIn)->field_uuid()),
    _etHandle(const_cast<EdbDs_FieldType*>(&ftIn)->entityType())
{
    
}


QString
EdbDs_FieldType::etype_uuid()
{
	if (_etHandle != nullptr) {
		return _etHandle->uuid();
	}
	
	return _etypeUuid;
	
}


bool
EdbDs_FieldType::removeFieldType()
{
    RLP_LOG_DEBUG("")

    if (_sthandle != nullptr)
    {
        RLP_LOG_DEBUG("calling remove() " << _sthandle->metaObject()->className())

        return _sthandle->remove();

    } else
    {
        RLP_LOG_ERROR("cannot remove, no bound FieldType storage handle")
    }

    return false;
}


QJsonObject
EdbDs_FieldType::toJson()
{
	QJsonObject result;
	result.insert("display_name", _displayName);
	// result.insert("allowed_types", _allowedTypes);
	// result.insert("numval_type", _numValues);
	
	return result;
	
	
}