

#include "RlpCore/UTIL/AppGlobals.h"

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
#endif

#include "RlpCore/PY/Interp.h"


RLP_SETUP_LOGGER(prod, SITE, ServerController)


SITE_ServerController::SITE_ServerController(int port, bool runIOService):
    CoreCntrl_ControllerBase()
{
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
    #endif


    PY_Interp::init();

    QVariantMap globals;
    globals.insert("app.pid", QCoreApplication::applicationPid());

    UTIL_AppGlobals* ag = new UTIL_AppGlobals();
    ag->setInstance(ag);
    ag->setGlobals(globals);

    // Required to bind this object
    PY_Interp::eval("import RlpCoreDSmodule");
    PY_Interp::eval("import RlpCoreUTILmodule");
    PY_Interp::eval("import RlpCoreNETmodule");
    PY_Interp::eval("import RlpCoreCNTRLmodule");
    PY_Interp::eval("import RlpProdSITEmodule");
    PY_Interp::eval("import RlpEdbCNTRLmodule");

    PY_Interp::bindAppObject("RlpProdSITEmodule", "CNTRL", this);
    PY_Interp::bindAppObject("RlpEdbCNTRLmodule", "EDBC", _edbq);
    PY_Interp::bindAppObject("RlpCoreUTILmodule", "APPGLOBALS", UTIL_AppGlobals::instance());

    PY_Interp::eval("import rlp.core");
    PY_Interp::eval("import revlens_prod.site.server;revlens_prod.site.server.startup()");
}


void
SITE_ServerController::onEdbDataReady(QVariantMap msgObj)
{
    // RLP_LOG_DEBUG(msgObj)

    emit dataReady(msgObj);
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
SITE_ServerController::handleRemoteCall(QVariantMap msgObj)
{
    // RLP_LOG_DEBUG(msgObj)

    QString methodName = msgObj.value("method").toString();
    QString methodCat = msgObj.value("method.category").toString();

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
        CoreCntrl_CommandBase* comm = _commandRegistry.value(methodName);
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

        if (bresult.contains("broadcast"))
        {
            QString broadcastGroup = bresult.value("broadcast").toString();
            if (broadcastGroup != "")
            {
                _wsServer->broadcast(bresult, broadcastGroup, ident);
            }
            
            return;
        }


    } else if (methodName.startsWith("site.download"))
    {

        QString ioMethodName = methodName.replace("site.", "");
        msgObj.insert("method", ioMethodName);

        result = _ioHandler->handleDownload(msgObj);

    } else if (methodName == "site.login")
    {
        QVariantMap sresult;
        sresult.insert("site_name", "wdas");
        sresult.insert("auth_key", "none");
        sresult.insert("username", "guest");
        sresult.insert("first_name", "Guest");
        sresult.insert("last_name", "User");
        sresult.insert("user_id", 1);
        sresult.insert("result", "OK");

        result.setValue(sresult);

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


    } else {

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