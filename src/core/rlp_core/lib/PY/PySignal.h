

#ifndef CORE_PY_SIGNAL_H
#define CORE_PY_SIGNAL_H

#include "RlpCore/PY/Global.h"
#include "RlpCore/PY/PyQVariantMap.h"

namespace py = pybind11;

class CORE_PY_API PySigSlotConnector : public QObject {
    Q_OBJECT

public:
    PySigSlotConnector()
    {
        _pyslots.clear();
    }


    void setPySlot(py::object slot)
    {
        _pyslots.append(slot);
    }

    int pySlotCount() { return _pyslots.size(); }

protected:
    QList<py::object> _pyslots;

};


class CORE_PY_API PySignal : public QObject {
    Q_OBJECT

public:
    PySignal(const QObject* parent, QString sigName);
    PySignal();

    void connect(py::object slot);
    void emit_(py::dict data);

    static void registerConnector(QString signalSignature, PySigSlotConnector* conn);

public slots:

    void dest();
    void dest(bool val);
    void dest(int val);
    void dest(QString val);
    void dest(QVariantMap data);


protected:

    const QObject* _parent;
    QString _sigName;
    QMetaMethod _signal;
    

    QMetaMethod _slot;
    QList<py::object> _pyslots;

    QObject* _destObject;
    QList<py::object> _slots;

    static QHash<QString, PySigSlotConnector*> _connectors;
};


#endif