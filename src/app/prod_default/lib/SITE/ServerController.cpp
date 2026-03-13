

#include "RlpCore/UTIL/AppGlobals.h"

#include "RlpRevlens/MEDIA/Factory.h"

#include "RlpProd/SITE/ServerController.h"
#include "RlpProd/SITE/pymodule.h"

#ifdef SCAFFOLD_PYBIND_STATIC
#include "RlpCore/QTCORE/QtCoreTypes.h"
#include "RlpCore/DS/pymodule.h"
#include "RlpCore/NET/pymodule.h"
#include "RlpCore/UTIL/pymodule.h"
#include "RlpCore/CNTRL/pymodule.h"
#include "RlpCore/PROC/pymodule.h"

#include "RlpEdb/CNTRL/pymodule.h"

#include "RlpRevlens/DS/pymodule.h"
#include "RlpRevlens/MEDIA/pymodule.h"

#include "RlpExtrevlens/RLSGCMD/pymodule.h"
#include "RlpExtrevlens/RLFFMPEG/pymodule.h"

#endif

#include "RlpCore/PY/Interp.h"


RLP_SETUP_LOGGER(prod, SITE, ServerController)


SITE_ServerController::SITE_ServerController(int port, bool runIOService):
    CoreCntrl_ControllerBase(),
    _locator(nullptr)
{
    _netMgr = new QNetworkAccessManager(this);

    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    QString edbRootDir = e.value("RLP_EDB_ROOT");
    if (edbRootDir.size() == 0)
    {
        edbRootDir = "/opt/rlp";
        RLP_LOG_WARN("RLP_EDB_ROOT not set, using /opt/rlp")
    }

    QString dbPath = QString("%1/edb/production_%2.db").arg(edbRootDir).arg(port);

    _ioHandler = new CoreNet_WebSocketIOHandler();

    QString dbName = "rlp_site";
    QString dbType = "production_db";

    _edbq = new EdbCntrl_Query(dbPath, dbName, dbType);
    connect(
        _edbq,
        SIGNAL(dataReady(QVariantMap)),
        this,
        SLOT(onEdbDataReady(QVariantMap))
    );

    
    _ffplug = new RLFFMPEG_Plugin();
}


void
SITE_ServerController::initPython()
{
    RLP_LOG_INFO("")

    #ifdef SCAFFOLD_PYBIND_STATIC
    RlpCoreQTCOREmodule_bootstrap();

    RlpCoreDSmodule_bootstrap();
    RlpCoreNETmodule_bootstrap();
    RlpCoreUTILmodule_bootstrap();
    RlpCoreCNTRLmodule_bootstrap();
    RlpCorePROCmodule_bootstrap();

    RlpEdbCNTRLmodule_bootstrap();
    RlpProdSITEmodule_bootstrap();

    RlpRevlensDSmodule_bootstrap();
    RlpRevlensMEDIAmodule_bootstrap();

    RlpExtrevlensRLSGCMDmodule_bootstrap();
    RlpExtrevlensRLFFMPEGmodule_bootstrap();
    #endif


    PY_Interp::init();

    QVariantMap globals;
    globals.insert("app.pid", QCoreApplication::applicationPid());

    UTIL_AppGlobals* ag = new UTIL_AppGlobals();
    ag->setInstance(ag);
    ag->setGlobals(globals);

    PY_Interp::eval("import RlpCoreQTCORE");

    // Required to bind this object
    PY_Interp::eval("import RlpCoreDSmodule");
    PY_Interp::eval("import RlpCoreUTILmodule");
    PY_Interp::eval("import RlpCoreNETmodule");
    PY_Interp::eval("import RlpCoreCNTRLmodule");
    PY_Interp::eval("import RlpProdSITEmodule");
    PY_Interp::eval("import RlpEdbCNTRLmodule");

    PY_Interp::eval("import RlpRevlensDSmodule");
    PY_Interp::eval("import RlpRevlensMEDIAmodule");

    PY_Interp::bindAppObject("RlpProdSITEmodule", "CNTRL", this);
    PY_Interp::bindAppObject("RlpEdbCNTRLmodule", "EDBC", _edbq);
    PY_Interp::bindAppObject("RlpCoreUTILmodule", "APPGLOBALS", UTIL_AppGlobals::instance());

    PY_Interp::eval("import rlp.core");
    PY_Interp::eval("import rlp.core.rlp_logging;rlp.core.rlp_logging._init_rlp_logging()");
    PY_Interp::eval("import revlens_prod.site.server;revlens_prod.site.server.startup()");
}


void
SITE_ServerController::onEdbDataReady(QVariantMap msgObj)
{
    // RLP_LOG_DEBUG(msgObj)

    emit dataReady(msgObj);
}


bool
SITE_ServerController::registerMediaLocator(MEDIA_Locator* plugin)
{
    RLP_LOG_DEBUG(plugin)

    return MEDIA_Factory::instance()->registerMediaLocator(plugin);
}



bool
SITE_ServerController::registerMediaPlugin(MEDIA_Plugin* plugin)
{
    RLP_LOG_DEBUG(plugin->name());

    // plugin->setMainController(this);
    return MEDIA_Factory::instance()->registerMediaPlugin(plugin);
}

// QVariantMap
// SITE_ServerController::loginCheck(QString username, QString password)
// {
//     RLP_LOG_DEBUG(username)

//     QVariantList args;
//     args.push_back(username);
//     args.push_back(password);

//     QVariant rawResult = PY_Interp::call(
//         "rlp_prod.site.server.login_check_password",
//         args
//     );

//     // RLP_LOG_DEBUG(rawResult)

//     QVariantMap result = rawResult.toMap();
//     // RLP_LOG_DEBUG(result)

//     return result;
// }


// QVariant
// SITE_ServerController::createUser(QString username, QString pw, QString firstname, QString lastname, QString email)
// {
//     RLP_LOG_DEBUG(username << firstname << lastname << email)

//     QVariantList args;
//     args.push_back(username);
//     args.push_back(pw);
//     args.push_back(firstname);
//     args.push_back(lastname);
//     args.push_back(email);

//     return PY_Interp::call("rlp_site.server.create_user", args);
// }


void
SITE_ServerController::onProxyRequestFinished(QString runId)
{
    RLP_LOG_DEBUG(runId)

    if (!_proxyRequestMap.contains(runId))
    {
        RLP_LOG_ERROR("Invalid proxy request runid:" << runId)
        return;
    }

    CoreNet_HttpRequest* req = _proxyRequestMap.value(runId);

    QString replyStr = QString(req->reply()->readAll());

    QString clientIdent = req->metadata().value("clientIdent").toString();
    QString fromMethod = req->metadata().value("method").toString();

    RLP_LOG_DEBUG(replyStr << "to" << clientIdent)

    QVariantMap result;
    result.insert("from_method", fromMethod);
    result.insert("result", replyStr);

    QVariantMap renvelope;
    renvelope.insert("ident", clientIdent);
    renvelope.insert("status", "OK");
    renvelope.insert("run_id", req->runId());
    renvelope.insert("result", result);

    emit dataReady(renvelope);

}


void
SITE_ServerController::handleRemoteCall(QVariantMap msgObj)
{
    // RLP_LOG_DEBUG(msgObj)

    QString methodName = msgObj.value("method").toString();
    QString methodCat = msgObj.value("method.category").toString();

    // RLP_LOG_DEBUG(methodName)

    QString ident = msgObj.value("ident").toString();
    QVariantMap sessionObj;

    QString status = "OK";
    QString errMsg = "";

    QString runId = "";
    if (msgObj.contains("session"))
    {
        sessionObj = msgObj.value("session").toMap();
        if (sessionObj.contains("run_id"))
        {
            runId = sessionObj.value("run_id").toString();
        }
    }

    sessionObj.insert("client.ident", ident);


    QVariant result;

    if (_commandRegistry.contains(methodName))
    {
        // RLP_LOG_DEBUG("FOUND:" << methodName)
        CoreCntrl_CommandBase* comm = _commandRegistry.value(methodName);

        // RLP_LOG_DEBUG(methodName << comm)
        result = comm->run(QVariant(msgObj));
    }

    else if (methodName.startsWith("edbc."))
    {
        RLP_LOG_INFO("FOUND edbc call")
        QString edbMethodName = methodName.replace("edbc.", "");
        msgObj.insert("method", edbMethodName);

        return _edbq->handleRemoteCall(msgObj);

    } else if (methodName.startsWith("rooms."))
    {

        QString roomMethodName = methodName.replace("rooms.", "");
        msgObj.insert("method", roomMethodName);


        QVariantList margs = msgObj.value("args").toList();
        QVariantMap mkwargs = msgObj.value("kwargs").toMap();

        QVariantList pyArgs;
        pyArgs.push_back(sessionObj);
        pyArgs.push_back(roomMethodName);
        pyArgs.push_back(margs);
        pyArgs.push_back(mkwargs);

        QString method = "rlplug_sync.rooms.service.rlpqt.handle_message";

        result = PY_Interp::call(method, pyArgs);
        QVariantMap bresult = result.toMap();

        //RLP_LOG_DEBUG(bresult)
        // LOG_Logging::pprint(bresult);

        if (bresult.contains("broadcast"))
        {
            QString broadcastGroup = bresult.value("broadcast").toString();
            if (broadcastGroup != "")
            {
                _wsServer->broadcast(bresult, broadcastGroup, ident);
            }
            
            return;
        }


    } else if (methodName == "media.locate")
    {
        MEDIA_Factory* factory = MEDIA_Factory::instance();

        QVariantList margs = msgObj.value("args").toList();
        QVariantMap mediaInput = margs.at(0).toMap();

        RLP_LOG_DEBUG("Running media.locate:" << mediaInput)

        // We're expecting this to be called with an RPCFuture / callback
        // setup, so just send as-is

        RLP_LOG_DEBUG("Running identify")

        RLP_LOG_WARN("NOT AVAILABLE")
        // factory->locate(mediaInput);

        // factory->insertInitialMediaProperties(&mediaInput);
        // _locator->locate(&mediaInput);
        // QVariantList streamResult = _ffplug->identifyMedia(mediaInput, DS_Node::DATA_VIDEO);

        // // QV = factory->identifyMedia2(mediaInput, DS_Node::DATA_VIDEO);

        // if (streamResult.size() > 0)
        // {
        //     mediaInput.insert(streamResult.at(0).toMap());
            
        //     result = mediaInput;

        //     // LOG_Logging::pprint(mediaInput);

        // }

    } else if (methodName.startsWith("site.download"))
    {

        QString ioMethodName = methodName.replace("site.", "");
        msgObj.insert("method", ioMethodName);

        result = _ioHandler->handleDownload(msgObj);

    } else if (methodName == "site.login")
    {
        QVariantMap sresult;
        sresult.insert("site_name", "rlp");
        sresult.insert("auth_key", "none");
        sresult.insert("username", "guest");
        sresult.insert("first_name", "Guest");
        sresult.insert("last_name", "User");
        sresult.insert("user_id", 1);
        sresult.insert("result", "OK");

        result.setValue(sresult);
    
    } else if (methodName == "stream.list")
    {
        QString url = "http://localhost:9997/v3/paths/list";
        if (runId == "")
        {
            RLP_LOG_WARN("Creating new UUID for proxy request")
            runId = QUuid::createUuid().toString();
        }

        QNetworkReply* reply = _netMgr->get(QNetworkRequest(url));
        CoreNet_HttpRequest* req = new CoreNet_HttpRequest(url, reply, runId);
        req->insertMetadata("clientIdent", ident);
        req->insertMetadata("method", methodName);

        _proxyRequestMap.insert(runId, req);
        connect(
            req,
            &CoreNet_HttpRequest::finished,
            this,
            &SITE_ServerController::onProxyRequestFinished
        );

        return;


    } else if (methodName == "__logrecord__")
    {
        result = msgObj.value("result");
        QVariantMap mresult = result.toMap();

        emit logRecordReady(mresult);

        QVariantMap extra = mresult.value("extra").toMap();
        QVariantMap mdata = extra.value("metadata").toMap();
        runId = mdata.value("run_id").toString();


        if (runId != "")
        {
            RLP_LOG_DEBUG("GOT RUN ID:" << runId)

            msgObj.insert("status", "OK");

            _wsServer->broadcast(msgObj, runId, ident);
            return;
        }


    } else
    {

        RLP_LOG_WARN("Unknown method:" << methodName)

        status = "ERR";
        errMsg = "Invalid method: ";
        errMsg += methodName;
    }


    {
        QVariantMap rresult;
        rresult.insert("status", status);
        rresult.insert("ident", ident);
        rresult.insert("run_id", runId);
        rresult.insert("result", result);

        if (errMsg != "")
        {
            rresult.insert("err_msg", errMsg);
        }

        emit dataReady(rresult);
    }
}