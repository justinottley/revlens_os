
#ifndef CORE_PY_SIG_SLOT_CONNECTOR_H
#define CORE_PY_SIG_SLOT_CONNECTOR_H

#include "RlpCore/PY/Global.h"
#include "RlpCore/PY/PyQVariantMap.h"

namespace py = pybind11;

class CORE_PY_API PY_SigSlotConnector : public QObject {
    Q_OBJECT

public:
    PY_SigSlotConnector();

    void addPySlot(py::object slot);
    bool removePySlot(py::object slot);

    int pySlotCount() { return _pyslots.size(); }
    void registerConnection(QMetaObject::Connection conn);

    virtual PY_SigSlotConnector* clone() { return nullptr; }

    void emit_(py::dict data); // fake signal-slot setup from python to python

protected:
    QList<py::object> _pyslots;
    QMetaObject::Connection _conn;

};

#endif