
#ifndef PROD_SITE_SERVER_CONTROLLER_H
#define PROD_SITE_SERVER_CONTROLLER_H

#include "RlpProd/SITE/Global.h"

#include "RlpCore/CNTRL/Auth.h"
#include "RlpCore/CNTRL/ControllerBase.h"

#include "RlpCore/NET/WebSocketIOHandler.h"

#include "RlpEdb/CNTRL/Query.h"

class PROD_SITE_API SITE_ServerController : public CoreCntrl_ControllerBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    SITE_ServerController(int port, bool runIOService=false);


public slots:

    virtual void initPython();

    void onEdbDataReady(QVariantMap);

    // QVariantMap loginCheck(QString username, QString password);
    // QVariant createUser(QString username, QString pw, QString firstname, QString lastname, QString email);

    virtual void handleRemoteCall(QVariantMap msgObj);

    EdbCntrl_Query* edbq() { return _edbq; }

protected:
    EdbCntrl_Query* _edbq;
    CoreNet_WebSocketIOHandler* _ioHandler;

};

#endif

