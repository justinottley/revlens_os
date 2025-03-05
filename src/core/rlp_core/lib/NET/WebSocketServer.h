//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef CORE_NET_WEBSOCKET_SERVER_H
#define CORE_NET_WEBSOCKET_SERVER_H

#include "RlpCore/NET/Global.h"

#include <QtCore/QJsonParseError>
#include <QtCore/QJsonDocument>

#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>


#include "RlpCore/CRYPT/VinCrypt.h"


typedef QMap<QString, QWebSocket*>::iterator ClientSockIterator;

class CORE_NET_API CoreNet_WebSocketServer : public QObject {
    Q_OBJECT

signals:

    void msgReceived(QVariantMap msgObj);
    void serverReady(int port);
    void serviceReady(QString serviceName, QString serviceIdent);
    void clientDisconnected(QString ident);

public:
    RLP_DEFINE_LOGGER

    CoreNet_WebSocketServer(int port=0, bool secure=false);
    ~CoreNet_WebSocketServer();

    void start();

public slots:

    void setModeEncrypted(bool doEncrypted);
    void setSecure(bool doWss);
    bool isSecure() { return _secure; }

    int serverPort() { return _port; } // _wserver->serverPort(); }
    QString serverUrl();

    QString sockIdent(QWebSocket* sock);

    void registerService(QString serviceName, QString ident);
    QStringList services() { return _serviceNameMap.keys(); }
    bool hasService(QString serviceName) { return _serviceNameMap.contains(serviceName); }

    void onNewConnection();
    void onTextMessageReceived(QString message);
    void onSocketDisconnected();

    void sendCallToClient(QVariantMap data);
    void sendBinaryToClient(QString clientIdent, const QByteArray& data);

    /*
     * Send a call into a unique service, returning a call id
     */
    QString sendCallToService(QString serviceName, QVariantMap data);

    bool addClientToBroadcastGroup(QString clientIdent, QString broadcastGroupName);
    bool removeClientFromBroadcastGroup(QString clientIdent, QString broadcastGroupName);
    int numClientsInBroadcastGroup(QString broadcastGroupName);

    // send to all known connected clients in the input broadcast group, except the one who sent
    // which should be passed in as skipIdent
    QString broadcast(QVariantMap data, QString broadcastGroup, QString skipIdent="");

    QString formatMethodCall(QString method, QVariantList args, QVariantMap kwargs, QVariantMap skwargs);

    bool hasCallResult(QString runId);
    QVariantMap getCallResult(QString runId, bool clear=true);


private:

    int _port;
    QWebSocketServer* _wserver;

    // QList<QWebSocket *> _clients;
    QMap<QString, QWebSocket*> _clients;
    QMap<QString, QSet<QString> > _broadcastGroupMap;
    QMap<QString, QString> _serviceNameMap;
    QMap<QString, QVariantMap> _callResults;

    QJsonParseError _error;

    bool _encrypted;
    bool _secure; // wss

    CRYPT_VinCrypt* _crypt;
};

#endif