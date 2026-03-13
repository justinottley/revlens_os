
#include "RlpEdb/DS/EntityType.h"
#include "RlpEdb/DS/DataSource.h"
#include "RlpEdb/DS/Global.h"

RLP_SETUP_LOGGER(edb, EdbDs, EntityType)

EdbDs_EntityType::EdbDs_EntityType(
    QString name,
    QVariantList fieldList,
    QString uuidStr,
    QString dtsUuidStr,
    EdbDs_DataSource* dts):
        EdbDs_ObjectBase::EdbDs_ObjectBase(
            name,
            uuidStr
        ),
        _dtsUuid(dtsUuidStr),
        _dtsHandle(dts)
{
	for(int i=0; i != fieldList.size(); ++i) {
		
		EdbDs_FieldType ft = fieldList.at(i).value<EdbDs_FieldType>();
		
		_fieldData.insert(ft.name(), fieldList.at(i));
        
	}
}


EdbDs_EntityType::EdbDs_EntityType():
    EdbDs_ObjectBase::EdbDs_ObjectBase(),
    _fieldData(QVariantMap()),
    _dtsUuid("")
{
}



EdbDs_EntityType::EdbDs_EntityType(
    QString name,
    EdbDs_DataSource* dts,
    EdbDs_EntityType* parent):
        EdbDs_ObjectBase::EdbDs_ObjectBase(
            name
    ),
    _fieldData(QVariantMap()),
    _dtsHandle(dts),
    _parentHandle(parent),
    _parentUuid("")
{
	if (dts->bound()) {
		bindStorage(dts->storageHandle()->mgr());
	}
}


EdbDs_EntityType::EdbDs_EntityType(const EdbDs_EntityType& etIn):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        const_cast<EdbDs_EntityType*>(&etIn)->name(),
        const_cast<EdbDs_EntityType*>(&etIn)->uuid()
    ),
    _fieldData(const_cast<EdbDs_EntityType*>(&etIn)->field_data()),
    _dtsUuid(const_cast<EdbDs_EntityType*>(&etIn)->dts_uuid())
{
    
}


QString
EdbDs_EntityType::dts_uuid()
{
	 
    if (_dtsHandle != nullptr) {
        return _dtsHandle->uuid();
    }
    
    return _dtsUuid;
}


QString
EdbDs_EntityType::parent_uuid()
{
	if (_parentHandle != nullptr) {
        return _parentHandle->uuid();
    }
    
    return _parentUuid;
	
}


EdbDs_FieldType*
EdbDs_EntityType::addField(
    QString fieldName,
    QString displayName,
    QString typeName,
    FtValType valType)
{
    RLP_LOG_VERBOSE(fieldName << "Display Name:" << displayName << "Type:" << typeName)

	EdbDs_FieldType* ft = new EdbDs_FieldType(
	    fieldName,
	    displayName,
	    typeName,
	    valType,
	    "",
	    "",
	    this
	);
	
	if (_sthandle) {
		ft->bindStorage(_sthandle->mgr());
		// ft.save();
	}
	
	QVariant v;
	v.setValue(ft);
	_fieldData.insert(ft->name(), v);
	
	return ft;
}


bool
EdbDs_EntityType::removeField(
    QString fieldName)
{
    RLP_LOG_DEBUG(fieldName)

    EdbDs_FieldType ft(fieldName, this);
    ft.bindStorage(_sthandle->mgr());

    bool result = ft.removeFieldType();

    return result;
}



EdbDs_FieldType
EdbDs_EntityType::field(QString fieldName)
{
    return _fieldData.value(fieldName).value<EdbDs_FieldType>();
}


QString
EdbDs_EntityType::typeName()
{
	return "entity";
}