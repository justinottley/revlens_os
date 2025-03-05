//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef EDB_GRAPH_NODE_H
#define EDB_GRAPH_NODE_H

#include "RlpEdb/GRAPH/Global.h"

class EdbGraph_Node;
typedef std::shared_ptr<EdbGraph_Node> EdbGraph_NodePtr;

typedef QMap<int, EdbGraph_NodePtr> EdbGraph_Path;

class EDB_GRAPH_API EdbGraph_Node : public QObject {
    Q_OBJECT
    
public:

    EdbGraph_Node(
        QString entityType,
        QVariantList filters,
        QStringList returnFields,
        QString edgeField,
        int edgeLabel
    );

    
    EdbGraph_Node(
        QString entityType
    );
    
    
    QString entityType() { return _entityType; }
    QVariantList filters() { return _filters; }
    QStringList returnFields() { return _returnFields; }
    
    QString edgeField() { return _edgeField; }
    bool hasEdge() { return _edgeField.length() > 0; }
    
    int edgeLabel() { return _edgeLabel; }
    
    
    void setFilters(QVariantList filters)
    {
        _filters = filters;
    }
    
    void setReturnFields(QStringList returnFields)
    {
        _returnFields = returnFields;
    }
    
    void setLabel(int label)
    {
        _label = label;
    }
        
        
    void setEdgeField(QString efield)
    {
        _edgeField = efield;
    }
    
    void setEdgeLabel(int label)
    {
        _edgeLabel = label;
    }
    
    /*
    
    void setUuidResult(QStringList uuidResult)
    {
        _uuidResult = uuidResult;
    }
    
    */
    
    
    void setReturnResult(QVariantList result)
    {
        _returnResult = result;
    }
        
    QList<QUuid> returnResultUuids()
    {
        QList<QUuid> result;

        QVariantList::iterator it;
        for (it = _returnResult.begin(); it != _returnResult.end(); ++it) {
            result.append(QUuid(it->toMap().value("uuid").toString()));
        }
        
        return result;
    }
        
    
private:

    QString _entityType;
    QVariantList _filters;
    QStringList _returnFields;
    int _label;
    
    QString _edgeField;
    int _edgeLabel;
    
    // QStringList _uuidResult;
    QVariantList _returnResult;
    
};

#endif
