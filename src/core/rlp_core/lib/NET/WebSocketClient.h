//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef CORE_NET_WEBSOCKET_CLIENT_H
#define CORE_NET_WEBSOCKET_CLIENT_H

#include "RlpCore/NET/Global.h"

#include "RlpCore/CRYPT/VinCrypt.h"



class CORE_NET_API CoreNet_WebSocketClient : public QObject {
    Q_OBJECT


signals:
    void onClientConnected(); // re-emitted from internal QWebSocket signal for public consumption
    void onClientDisconnected(); // re-emitted from internal QWebSocket signal for public consumption
    void messageReceived(QVariantMap msgObj);
    void errorReceived(QVariantMap errInfo);
    void binaryReceived(const QByteArray& msg);


public:
    RLP_DEFINE_LOGGER

    CoreNet_WebSocketClient();

    void reconnect();

    void sendCall(QString method);

    void sendCall(QString method, QVariantList args) {
        QVariantMap kwargs;
        QVariantMap skwargs;
        sendCall(method, args, kwargs, skwargs);
    }

    void sendCall(QString method, QVariantMap kwargs) {
        QVariantMap skwargs;
        QVariantList args;
        sendCall(method, args, kwargs, skwargs);
    }


    QString
    formatMethodCall(QString method)
    {
        QVariantMap kwargs;
        QVariantList args;
        QVariantMap skwargs;
        return formatMethodCall(method, args, kwargs, skwargs); 
    }


public slots:

    static CoreNet_WebSocketClient* new_CoreNet_WebSocketClient() { return new CoreNet_WebSocketClient(); }

    QUrl url() { return _url; }

    void setModeEncrypted(bool doEncrypted);
    bool isEncrypted() { return _encrypted; }

    bool setUrl(QString url);
    void setAutoReconnect(bool doReconnect) { _autoReconnect = doReconnect; }

    void sendCall(QString method, QVariantList args, QVariantMap kwargs, QVariantMap skwargs);

    void sendTextMessage(QString msg);

    void onConnected();
    void onDisconnected();
    bool isConnected() { return _connected; }

    void onTextMessageReceived(QString msg);
    void onBinaryReceived(const QByteArray& data);

    void onSslErrors(const QList<QSslError> &errors);
    void onError(QAbstractSocket::SocketError);

    bool isValid() { return _wsocket.isValid(); }

    QString formatMethodCall(QString method, QVariantList iargs, QVariantMap ikwargs, QVariantMap skwargs);

    void emitErrorReceived(QVariantMap errInfo);

private:
    void setupConnections();


protected:
    
    QWebSocket _wsocket;
    QUrl _url;
    QJsonParseError _error;

    bool _autoReconnect;
    QTimer _reconnectTimer;

    bool _encrypted;
    CRYPT_VinCrypt* _crypt;

    bool _connected;
};


class CORE_NET_API CoreNet_WebSocketClientDecorator : public QObject {
    Q_OBJECT

public slots:

    CoreNet_WebSocketClient* new_CoreNet_WebSocketClient()
    {
        return new CoreNet_WebSocketClient();
    }

};

#endif