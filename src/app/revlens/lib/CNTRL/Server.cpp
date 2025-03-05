//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//


#include "RlpRevlens/CNTRL/Server.h"

#include "RlpCore/PY/Interp.h"

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpRevlens/CNTRL/MainController.h"

RLP_SETUP_LOGGER(revlens, CNTRL, Server)

CNTRL_Server::CNTRL_Server(CNTRL_MainController* cntrl) :
    _controller(cntrl),
    _wsClient(nullptr)
{
    RLP_LOG_INFO("")

    /*
    _wsClient = new CoreNet_WebSocketClient(); // for rooms - TODO FIXME: move to RoomController
    */

    int port = 0;
    QString batch = getenv("RLP_BATCH");
    if (batch == "1")
    {
        port = 8140;
    }

    _wsServer = new CoreNet_WebSocketServer(port, false);
    cntrl->setWsServer(_wsServer);

    _wsServer->start();

    connect(
        cntrl,
        &CNTRL_MainController::remoteCall,
        this,
        &CNTRL_Server::onRemoteCallReceived
    );


     /*

    _netMgr.clearAccessCache();
    _netMgr.clearConnectionCache();

    #ifndef SCAFFOLD_WASM
    _netMgr.activeConfiguration().setConnectTimeout(15);
    #endif
    
    connect(&_netMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(onLicenseRequestFinished(QNetworkReply*)));
    */
   
    // start ioservice if not in service mode
    //
    // QVariantMap globals = UTIL_AppGlobals::instance()->globals();
    // if (!(globals.contains("startup.mode.service")))
    // {
    //     startIOService();
    // }
    

    
}


void
CNTRL_Server::startIOService()
{
    RLP_LOG_DEBUG("")

    // Start IOService
    //
    #ifdef _WIN32
    QVariantMap* globals = UTIL_AppGlobals::instance()->globalsPtr();
    QString appRoot = globals->value("app.rootpath").toString();
    QString ioServiceCmd = "rlp_io_service.bat";
    QString ioServicePath = QString("%1\\core\\global\\bin\\%2").arg(appRoot).arg(ioServiceCmd);
    #else
    QString ioServicePath = "rlp_io_service";
    #endif

    QStringList args;
    args.append(QString("%1").arg(_wsServer->serverPort()));

    // RLP_LOG_DEBUG("Starting IOService: " << ioServicePath << args)

    // bool result = QProcess::startDetached(ioServicePath, args);
    // RLP_LOG_DEBUG(result)
}


void
CNTRL_Server::startBatchService()
{
    RLP_LOG_DEBUG("")

    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    QString appRootDir = e.value("REVLENS_APP_ROOT");

    QStringList args;
    args.append("--service");
    args.append("--batch");
    args.append("--script");
    
    
    #ifdef _WIN32
    QVariantMap* globals = UTIL_AppGlobals::instance()->globalsPtr();
    QString appRoot = globals->value("app.rootpath").toString();
    QString revCmd = "revlens.bat";
    QString rCmd = QString("%1\\revlens\\global\\bin\\%2").arg(appRoot).arg(revCmd);

    args.append(QString("%1\\revlens\\global\\bin\\rlp_batch_service.py").arg(appRootDir));

    #else
    args.append(QString("%1/revlens/global/bin/rlp_batch_service").arg(appRootDir));
    QString rCmd = "revlens";
    #endif

    args.append(QString("%1").arg(_wsServer->serverPort()));


    #ifndef SCAFFOLD_WASM
    #ifndef SCAFFOLD_IOS
    RLP_LOG_DEBUG("Starting Revlens Batch Service: " << rCmd << args)
    bool result = QProcess::startDetached(rCmd, args);
    RLP_LOG_DEBUG(result)
    #endif
    #endif
}


void
CNTRL_Server::setWebSocketClient(CoreNet_WebSocketClient* client)
{
    RLP_LOG_DEBUG(client << client->url())

    _wsClient = client;
    connect(
        _wsClient,
        &CoreNet_WebSocketClient::messageReceived,
        _controller,
        &CNTRL_MainController::handleRemoteCall
    );
}



void
CNTRL_Server::sendCallToService(QString method)
{
    // RLP_LOG_DEBUG(method)

    // Example for separate thread
    // QMetaObject::invokeMethod(_wsClient, "sendCall", Qt::QueuedConnection, Q_ARG(QString, method));
    if (_wsClient == nullptr)
    {
        RLP_LOG_ERROR("No client!")
        return;
    }


    _wsClient->sendCall(method);
}


void
CNTRL_Server::sendCallToService(QString method, QVariantList args, QVariantMap kwargs, QVariantMap skwargs)
{
    // RLP_LOG_DEBUG(method << " " << args << " " << kwargs)

    // Example of separate thread
    //
    // QMetaObject::invokeMethod(_wsClient, "sendCall",
    //    Qt::QueuedConnection, Q_ARG(QString, method), Q_ARG(QVariantList, args), Q_ARG(QVariantMap, kwargs)
    // );
    if (_wsClient == nullptr)
    {
        RLP_LOG_ERROR("No client!")
        return;
    }


    _wsClient->sendCall(method, args, kwargs, skwargs);
}


void
CNTRL_Server::sendCallInternal(QVariantMap data)
{
    RLP_LOG_ERROR("DISABLED, SPECIFY BROADCAST OR CLIENT?")

    //_wsServer->sendCall(data);
}


QString
CNTRL_Server::sendCallInternalTo(QString serviceName, QVariantMap data)
{
    RLP_LOG_DEBUG("" );

    return _wsServer->sendCallToService(serviceName, data);
}


void
CNTRL_Server::onRemoteCallReceived(QVariantMap msgObj)
{
    // RLP_LOG_VERBOSE(msgObj);

    QString methodName = msgObj.value("method").toString();
    QVariantList argList = msgObj.value("args").toList();

    if (methodName == "register_service")
    {
        QString serviceName = argList.at(0).toString();
        QString serviceIdent = msgObj.value("ident").toString();

        if (serviceName == "rlp_batch")
        {
            int batchServicePort = argList.at(1).toInt();
            RLP_LOG_INFO("Registering Batch Service on port " << batchServicePort)

            // _batchWsClient->setUrl(QString("ws://127.0.0.1:%1").arg(batchServicePort));


        } else
        {
            //
            // setup is supposed to be a *client* websocket connecting OUT to the server running in the interactive process.
            // client websocket then goes into a listening mode for commands from interactive process
            //

            RLP_LOG_DEBUG("Registering service " << serviceName << " - " << serviceIdent)

            _wsServer->registerService(serviceName, serviceIdent);

        }

        QVariantMap serviceInfo;
        serviceInfo.insert("event", "service_ready");
        serviceInfo.insert("service_name", serviceName);
        serviceInfo.insert("service_ident", serviceIdent);
        serviceInfo.insert("service_args", argList);

        RLP_LOG_DEBUG("SERVICE DATA READY")
        RLP_LOG_DEBUG(serviceInfo)

        // _controller->emitDataReady(serviceInfo);
        emit serviceReady(serviceInfo);

    } else if (methodName == "pyCall")
    {

        RLP_LOG_ERROR("pyCall from remote - not available")

        // QVariantMap callInfo = argList.at(0).toMap();

        // RLP_LOG_DEBUG(callInfo)

        // PY_Interp::call(callInfo);

    } else if (methodName == "pyEval")
    {
        RLP_LOG_INFO("pyEval from remote")

        QString pyStr = argList.at(0).toString();

        PY_Interp::eval(pyStr);

    } else if (methodName == "pyEvalFile")
    {
        RLP_LOG_INFO("pyEvalFile from remote")

        QString pyFilePath = argList.at(0).toString();

        PY_Interp::evalFile(pyFilePath);
    }
}


void
CNTRL_Server::licenseCheck()
{
    RLP_LOG_DEBUG("")

    // SCAFFOLD_BUILD_TIME is shipped via the build system
    //
    if (SCAFFOLD_BUILD_TIME == 0) {
        RLP_LOG_WARN("ZERO BUILD TIME - OK")
        return;
    }

    QDateTime buildTime = QDateTime::fromSecsSinceEpoch(SCAFFOLD_BUILD_TIME);
    QDateTime now = QDateTime::currentDateTime();

    qint64 secsSinceBuildTime = buildTime.secsTo(now);
    qint64 ONE_WEEK = 604800;
    qint64 ONE_MONTH = 2592000; // 30 days

    qint64 THREE_WEEKS = ONE_WEEK * 3;
    
    qint64 SIX_MONTHS = ONE_MONTH * 6;

    qint64 max_secs_diff = SIX_MONTHS;

    RLP_LOG_DEBUG("Build Time: " << buildTime)
    RLP_LOG_DEBUG("Secs since build time: " << secsSinceBuildTime)
    RLP_LOG_DEBUG("Max secs: " << max_secs_diff)

    if (secsSinceBuildTime > max_secs_diff) {
        
        RLP_LOG_ERROR("Timed License expired. exiting");
        exit(2);

    } else {

        RLP_LOG_INFO("Timed License OK")
    }

    /*

    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();

    QString licUrlStr = e.value("REVLENS_LICENSE_SERVER_URL");
    if (licUrlStr == "") {

        std::cout << "License server not specified, aborting" << std::endl;

        RLP_LOG_ERROR("License server not specified, aborting" );
        exit(10);

        
    } else {

        RLP_LOG_DEBUG(licUrlStr.toStdString().c_str() );

        QUrlQuery query;
        query.addQueryItem("username", e.value("USER"));
        query.addQueryItem("revlens_version", "001");

        // RLP_LOG_DEBUG("CNTRL_Server::licenseCheck(): " << query.query().toStdString().c_str() );

        QNetworkRequest licRequest(QUrl("http://" + licUrlStr));
        licRequest.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
        _netMgr.post(licRequest, QByteArray(query.query().toLatin1()));
    }

    */

}


void
CNTRL_Server::onLicenseRequestFinished(QNetworkReply* reply)
{
    // RLP_LOG_DEBUG("CNTRL_Server::onLicenseRequestFinished()" );

    if (reply->error() != QNetworkReply::NoError) {

        std::cout << "license request failed, aborting" << std::endl;
        RLP_LOG_ERROR("license request failed, aborting" );
        exit(10);
    }

    QByteArray result = reply->readAll();
    QJsonDocument resultData = QJsonDocument::fromJson(QByteArray::fromBase64(result));
    QVariantMap data = resultData.object().toVariantMap();

    if (data.value("retcode").toString() != "ok") {

        std::cout <<  "license request failed, aborting" << std::endl;
        
        RLP_LOG_ERROR("license request failed, aborting" );
        exit(10);
    }

}
