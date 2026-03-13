

#ifndef CORE_PY_SIGNAL_H
#define CORE_PY_SIGNAL_H

#include "RlpCore/PY/Global.h"
#include "RlpCore/PY/PyQVariantMap.h"
#include "RlpCore/PY/SigSlotConnector.h"


class CORE_PY_API PY_Signal : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    PY_Signal(const QObject* parent, QString sigName);
    PY_Signal(QObject* parent);

    void connect(py::object slot);
    void emit_(py::dict data); // fake sig-slot from python to python

    // Find a connector by traversing the class heirarchy, checking parent classes if necessary
    PY_SigSlotConnector* findConnector(const QMetaObject* metaObj, QString sigName);

    static void registerConnector(QString signalSignature, PY_SigSlotConnector* conn);

    void disconnectAll();

protected:

    const QObject* _parent;

    QString _sigName;
    QMetaMethod _signal;
    QMetaMethod _slot;
    PY_SigSlotConnector* _connector;

    // Connector registry
    static QHash<QString, PY_SigSlotConnector*> _connectors;
};


#endif