//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_GRAPH_RESULT_H
#define EDB_GRAPH_RESULT_H

#include "RlpEdb/GRAPH/Global.h"

class EDB_GRAPH_API EdbGraph_Result {

public:

    EdbGraph_Result();

    QString etype() { return _etype; }
    
    QVariantMap
    QVariantList* result() { return &_result; }
    
private:

    QString _etype;
    QVariantMap _etypeMap;
    QVariantList _result;
    

};

#endif
