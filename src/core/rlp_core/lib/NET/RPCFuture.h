
#ifndef CORE_NET_RPC_FUTURE_H
#define CORE_NET_RPC_FUTURE_H

#include "RlpCore/NET/Global.h"

class CoreNet_RPCFuture : public QObject {
    Q_OBJECT

signals:
    void resultReady(QVariantMap result);

public:
    CoreNet_RPCFuture():
        _runId(QUuid::createUuid().toString(QUuid::WithoutBraces))
    {
    }

    virtual ~CoreNet_RPCFuture()
    {
        qInfo() << "CoreNet_RPCFuture::~CoreNet_RPCFuture()";
    }

public slots:

    static CoreNet_RPCFuture*
    new_CoreNet_RPCFuture()
    {
        return new CoreNet_RPCFuture();
    }


    virtual void onResultReady(QVariantMap msgObj);

    QString runId() { return _runId; }

protected:
    QString _runId;

};

#endif
