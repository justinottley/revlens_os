//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_STORAGEHANDLEBASE_H
#define EDB_DS_STORAGEHANDLEBASE_H

#include <memory>
#include "RlpEdb/DS/Global.h"

// class EdbDs_ObjectBase;

class EdbDs_StorageManagerBase;

// typedef std::shared_ptr<DS_EdbDs_StorageHandleBase>DS_EdbDs_StorageHandleBasePtr;

class EdbDs_ObjectBase;

class EDB_DS_API EdbDs_StorageHandleBase : public QObject {
    Q_OBJECT
    
public:

    enum BindResult {
        BIND_UNKNOWN,
        BIND_FAIL,
        BIND_EXISTING,
        BIND_NEW
    };
    
    
    EdbDs_StorageHandleBase(
        EdbDs_ObjectBase* eobj,
        EdbDs_StorageManagerBase* mgr);
    
    virtual BindResult bind() { return BIND_FAIL; }
    virtual bool save() { return false; }
    
    virtual bool remove() { return false; }

    virtual void saveOne(QString methodName, int val) {}
    virtual void saveOne(QString methodName, QString val) {}
    virtual void saveOne(QString methodName, QJsonObject val) {}
    
    virtual EdbDs_StorageManagerBase* mgr() { return _mgr; }
    
protected:

    EdbDs_ObjectBase* _eobj;
    EdbDs_StorageManagerBase* _mgr;
    
};

// Q_DECLARE_METATYPE(EdbDs_StorageBase);

#endif