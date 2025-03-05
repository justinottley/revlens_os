//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_DS_STORAGECONNECTIONBASE_H
#define EDB_DS_STORAGECONNECTIONBASE_H

#include <memory>
#include "RlpEdb/DS/Global.h"

// typedef std::shared_ptr<DS_EdbDs_StorageHandleBase>DS_EdbDs_StorageHandleBasePtr;

class EDB_DS_API EdbDs_StorageConnectionBase : public QObject {
    Q_OBJECT

public:
    EdbDs_StorageConnectionBase();

public slots:

};

// Q_DECLARE_METATYPE(EdbDs_StorageBase);

#endif