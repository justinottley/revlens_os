//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/NET/WebSocketClient.h"
#include "RlpCore/CRYPT/Global.h"
#include "RlpCore/UTIL/Convert.h"

RLP_SETUP_LOGGER(core, CoreNet, WebSocketClient)

CoreNet_WebSocketClient::CoreNet_WebSocketClient():
    _autoReconnect(false),
    _encrypted(false),
    _connected(false)
{
    _crypt = new CRYPT_VinCrypt();

    setupConnections();

    _reconnectTimer.callOnTimeout(this, &CoreNet_WebSocketClient::reconnect);
    _reconnectTimer.setInterval(5000);
}


void
CoreNet_WebSocketClient::setupConnections()
{
    connect(&_wsocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&_wsocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(&_wsocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(&_wsocket, SIGNAL(binaryMessageReceived(const QByteArray&)), this, SLOT(onBinaryReceived(const QByteArray&)));
    connect(&_wsocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    
    #ifndef SCAFFOLD_WASM
    connect(&_wsocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &CoreNet_WebSocketClient::onSslErrors);
    #endif
    
}


bool
CoreNet_WebSocketClient::setUrl(QString url)
{
    RLP_LOG_INFO(url);

    #ifndef SCAFFOLD_WASM
    if (url.contains("wss"))
    {
        bool sslOk = QSslSocket::supportsSsl();
        RLP_LOG_DEBUG("SSL:" << sslOk)
    }
    #endif

    _url = QUrl(url);
    _wsocket.open(_url);


    return true;
}


void
CoreNet_WebSocketClient::setModeEncrypted(bool doEncrypted)
{
    RLP_LOG_INFO(doEncrypted)

    _encrypted = doEncrypted;
}


QString
CoreNet_WebSocketClient::formatMethodCall(QString method, QVariantList iargs, QVariantMap ikwargs, QVariantMap skwargs)
{
    // RLP_LOG_DEBUG(method << iargs << ikwargs << skwargs)
    QJsonObject result;

    QJsonObject kwargs = QJsonObject::fromVariantMap(ikwargs);
    QJsonArray args = QJsonArray::fromVariantList(iargs);
    QJsonObject session_kwargs = QJsonObject::fromVariantMap(skwargs);
    // session_kwargs.insert("key", "adc32e13cd904ca8b797d58ac02bcb3d");
    
    /*
    // NOTE: no args
    //
    // QJsonArray args;
    QVariantList::iterator it;
    for (it=iargs.begin(); it != iargs.end(); ++it) {
        args.append(QJsonValue::fromVariant(*it));
    }
    */

    // QStringList methodParts = method.split('.');
    // if (methodParts.size() == 2)
    // {
    //     result.insert("method.category", methodParts[0]);
    //     result.insert("method.func", methodParts[1]);
    // }

    result.insert("method", method);
    result.insert("args", args);
    result.insert("kwargs", kwargs);
    result.insert("session", session_kwargs);
    
    QString plainTextMsg = QString(QJsonDocument(result).toJson(QJsonDocument::Compact));

    if (_encrypted)
    {
        return _crypt->encryptToString(plainTextMsg);
    }

    return plainTextMsg;
}


void
CoreNet_WebSocketClient::sendCall(QString method)
{
    if (!_wsocket.isValid())
    {
        RLP_LOG_ERROR("INVALID WEBSOCKET" );
        return;
    }
    
    // RLP_LOG_DEBUG(_wsocket.isValid() << _wsocket.state())

    QString jsonMsg = formatMethodCall(method);

    _wsocket.sendTextMessage(jsonMsg);
    _wsocket.flush();
}


void
CoreNet_WebSocketClient::sendCall(QString method, QVariantList args, QVariantMap kwargs, QVariantMap skwargs)
{
    // RLP_LOG_DEBUG("sending " << method)
    // RLP_LOG_DEBUG(_wsocket.isValid() << _wsocket.state() << _url)

    QString jsonMsg = formatMethodCall(method, args, kwargs, skwargs);
    
    // RLP_LOG_DEBUG(jsonMsg);
    
    _wsocket.sendTextMessage(jsonMsg);
    _wsocket.flush();

    // RLP_LOG_DEBUG("Done")
}


void
CoreNet_WebSocketClient::sendTextMessage(QString msg)
{
    // RLP_LOG_DEBUG(msg)

    _wsocket.sendTextMessage(msg);
    _wsocket.flush();

}


void
CoreNet_WebSocketClient::onTextMessageReceived(QString rawMsg)
{
    // RLP_LOG_DEBUG(rawMsg)

    if (_encrypted)
    {
        rawMsg = _crypt->decryptToString(rawMsg);
    }

    // RLP_LOG_DEBUG(rawMsg)

    QVariantMap msgObj = UTIL_Convert::fromJsonString(rawMsg);

    // RLP_LOG_DEBUG(msgObj)

    emit messageReceived(msgObj);
}


void
CoreNet_WebSocketClient::onBinaryReceived(const QByteArray& data)
{
    emit binaryReceived(data);
}


void
CoreNet_WebSocketClient::onConnected()
{
    RLP_LOG_DEBUG("");

    _connected = true;

    emit onClientConnected();
    _reconnectTimer.stop();
}


void
CoreNet_WebSocketClient::onDisconnected()
{
    RLP_LOG_WARN(_wsocket.errorString());

    _connected = false;

    emit onClientDisconnected();

    if (_autoReconnect)
    {
        _reconnectTimer.start();
    }
}


void CoreNet_WebSocketClient::onSslErrors(const QList<QSslError> &errors)
{
    Q_UNUSED(errors);
    RLP_LOG_ERROR("")

    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.

    #ifndef SCAFFOLD_WASM
    RLP_LOG_ERROR(errors)
    _wsocket.ignoreSslErrors();
    #endif
}


void
CoreNet_WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    RLP_LOG_ERROR("")
    RLP_LOG_ERROR(error)
}


void
CoreNet_WebSocketClient::reconnect()
{
    RLP_LOG_DEBUG((int)_wsocket.state())

    if (_wsocket.state() != QAbstractSocket::ConnectedState)
    {
        RLP_LOG_WARN("Disconnected, attempting reconnect")

        setUrl(_url.toString());

    } else {

        RLP_LOG_DEBUG("Stopping reconnect timer")
        _reconnectTimer.stop();
    }
}


void
CoreNet_WebSocketClient::emitErrorReceived(QVariantMap errInfo)
{
    RLP_LOG_DEBUG(errInfo);
    emit errorReceived(errInfo);
}