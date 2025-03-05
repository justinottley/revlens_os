
#include "RlpEdb/DS/StorageManagerBase.h"
#include "RlpEdb/DS/DataSource.h"

EdbDs_DataSource::EdbDs_DataSource(
            QString name,
            QString dataSourceType,
            QString uuidStr,
            QString schema,
            DtsOriginType origin,
            int version):
    EdbDs_ObjectBase::EdbDs_ObjectBase(name, uuidStr),
    _type(dataSourceType),
    _schema(schema),
    _origin(origin),
    _version(version)
{
}

EdbDs_DataSource::EdbDs_DataSource():
    EdbDs_ObjectBase::EdbDs_ObjectBase(),
    _type(""),
    _schema("")
{
}

EdbDs_DataSource::EdbDs_DataSource(const EdbDs_DataSource& dsIn):
    EdbDs_ObjectBase::EdbDs_ObjectBase(
        const_cast<EdbDs_DataSource*>(&dsIn)->name(),
        const_cast<EdbDs_DataSource*>(&dsIn)->uuid()
    ),
    _type(const_cast<EdbDs_DataSource*>(&dsIn)->type()),
    _schema(const_cast<EdbDs_DataSource*>(&dsIn)->schema())
{
}


