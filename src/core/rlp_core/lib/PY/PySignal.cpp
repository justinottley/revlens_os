

#include "RlpCore/PY/PySignal.h"
#include "RlpCore/PY/Interp.h"

QHash<QString, PySigSlotConnector*> PySignal::_connectors;

PySignal::PySignal(const QObject* parent, QString sigName):
    _parent(parent),
    _sigName(sigName)
{
    int index = _parent->metaObject()->indexOfSignal(
        QMetaObject::normalizedSignature(qPrintable(sigName)));

    if (index == -1) {
        qWarning() << _parent << "PySignal : Wrong signal name:" << sigName;
        return;
    }

    _signal = _parent->metaObject()->method(index);

    QString slotName = "dest_dict(QVariantMap)";
    int sigArgCount = _signal.parameterCount();
    if (sigArgCount == 0)
    {
        slotName = "dest()";

    } else if (sigArgCount == 1)
    {
        QString paramTypeName = _signal.parameterTypeName(0);
        // qInfo() << "GOT PARAMETER TYPE:" << paramTypeName;

        slotName = "dest(" + paramTypeName + ")";
    }

    // qInfo() << "PySignal : SLOT NAME:" << slotName;

    int slotIndex = metaObject()->indexOfSlot(
        QMetaObject::normalizedSignature(qPrintable(slotName)));

    if (slotIndex == -1)
    {
        QString signalSignature = QString("%1_%2").
            arg(_parent->metaObject()->className()).
            arg(sigName);

        if (PySignal::_connectors.contains(signalSignature))
        {
            slotName = "slot_" + sigName;
            
            PySigSlotConnector* conn = PySignal::_connectors.value(signalSignature);
            slotIndex = conn->metaObject()->indexOfSlot(
                QMetaObject::normalizedSignature(qPrintable(slotName)));

            if (slotIndex != -1)
            {
                _slot = conn->metaObject()->method(slotIndex);
                _destObject = conn;
            }
            

        } else {
            qWarning() << "PySignal: no connector found for " << signalSignature; // SLOT NOT FOUND:" << slotName;
        }

        return;
    }

    _slot = metaObject()->method(slotIndex);
    _destObject = this;

}


PySignal::PySignal():
    _parent(nullptr),
    _destObject(this)
{
    _pyslots.clear();
    _slots.clear();
}


void
PySignal::registerConnector(QString signalSignature, PySigSlotConnector* conn)
{
    qInfo() << "PySignal::registerConnector()" << signalSignature;

    _connectors[signalSignature] = conn;
}


void
PySignal::connect(py::object slot)
{
    // qInfo() << "PySignal::connect()";

    if (_destObject == this)
    {
        if ((_parent != nullptr) && (_pyslots.size() == 0))
        {
            QObject::connect(_parent, _signal, _destObject, _slot);
        }

        _pyslots.append(slot);

    } else
    {
        PySigSlotConnector* pssc = qobject_cast<PySigSlotConnector*>(_destObject);

        if (pssc->pySlotCount() == 0)
        {
            // qInfo() << "::connect(): attempting QObject::connect";
            QObject::connect(_parent, _signal, _destObject, _slot);
        }

        pssc->setPySlot(slot);
    }


    if (_parent == nullptr) // from python?
    {
        _slots.push_back(slot);
    }
}


void
PySignal::emit_(py::dict data)
{
    PY_Interp::acquireGIL();

    for (auto it : _slots)
    {
        if (it != nullptr)
        {
            it(data);
        }
        
    }
}


void
PySignal::dest()
{
    PY_Interp::acquireGIL();

    for (auto pslot : _pyslots)
    {
        if (pslot != nullptr)
        {
            pslot();
        }
    }
}


void
PySignal::dest(bool val)
{
    PY_Interp::acquireGIL();
    
    for (auto pslot : _pyslots)
    {
        if (pslot != nullptr)
        {
            pslot(val);
        }
    }
}


void
PySignal::dest(int val)
{
    PY_Interp::acquireGIL();

    for (auto pslot : _pyslots)
    {
        if (pslot != nullptr)
        {
            pslot(val);
        }
    }
}

void
PySignal::dest(QString val)
{
    PY_Interp::acquireGIL();

    for (auto pslot : _pyslots)
    {
        if (pslot != nullptr)
        {
            pslot(val.toStdString());
        }
    }
}


void
PySignal::dest(QVariantMap data)
{
    PY_Interp::acquireGIL();

    py::dict pydata = PyQVariantMap(data).toDict();

    for (auto pslot : _pyslots)
    {
        if (pslot != nullptr)
        {
            pslot(pydata);
        }
    }
}