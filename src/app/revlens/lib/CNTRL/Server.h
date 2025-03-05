//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_CNTRL_SERVER_H
#define REVLENS_CNTRL_SERVER_H

#include "RlpRevlens/CNTRL/Global.h"

// #include "RlpRevlens/NET/HttpServer.h"

#include "RlpCore/NET/WebSocketServer.h"
#include "RlpCore/NET/WebSocketClient.h"



class CNTRL_MainController;

class REVLENS_CNTRL_API CNTRL_Server : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    CNTRL_Server(CNTRL_MainController* controller);

    void sendCallToService(QString method);

signals:
    void serviceReady(QVariantMap serviceInfo);


public slots:
    
    void startIOService();
    void startBatchService();

    // Send on client call to outside server / service
    // SITE Client
    void setWebSocketClient(CoreNet_WebSocketClient* client);

    // send to SITE Client
    void sendCallToService(QString method, QVariantList args, QVariantMap kwargs, QVariantMap skwargs);

    void sendCallToClient(QVariantMap data) { _wsServer->sendCallToClient(data); }
    void sendBinaryToClient(QString clientIdent, const QByteArray& data) { _wsServer->sendBinaryToClient(clientIdent, data); }

    // Send on internal server
    void sendCallInternal(QVariantMap data);
    QString sendCallInternalTo(QString serviceName, QVariantMap data);
    
    bool hasCallResult(QString runId) { return _wsServer->hasCallResult(runId); }
    QVariantMap getCallResult(QString runId, bool clear=true) { return _wsServer->getCallResult(runId, clear); }
    

    void onRemoteCallReceived(QVariantMap msgObj);
    
    quint64 webSocketServerPort() { return _wsServer->serverPort(); }

public slots:

    void licenseCheck();
    void onLicenseRequestFinished(QNetworkReply *);

    QStringList services() { return _wsServer->services(); }
    CoreNet_WebSocketClient* wsClient() { return _wsClient; }



private:
    
    CNTRL_MainController* _controller;
    
    QNetworkAccessManager _netMgr;

    // NET_HttpServer* _httpServer;
    CoreNet_WebSocketClient* _wsClient;
    CoreNet_WebSocketServer* _wsServer;

};

// qRegisterMetaType(QAbstractSocket::SocketError);

#endif

