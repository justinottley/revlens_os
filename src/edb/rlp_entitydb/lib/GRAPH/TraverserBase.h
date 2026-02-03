//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_GRAPH_TRAVERSER_H
#define EDB_GRAPH_TRAVERSER_H

#include "RlpEdb/DS/StorageConnectionBase.h"
#include "RlpEdb/GRAPH/Global.h"

class EDB_GRAPH_API EdbGraph_TraverserBase : public QObject {
    Q_OBJECT
    
public:

    EdbGraph_TraverserBase();

    virtual QVariantList find(
        QString etypeName,
        QVariantList filters,
        QStringList returnFields)
    {
        return QVariantList();
    }
    

    virtual EdbDs_StorageConnectionBase* conn()
    {
        return nullptr;
    }
    

};

#endif
