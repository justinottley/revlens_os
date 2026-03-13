
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

    _flags = new CoreDs_Flags();
    connect(
        _flags,
        &CoreDs_Flags::flagStateChanged,
        this,
        &DS_Graph::onInternalFlagStateChanged
    );

    connect(
        this,
        &DS_Graph::dataReady,
        this,
        &DS_Graph::onGraphDataReady,
        Qt::AutoConnection // for complete clarity
    );
}


void
DS_Graph::emitDataReady(QString dataType, QVariantMap data)
{
    data.insert("graph.uuid", _uuid);

    if (_properties->contains("load_uuid"))
    {
        data.insert("load_uuid", _properties->value("load_uuid"));
    }

    // RLP_LOG_DEBUG(dataType)
    // LOG_Logging::pprint(data);

    emit dataReady(dataType, data);
}


void
DS_Graph::emitNoticeEvent(QString evtName, QVariantMap evtInfo)
{
    evtInfo.insert("graph.uuid", _uuid);
    emit noticeEvent(evtName, evtInfo);
}


void
DS_Graph::onGraphDataReady(QString dataType, QVariantMap data)
{
    // RLP_LOG_DEBUG(dataType);

    if (dataType == "update_properties")
    {
        QVariantMap props = data.value("props").toMap();

        // LOG_Logging::pprint(data);

        // TODO FIXME HACK
        if (props.contains("audio.data") && _properties->contains("audio.data"))
        {
            RLP_LOG_DEBUG("Deallocating audio.data")

            QByteArray* ad = _properties->value("audio.data").value<QByteArray*>();
            delete ad;
            _properties->remove("audio.data");
        }

        _properties->insert(props);

        if (data.contains("evt_name"))
        {
            QString evtName = data.value("evt_name").toString();
            QVariantMap evtInfo = data.value("evt_info").toMap();
            if (evtName != "update_properties") // infinite recursion check
            {
                emitDataReady(evtName, evtInfo);
            }
        }

        if (data.contains("flags"))
        {
            QVariantMap flagData = data.value("flags").toMap();
            QVariantMap::iterator it;
            for (it = flagData.begin(); it != flagData.end(); ++it)
            {
                _flags->setFlagByName(it.key(), it.value().toBool());
            }
        }
    }
}


void
DS_Graph::onInternalFlagStateChanged(QString flagName, bool val)
{
    emit flagStateChanged(_uuid.toString(), flagName, val);
}