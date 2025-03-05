//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpCore/NET/WebSocketServer.h"
#include "RlpCore/CRYPT/Global.h"

RLP_SETUP_LOGGER(core, CoreNet, WebSocketServer)

CoreNet_WebSocketServer::CoreNet_WebSocketServer(
    int port,
    bool secure):
        _port(port),
        _wserver(nullptr),
        _encrypted(false),
        _secure(secure)
{
    _broadcastGroupMap.clear();
    _serviceNameMap.clear();

    _crypt = new CRYPT_VinCrypt();

    setSecure(secure);
}


CoreNet_WebSocketServer::~CoreNet_WebSocketServer()
{
    RLP_LOG_DEBUG("");

    _wserver->close();
    qDeleteAll(_clients.begin(), _clients.end());
}


void
CoreNet_WebSocketServer::setModeEncrypted(bool doEncrypted)
{
    RLP_LOG_INFO(doEncrypted)

    _encrypted = doEncrypted;
}


void
CoreNet_WebSocketServer::setSecure(bool doWss)
{
    RLP_LOG_INFO(doWss)

    _secure = doWss;

    QWebSocketServer::SslMode mode = QWebSocketServer::NonSecureMode;
    #ifndef SCAFFOLD_WASM
    if (_secure)
    {
        mode = QWebSocketServer::SecureMode;
    }
    #endif

    if (_wserver != nullptr)
    {
        RLP_LOG_WARN("Deleting server object")
        delete _wserver;
    }

    _wserver = new QWebSocketServer(
        "RLP Websocket Server",
        mode,
        this);


    if (!doWss)
    {
        return;
    }

    #ifndef SCAFFOLD_WASM

    QSslConfiguration sslConfiguration;

    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    QString certFilePath = e.value("RLP_SITE_CERTFILE");
    QString keyFilePath = e.value("RLP_SITE_KEYFILE");

    if (certFilePath.size() == 0)
    {
        certFilePath = ":/localhost.crt";
    }


    if (keyFilePath.size() == 0)
    {
        keyFilePath = ":/server.key";
    }

    RLP_LOG_DEBUG("Certfile:" << certFilePath << "Keyfile:" << keyFilePath)

    QFile certFile(certFilePath);
    QFile keyFile(keyFilePath);

    certFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);

    if ((!certFile.exists()) || (!keyFile.exists()))
    {
        RLP_LOG_ERROR("invalid certfile or keyfile, aborting");
        exit(1);
    }

    RLP_LOG_INFO("Certfile / Keyfile OK")

    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);

    certFile.close();
    keyFile.close();

    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1_2);

    _wserver->setSslConfiguration(sslConfiguration);

    #endif
}


QString
CoreNet_WebSocketServer::serverUrl()
{
    QString result = "ws";
    if (_secure)
    {
        result = "wss";
    }

    result += "://127.0.0.1:";
    result += QString("%1").arg(_port);

    return result;
}


QString
CoreNet_WebSocketServer::sockIdent(QWebSocket* sock)
{
    return QString("%1:%2").arg(sock->peerName()).arg(sock->peerPort());
}


void
CoreNet_WebSocketServer::start()
{
    RLP_LOG_DEBUG("")

    if (_wserver->listen(QHostAddress::AnyIPv4, _port)) { // 0 = Random port
        connect(_wserver, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

        RLP_LOG_DEBUG("listening: " << _wserver->serverUrl());

        emit serverReady(_port);
    }
}


void
CoreNet_WebSocketServer::registerService(QString serviceName, QString ident)
{
    RLP_LOG_DEBUG(serviceName << ident)
    _serviceNameMap.insert(serviceName, ident);

    emit serviceReady(serviceName, ident);
}


void
CoreNet_WebSocketServer::onNewConnection()
{
    QWebSocket* pSocket = _wserver->nextPendingConnection();
    QString si = sockIdent(pSocket); // pSocket->origin()

    RLP_LOG_DEBUG(si)

    connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

    _clients.insert(si, pSocket);
}


void
CoreNet_WebSocketServer::onTextMessageReceived(QString rawMsg)
{
    // RLP_LOG_DEBUG("Encrypted:" << _encrypted << "Message:" << rawMsg)
    // RLP_LOG_VERBOSE(rawMsg);

    if (_encrypted)
    {
        // RLP_LOG_DEBUG("Decrypting:")
        // RLP_LOG_DEBUG(rawMsg)

        rawMsg = _crypt->decryptToString(rawMsg);

        // RLP_LOG_DEBUG("GOT DECRYPTED:")
        // RLP_LOG_DEBUG(rawMsg)
    }


    QJsonDocument msg = QJsonDocument::fromJson(rawMsg.toUtf8(), &_error);
    
    if (msg.isNull()) {
        
        RLP_LOG_DEBUG("error decoding message as JSON: " << _error.errorString());
        return;
    }
    
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString si = sockIdent(pClient);

    QVariantMap msgObj = msg.object().toVariantMap();
    if (msgObj.contains("run_id"))
    {
        QString runId = msgObj.value("run_id").toString();
        _callResults.insert(runId, msgObj);
    }
    msgObj.insert("ident", si);

    emit msgReceived(msgObj);

    // _controller->handleRemoteCall(msgObj);

    // RLP_LOG_DEBUG(msgObj.value("method").toString().toStdString().c_str() );
}


void
CoreNet_WebSocketServer::onSocketDisconnected()
{
    RLP_LOG_DEBUG("")

    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    
    if (pClient) {
        QString si = sockIdent(pClient);
        _clients.remove(si);
        pClient->deleteLater();

        emit clientDisconnected(si);
    }
}


QString
CoreNet_WebSocketServer::formatMethodCall(QString method, QVariantList iargs, QVariantMap ikwargs, QVariantMap skwargs)
{
    QJsonObject result;
    QJsonArray args;
    
    QJsonObject kwargs = QJsonObject::fromVariantMap(ikwargs);
    
    QJsonObject session_kwargs = QJsonObject::fromVariantMap(skwargs);
    // session_kwargs.insert("key", "adc32e13cd904ca8b797d58ac02bcb3d");
    
    QVariantList::iterator it;
    for (it=iargs.begin(); it != iargs.end(); ++it) {
        args.append(QJsonValue::fromVariant(*it));
    }
    result.insert("method", method);
    result.insert("args", args);
    result.insert("kwargs", kwargs);
    result.insert("session", session_kwargs);
    
    return QString(QJsonDocument(result).toJson(QJsonDocument::Compact));
}


void
CoreNet_WebSocketServer::sendCallToClient(QVariantMap data)
{
    if (!data.contains("ident"))
    {
        RLP_LOG_ERROR("Cannot send to client, no client ident: " << data)
        return;
    }

    QString clientIdent = data.value("ident").toString();
    // RLP_LOG_DEBUG(data)

    QJsonObject jsonData = QJsonObject::fromVariantMap(data);
    QString msgStr(QJsonDocument(jsonData).toJson(QJsonDocument::Compact));

    // RLP_LOG_DEBUG(msgStr)

    if (_encrypted)
    {
        msgStr = _crypt->encryptToString(msgStr);
    }

    // RLP_LOG_DEBUG("num clients: " << _clients.size() << " msg: " << msgStr);

    if (_clients.contains(clientIdent))
    {
        // RLP_LOG_DEBUG("Sending to:" << clientIdent) //  << ":" << msgStr)

        _clients.value(clientIdent)->sendTextMessage(msgStr);
    } else 
    {
        RLP_LOG_WARN("No client with ident" << clientIdent << " - skipping send")
    }
}


void
CoreNet_WebSocketServer::sendBinaryToClient(QString clientIdent, const QByteArray& data)
{
    if (_clients.contains(clientIdent))
    {
        _clients.value(clientIdent)->sendBinaryMessage(data);
    } else 
    {
        RLP_LOG_WARN("No client with ident" << clientIdent << " - skipping send")
    }

}


QString
CoreNet_WebSocketServer::sendCallToService(QString serviceName, QVariantMap data)
{
    if (!_serviceNameMap.contains(serviceName))
    {
        RLP_LOG_ERROR("not found: " << serviceName)
        return QString("");
    }

    QString runId = QUuid::createUuid().toString();
    if (!data.contains("run_id"))
    {
        data.insert("run_id", runId);
    } else
    {
        runId = data.value("run_id").toString();
    }

    QJsonObject jsonData = QJsonObject::fromVariantMap(data);
    QString msgStr(QJsonDocument(jsonData).toJson(QJsonDocument::Compact));

    if (_encrypted)
    {
        msgStr = _crypt->encryptToString(msgStr);
    }

    // WARNING: credentials might be passing on this message
    // be careful enabling logging here
    // "num clients: " << _clients.size() << " msg: " << msgStr);

    
    QString clientIdent = _serviceNameMap.value(serviceName);

    RLP_LOG_VERBOSE(serviceName << clientIdent << runId)

    _clients.value(clientIdent)->sendTextMessage(msgStr);

    return runId;
}


QString
CoreNet_WebSocketServer::broadcast(QVariantMap data, QString broadcastGroup, QString skipIdent)
{
    // RLP_LOG_VERBOSE("num clients:" << _clients.size() << "skipIdent:" << skipIdent)
    // RLP_LOG_VERBOSE(data)

    QString runId = QUuid::createUuid().toString();
    data.insert("run_id", runId);

    QJsonObject jsonData = QJsonObject::fromVariantMap(data);
    QString msgStr(QJsonDocument(jsonData).toJson(QJsonDocument::Compact));

    if (_encrypted)
    {
        msgStr = _crypt->encryptToString(msgStr);
    }

    if (broadcastGroup == "")
    {
        // RLP_LOG_DEBUG("BROADCAST TO EVERYONE CONNECTED!!")

        ClientSockIterator it;

        for (it=_clients.begin(); it != _clients.end(); ++it)
        {
            if (it.key() == skipIdent) {
                // ("SKIPPING: " << skipIdent)
                continue;
            }

            // RLP_LOG_DEBUG("Sending to:" << it.key())
            it.value()->sendTextMessage(msgStr);
        }
    } else
    {
        QSet<QString> clients = _broadcastGroupMap.value(broadcastGroup);

        // RLP_LOG_DEBUG("BROADCAST GROUP: CLIENTS:" << clients)
        // RLP_LOG_DEBUG("All clients:" << _clients.keys())

        QSet<QString>::iterator it;

        for (it=clients.begin(); it != clients.end(); ++it)
        {
            QString clientIdent = *it;
            if (clientIdent == skipIdent)
            {
                // RLP_LOG_DEBUG("SKIPPING:" << skipIdent);
                continue;
            }

            if (_clients.contains(clientIdent))
            {
                // RLP_LOG_DEBUG("SENDING TO:"  << clientIdent)
                _clients.value(clientIdent)->sendTextMessage(msgStr);
            } else
            {
                RLP_LOG_WARN("Client not found at" << clientIdent << "skipping")
            }
            
        }

        if (data.contains("broadcast.clients"))
        {
            QVariantList forceClients = data.value("broadcast.clients").toList();

            // RLP_LOG_VERBOSE("GOT BROADCAST CLIENTS")
            // RLP_LOG_VERBOSE(forceClients)

            for (int ci=0; ci != forceClients.size(); ci++)
            {
                QString cident = forceClients.at(ci).toString();
                if (_clients.contains(cident))
                {
                    // RLP_LOG_DEBUG("Sending to:" << cident)
                    _clients.value(cident)->sendTextMessage(msgStr);
                }
            }
        }
        
    }
    

    return runId;
}



bool
CoreNet_WebSocketServer::addClientToBroadcastGroup(QString clientIdent, QString broadcastGroup)
{
    RLP_LOG_DEBUG("Broadcast Group:" << broadcastGroup << "Client Ident:" << clientIdent)

    if (!_broadcastGroupMap.contains(broadcastGroup))
    {
        RLP_LOG_INFO("Creating Broadcast Group:" << broadcastGroup)

        QSet<QString> s;
        _broadcastGroupMap.insert(broadcastGroup, s);
    }

    QSet<QString> s = _broadcastGroupMap.value(broadcastGroup);
    s.insert(clientIdent);
    _broadcastGroupMap.insert(broadcastGroup, s);

    return true;
}


bool
CoreNet_WebSocketServer::removeClientFromBroadcastGroup(QString clientIdent, QString broadcastGroup)
{
    bool result = false;

    if (_broadcastGroupMap.contains(broadcastGroup))
    {
        QSet<QString> s = _broadcastGroupMap.value(broadcastGroup);
        if (s.contains(clientIdent))
        {
            RLP_LOG_INFO("Removing" << clientIdent << "from broadcast group:" << broadcastGroup)
            s.remove(clientIdent);
            _broadcastGroupMap.insert(broadcastGroup, s);

            result = true;
        }
    }

    return result;
}


int
CoreNet_WebSocketServer::numClientsInBroadcastGroup(QString broadcastGroupName)
{
    if (_broadcastGroupMap.contains(broadcastGroupName))
    {
        return _broadcastGroupMap.size();
    }

    return 0;
}


bool
CoreNet_WebSocketServer::hasCallResult(QString runId)
{
    return _callResults.contains(runId);
}


QVariantMap
CoreNet_WebSocketServer::getCallResult(QString runId, bool clear)
{
    QVariantMap result = _callResults.value(runId);
    if (clear)
    {
        _callResults.remove(runId);
    }

    return result;

}
