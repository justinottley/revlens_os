
#include "RlpCore/PY/SigSlotConnector.h"
#include "RlpCore/PY/Interp.h"

PY_SigSlotConnector::PY_SigSlotConnector()
{
    _pyslots.clear();
}


void
PY_SigSlotConnector::registerConnection(QMetaObject::Connection conn)
{
    // qInfo() << "REGISTERING CONNECTION";
    _conn = conn;
}


void
PY_SigSlotConnector::addPySlot(py::object slot)
{
    _pyslots.append(slot);
    // qInfo() << "PY_SigSlotConnector::addPySlot(): slot count:" << _pyslots.size();
}


bool
PY_SigSlotConnector::removePySlot(py::object slot)
{
    // qInfo() << "PY_SigSlotConnector::removePySlots(): slot count:" << _pyslots.size();

    bool result = false;
    QList<py::object> newSlots;
    for (auto myslot : _pyslots)
    {
        if (myslot != slot)
        {
            newSlots.append(myslot);
        } else
        {
            // qInfo() << "REMOVING SLOT";
            result = true;
        }
    }

    _pyslots = newSlots;

    if ((_pyslots.size() == 0) && (_conn))
    {
        // qInfo() << "PY_SigSlotConnector::removePySlots(): DISCONNECTING";
        QObject::disconnect(_conn);
    }

    // Ensure the incoming slot method to be disconnected doesn't get garbage collected yet,
    // can cause crashes
    slot.inc_ref();

    return result;
}


void
PY_SigSlotConnector::emit_(py::dict data)
{
    PY_Interp::acquireGIL();

    for (auto pyslot : _pyslots)
    {
        if (pyslot != nullptr)
        {
            pyslot(data);
        }
        
    }
}

