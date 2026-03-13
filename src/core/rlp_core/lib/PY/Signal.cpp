

#include "RlpCore/PY/Signal.h"
#include "RlpCore/PY/Interp.h"

#include <QtQuick/QQuickItem>

RLP_SETUP_LOGGER(core, PY, Signal)

QHash<QString, PY_SigSlotConnector*> PY_Signal::_connectors;

PY_Signal::PY_Signal(const QObject* parent, QString sigName):
    _parent(parent),
    _sigName(sigName),
    _connector(nullptr)
{
    int index = _parent->metaObject()->indexOfSignal(
        QMetaObject::normalizedSignature(qPrintable(sigName)));

    if (index == -1)
    {
        RLP_LOG_WARN(_parent << "wrong signal name:" << sigName)
        return;
    }

    _signal = _parent->metaObject()->method(index);

    int slotIndex = -1;

    PY_SigSlotConnector* pySigSlotConn = findConnector(_parent->metaObject(), sigName);
    if (!pySigSlotConn)
    {
        RLP_LOG_WARN("No Connector found for" << _parent->metaObject()->className() << sigName)
        return;
    }

    pySigSlotConn = pySigSlotConn->clone(); // important

    QString slotName = "slot_" + sigName;
    slotIndex = pySigSlotConn->metaObject()->indexOfSlot(
        QMetaObject::normalizedSignature(qPrintable(slotName)));

    if (slotIndex != -1)
    {
        _slot = pySigSlotConn->metaObject()->method(slotIndex);
        _connector = pySigSlotConn;
    } else
    {
        RLP_LOG_WARN("Slot not found:" << slotName << "for signal:" << sigName)
    }
}


PY_Signal::PY_Signal(QObject* parent):
    _parent(parent),
    _connector(new PY_SigSlotConnector())
{
}


PY_SigSlotConnector*
PY_Signal::findConnector(const QMetaObject* metaObj, QString sigName)
{
    if (!metaObj)
    {
        RLP_LOG_WARN("superclass hierarchy exhausted")
        return nullptr;
    }

    RLP_LOG_VERBOSE(metaObj->className() << sigName)

    QString signalSignature = QString("%1_%2").
        arg(metaObj->className()).
        arg(sigName);

    if (PY_Signal::_connectors.contains(signalSignature))
    {
        return PY_Signal::_connectors.value(signalSignature);
    }

    RLP_LOG_VERBOSE(metaObj->className() << " - Checking parents")

    return findConnector(metaObj->superClass(), sigName);
}


void
PY_Signal::registerConnector(QString signalSignature, PY_SigSlotConnector* conn)
{
    RLP_LOG_VERBOSE(signalSignature)

    _connectors[signalSignature] = conn;
}


void
PY_Signal::connect(py::object slot)
{
    if (_connector == nullptr)
    {
        RLP_LOG_ERROR("No connector, cannot connect")
        return;
    }

    if (_signal.isValid() && (_connector->pySlotCount() == 0))
    {
        QMetaObject::Connection qconn = QObject::connect(_parent, _signal, _connector, _slot);
        _connector->registerConnection(qconn);

        if (!qconn)
        {
            RLP_LOG_WARN("QObject::connect() Failed:" << _connector << ":" << QString(_slot.methodSignature()))
        }
    }

    _connector->addPySlot(slot);

    // Setup paper trail to allow disconnection of slots
    // when C++ object gets deallocated
    //
    if (py::hasattr(slot, "__self__"))
    {
        py::object parentPyObj = py::getattr(slot, "__self__");
        QString className = PyQVariant::getPyClassName(parentPyObj);

        // NOTE: This class can only be used (in python) if the parent is QObject
        //
        QObject* qobj = py::cast<QObject*>(parentPyObj);
        PyConnInfo connInfo({_connector, slot});

        QVariant v;
        v.setValue(connInfo);

        if (!qobj->property("pyslots").isValid())
        {
            QVariantList pyslots;
            ((QQuickItem*)qobj)->setProperty("pyslots", pyslots);
        }
        
        QVariantList pyslots = qobj->property("pyslots").toList();
        pyslots.push_back(v);

        ((QQuickItem*)qobj)->setProperty("pyslots", pyslots);

    }
}


void
PY_Signal::disconnectAll()
{
    RLP_LOG_DEBUG("")

    // TODO FIXME: CopyPasta from ItemBase::disconnectPySlots()

    if (_parent->property("pyslots").isValid())
    {
        QVariantList pyslots = _parent->property("pyslots").toList();

        // RLP_LOG_DEBUG(pyslots)

        for (auto pyslot : pyslots)
        {
            PyConnInfo pci = pyslot.value<PyConnInfo>();
            if (pci.second != py::none())
            {
                PY_SigSlotConnector* pssc = qobject_cast<PY_SigSlotConnector*>(pci.first);

                if (pssc->removePySlot(pci.second))
                {
                    // RLP_LOG_DEBUG("Slot disconnected")
                } else
                {
                    RLP_LOG_WARN("SLOT NOT DISCONNECTED")
                }
            }
        }

    }
}


void
PY_Signal::emit_(py::dict data)
{
    if (_connector == nullptr)
    {
        RLP_LOG_ERROR("No connector, cannot emit")
        return;
    }

    _connector->emit_(data);
}
