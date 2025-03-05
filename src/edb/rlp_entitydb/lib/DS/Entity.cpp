

// #include <chrono>

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/Entity.h"
#include "RlpEdb/DS/FieldValue.h"

RLP_SETUP_LOGGER(edb, EdbDs, Entity)

EdbDs_Entity::EdbDs_Entity(
    QString etUuid,
    QString entityName,
    QString uuid,
    QVariantMap metadata,
    int createDate,
    QString createUser,
    int updateDate,
    QString updateUser
):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        entityName,
        uuid,
        metadata,
        createDate,
        createUser,
        updateDate,
        updateUser
    ),
    _etUuid(etUuid)
{
}


EdbDs_Entity::EdbDs_Entity(
    EdbDs_EntityType* etype,
    QString entityName,
    QString entityUuid,
    bool autoCreate
):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        entityName,
        entityUuid
    ),
    _etHandle(etype)
{
    _autoCreate = autoCreate;

    if (etype->bound()) {
       bindStorage(etype->storageHandle()->mgr());
    }
}


EdbDs_Entity::EdbDs_Entity(
    EdbDs_EntityType* etype,
    QString entityName,
    bool autoCreate
):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        entityName
    ),
    _etHandle(etype)
{
    _autoCreate = autoCreate;

    if (etype->bound()) {
		bindStorage(etype->storageHandle()->mgr());
	}
}


EdbDs_Entity::EdbDs_Entity():
    EdbDs_ObjectBase::EdbDs_ObjectBase()
{
	
	
}


EdbDs_Entity::EdbDs_Entity(const EdbDs_Entity& eIn):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        const_cast<EdbDs_Entity*>(&eIn)->name(),
        const_cast<EdbDs_Entity*>(&eIn)->uuid()
    )
{
    
}


QString
EdbDs_Entity::typeName()
{
    QString result;

    if (_etHandle != nullptr) {
        result = _etHandle->name();
    }

    return result;
}


QString
EdbDs_Entity::et_uuid()
{
	if (_etHandle != nullptr) {
		return _etHandle->uuid();
	}
	
	return _etUuid;
}


bool
EdbDs_Entity::setFieldValue(
    QString fName,
    QString fVal)
{
	EdbDs_FieldValue fv(
        this,
        fName,
        QVariant(fVal),
        "str"
    );
	
	
	return true;
}


bool
EdbDs_Entity::setFieldValue(
    QString fName,
    int fVal)
{
	EdbDs_FieldValue fv(
        this,
        fName,
        QVariant(fVal),
        "int"
    );
	
	return true;
}


bool
EdbDs_Entity::setFieldValue(
    QString fName,
    qlonglong fVal)
{
	EdbDs_FieldValue fv(
        this,
        fName,
        QVariant(fVal),
        "int"
    );
	
	return true;
}


bool
EdbDs_Entity::setFieldValue(
    QString fName,
    QByteArray fVal)
{
	EdbDs_FieldValue fv(
        this,
        fName,
        QVariant(fVal),
        "binary"
    );
	
	return true;
}


bool
EdbDs_Entity::setFieldValue(
    QString fName,
    double val)
{
    EdbDs_FieldValue fv(
        this,
        fName,
        QVariant(val),
        "double"
    );

    return true;
}


bool
EdbDs_Entity::setFieldValue(
    QString fName,
    EdbDs_Entity* fVal)
{
	// return false;
	
	
	QVariant val;
	val.setValue(fVal);
	
	EdbDs_FieldValue fv(
	    this,
	    fName,
	    val,
	    "entity"
	);
	
	return true;
}


bool
EdbDs_Entity::setFieldValue(QString fName, QUuid eUuid)
{
    QVariant val;
    val.setValue(eUuid);

    EdbDs_FieldValue fv(
        this,
        fName,
        val,
        "entity"
    );

    return true;
}


bool
EdbDs_Entity::setFieldValue(QString name, QVariant val)
{
    RLP_LOG_VERBOSE(name << " " << val)

    QString typeName = val.typeName();
    if (typeName == "QString") {
        return setFieldValue(name, val.toString());

    } else if (typeName == "int") {
        return setFieldValue(name, val.toInt());

    } else if (typeName == "double") {
        return setFieldValue(name, val.toDouble());

    } else if (typeName == "QUuid") {
         return setFieldValue(name, val.toUuid());
         
    } else if (typeName == "qlonglong") {
        return setFieldValue(name, val.toLongLong());

    } else if (typeName == "QVariantMap") {

        QVariantMap valm = val.toMap();
        if (valm.contains("uuid"))
        {
            QUuid u(valm.value("uuid").toString());
            return setFieldValue(name, u);

        } else {
            RLP_LOG_ERROR("TYPE " << typeName << " NOT IMPLEMENTED YET")
            RLP_LOG_ERROR(valm)

            Q_ASSERT(false);
        }
    } else {

        RLP_LOG_ERROR("TYPE " << typeName << " NOT IMPLEMENTED YET")
        Q_ASSERT(false);
    }

}
