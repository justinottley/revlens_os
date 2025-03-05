

#include "RlpEdb/GRAPH/Node.h"


EdbGraph_Node::EdbGraph_Node(
    QString entityType,
    QVariantList filters,
    QStringList returnFields,
    QString edgeField,
    int edgeLabel):
        _entityType(entityType),
        _filters(filters),
        _returnFields(returnFields),
        _edgeField(edgeField),
        _edgeLabel(edgeLabel)
{
}


EdbGraph_Node::EdbGraph_Node(
    QString entityType):
        _entityType(entityType),
        _filters(QVariantList()),
        _returnFields(QStringList()),
        _edgeField(""),
        _edgeLabel(-1)
{
}
        
    