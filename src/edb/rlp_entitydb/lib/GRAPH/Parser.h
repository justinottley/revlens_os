//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_GRAPH_PARSER_H
#define EDB_GRAPH_PARSER_H

#include "RlpEdb/GRAPH/Global.h"
#include "RlpEdb/GRAPH/Node.h"

class EDB_GRAPH_API EdbGraph_Parser : public QObject {
    Q_OBJECT
    
public:

    EdbGraph_Parser();

    virtual EdbGraph_Path parsePath(QByteArray pathStr);

private:

    QMap<int, EdbGraph_Node*> _edgeIndex;
    

};

#endif
