

#ifndef EdbMedia_SCHEMA_H
#define EdbMedia_SCHEMA_H

#include "RlpEdb/MEDIA/Global.h"

#include "RlpEdb/DS/StorageManagerBase.h"
#include "RlpEdb/DS/EntityType.h"
#include "RlpEdb/DS/Entity.h"

class EDB_MEDIA_API EdbMedia_Schema : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    EdbMedia_Schema(EdbDs_StorageManagerBase* storage);

    void initSchema();
    void initData();

    EdbDs_EntityType* StreamType() { return _StreamType; }
    EdbDs_EntityType* VideoStreamType() { return _VideoStreamType; }
    EdbDs_EntityType* FrameInfoType() { return _FrameInfoType; }

private:
    EdbDs_StorageManagerBase* _storage;
    EdbDs_DataSource* _dsrc;

    
    EdbDs_EntityType* _StreamType;
    EdbDs_EntityType* _VideoStreamType;
    EdbDs_EntityType* _FrameInfoType;

};

#endif
