
#ifndef PROD_SITE_SERVER_CONTROLLER_H
#define PROD_SITE_SERVER_CONTROLLER_H

#include "RlpProd/SITE/Global.h"

#include "RlpCore/CNTRL/Auth.h"
#include "RlpCore/CNTRL/ControllerBase.h"

#include "RlpCore/NET/WebSocketIOHandler.h"
#include "RlpCore/NET/HttpRequest.h"

#include "RlpEdb/CNTRL/Query.h"

#include "RlpRevlens/MEDIA/Locator.h"

#include "RlpExtrevlens/RLFFMPEG/Plugin.h"


class PROD_SITE_API SITE_ServerController : public CoreCntrl_ControllerBase {
    Q_OBJECT

signals:
    void serviceReady(QVariantMap serviceInfo); // revlens server controller API compat

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

    // API Compat with CNTRL_MainController
    SITE_ServerController* getPluginManager() { return this; }
    SITE_ServerController* getServerManager() { return this; }

    void onProxyRequestFinished(QString runId);


public slots: // API Compat with CNTRL_MainController

    bool registerMediaLocator(MEDIA_Locator* plugin);
    bool registerMediaPlugin(MEDIA_Plugin* plugin);


protected:
    QNetworkAccessManager* _netMgr;
    EdbCntrl_Query* _edbq;
    CoreNet_WebSocketIOHandler* _ioHandler;
    QMap<QString, CoreNet_HttpRequest*> _proxyRequestMap;


    MEDIA_Locator* _locator;
    RLFFMPEG_Plugin* _ffplug;

};

#endif

