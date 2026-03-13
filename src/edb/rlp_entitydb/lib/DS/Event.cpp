

#include "RlpEdb/DS/Event.h"

RLP_SETUP_LOGGER(edb, EdbDs, Event)

EdbDs_Event::EdbDs_Event(
    EdbDs_StorageManagerBase* mgr,
    QString name,
    QVariantMap data,
    QVariantMap opts):
        EdbDs_ObjectBase::EdbDs_ObjectBase(
            name,
            "",
            data
        )
{
    if (opts.contains("remote_evt_id"))
    {
        _remoteEvtId = opts.value("remote_evt_id").toInt();
    } else
    {
        _remoteEvtId = -1;
    }

    if (opts.contains("event_uuid"))
    {
        _uuid = opts.value("event_uuid").toString();
    }

    
    bindStorage(mgr);
}


bool
EdbDs_Event::isSynced()
{
    RLP_LOG_DEBUG("HARDCODED")

    return true;
}


void
EdbDs_Event::setLastEventId(int evtId)
{
    RLP_LOG_INFO(evtId)

    _lastEvtId = evtId;
}