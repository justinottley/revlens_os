

#include <chrono>

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/FieldValue.h"
#include "RlpEdb/DS/Entity.h"

EdbDs_FieldValue::EdbDs_FieldValue(
    QString fvName,
    QString uuid,
    QVariantMap metadata,
    QString entityUuid,
    int createDate,
    QString createUser,
    int updateDate,
    QString updateUser
):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        fvName,
        uuid,
        metadata,
        createDate,
        createUser,
        updateDate,
        updateUser
    ),
    _entityUuid(entityUuid)
{
}


EdbDs_FieldValue::EdbDs_FieldValue(
    EdbDs_Entity* entity,
    QString fName,
    QVariant fVal,
    QString typeName
):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        fName
    ),
    _entityHandle(entity),
    _fVal(fVal),
    _typeName(typeName)
{
	
	if (entity->bound()) {
		
		EdbDs_StorageHandleBase::BindResult br = bindStorage(
            entity->storageHandle()->mgr()
        );
        
        if (br == EdbDs_StorageHandleBase::BIND_EXISTING) {
            
            save();
            
        }
	}
	
	
    // TODO FIXME: Not cross platform?
    //
    // auto now = std::chrono::system_clock::now();
    
}



EdbDs_FieldValue::EdbDs_FieldValue(
    EdbDs_Entity* entity,
    QString fName,
    bool autoCreate):
        EdbDs_ObjectBase::EdbDs_ObjectBase(
            fName
        ),
        _entityHandle(entity)
{
    _autoCreate = autoCreate;
    
    if (entity->bound()) {
		
		EdbDs_StorageHandleBase::BindResult br = bindStorage(
            entity->storageHandle()->mgr()
        );
    }
}


QString
EdbDs_FieldValue::e_uuid()
{
	if (_entityHandle != nullptr) {
		return _entityHandle->uuid();
	}
	
	return _entityUuid;
}


QString
EdbDs_FieldValue::et_uuid()
{
    if (_entityHandle != nullptr) {
        return _entityHandle->et_uuid();
    }

    return QString("");
}
