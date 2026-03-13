//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_STORAGEMANAGERBASE_H
#define EDB_DS_STORAGEMANAGERBASE_H

#include "RlpEdb/DS/Global.h"
#include "RlpEdb/DS/DataSource.h"
#include "RlpEdb/DS/StorageConnectionBase.h"

class EDB_DS_API EdbDs_StorageManagerBase {

public:

    EdbDs_StorageManagerBase() {}
    
    virtual EdbDs_StorageConnectionBase* conn() { return nullptr; }
    
    // virtual void flush(EdbDs_DataSource* obj) {}
    virtual EdbDs_StorageHandleBase* makeStorageHandle(EdbDs_ObjectBase* eobj) { return nullptr; }

};

// Q_DECLARE_METATYPE(EdbDs_StorageBase);

#endif