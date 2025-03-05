
#include "RlpRevlens/DS/Graph.h"

RLP_SETUP_LOGGER(revlens, DS, Graph)

DS_Graph::DS_Graph(QVariantMap* properties, QString uuidStr):
    _properties(properties)
{
    if (uuidStr != "")
    {
        _uuid = QUuid(uuidStr);
    } else
    {
        _uuid = QUuid::createUuid();
    }
}


void
DS_Graph::emitDataReady(QVariantMap data)
{
    // RLP_LOG_DEBUG("");

    data.insert("graph.uuid", _uuid);
    emit dataReady(data);
}