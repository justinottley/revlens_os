
#include <iostream>

#include "RlpCore/UTIL/Convert.h"

#include "RlpEdb/DS/StorageManagerBase.h"
#include "RlpEdb/DS/EdbObjectBase.h"

RLP_SETUP_LOGGER(edb, EdbDs, ObjectBase)

EdbDs_ObjectBase::EdbDs_ObjectBase(
    QString name,
    QString uuid,
    QVariantMap metadata,
    int createDate,
    QString createUser,
    int updateDate,
    QString updateUser,
    bool autoSave):
        _name(name),
        _uuid(uuid),
        _metadata(metadata),
        _createDate(createDate),
        _createUser(createUser),
        _updateDate(updateDate),
        _updateUser(updateUser),
        _sthandle(nullptr),
        _bindResult(EdbDs_StorageHandleBase::BIND_UNKNOWN),
        _autoSave(autoSave),
        _autoCreate(true)
{
    // RLP_LOG_DEBUG("name: " << _name << " uuid: " << _uuid)
}


EdbDs_StorageHandleBase::BindResult
EdbDs_ObjectBase::bindStorage(EdbDs_StorageManagerBase* storage)
{
    // RLP_LOG_DEBUG("")

	_sthandle = storage->makeStorageHandle(this);
	_bindResult = _sthandle->bind();

    // RLP_LOG_DEBUG("bind result: " << (int)br)

	return _bindResult;
}


void
EdbDs_ObjectBase::setMetadata(QVariantMap metadataIn)
{
    // RLP_LOG_VERBOSE(metadataIn)

    _metadata = metadataIn;

    if (_autoSave) {

        RLP_LOG_VERBOSE("autosaving metadata")

        saveOne("metadata", metadataString());
    }

}


QString
EdbDs_ObjectBase::metadataString()
{
    QJsonObject result;
    UTIL_Convert::toJsonObject(_metadata, &result);

    // RLP_LOG_VERBOSE("JSON converted metadata: " << result)

    QJsonDocument doc(result);
    return QString(doc.toJson(QJsonDocument::Compact));
}


// template<class T>
void
EdbDs_ObjectBase::saveOne(QString methodName, int val)
{
	if (_sthandle != nullptr) {
		_sthandle->saveOne(methodName, val);
	}
}


void
EdbDs_ObjectBase::saveOne(QString methodName, QString val)
{
	if (_sthandle != nullptr) {
		_sthandle->saveOne(methodName, val);
	}
}


void
EdbDs_ObjectBase::saveOne(QString methodName, QJsonObject val)
{
	if (_sthandle != nullptr) {
		_sthandle->saveOne(methodName, val);
	}
}


void
EdbDs_ObjectBase::save()
{
	if (_sthandle != nullptr) {
	    _sthandle->save();
    } else {
        std::cout << "cannot save, no sthandle" << std::endl;
    }
}


bool
EdbDs_ObjectBase::remove()
{
    if (_sthandle != nullptr) {
        return _sthandle->remove();
    }

    return false;
}
