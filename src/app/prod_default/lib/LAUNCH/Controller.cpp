
#include "RlpProd/LAUNCH/Controller.h"
#include "RlpProd/LAUNCH/pymodule.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpCore/PY/Interp.h"
#include "RlpCore/QTCORE/QtCoreTypes.h"
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/WIDGET/PyScripting.h"


#include "RlpCore/QTCORE/QtCoreTypes.h"

#include "RlpCore/DS/pymodule.h"
#include "RlpCore/UTIL/pymodule.h"
#include "RlpCore/SEQ/pymodule.h"
#include "RlpCore/NET/pymodule.h"
#include "RlpCore/CNTRL/pymodule.h"
#include "RlpCore/PROC/pymodule.h"

#include "RlpGui/BASE/pymodule.h"
#include "RlpGui/PANE/pymodule.h"
#include "RlpGui/MENU/pymodule.h"
#include "RlpGui/GRID/pymodule.h"
#include "RlpGui/WIDGET/pymodule.h"

#include "RlpGui/QTGUI/QtGuiTypes.h"

#include "RlpRevlens/DS/pymodule.h"
#include "RlpRevlens/MEDIA/pymodule.h"

#include "RlpExtrevlens/RLFFMPEG/pymodule.h"


RLP_SETUP_LOGGER(prod, LAUNCH, Controller)

LAUNCH_Controller::LAUNCH_Controller(GUI_View* view):
     CoreCntrl_ControllerBase(),
    _view(view)
{
    _auth = new CoreCntrl_Auth();

    int port = 8141;
    _wsServer = new CoreNet_WebSocketServer(port, false);

    setWsServer(_wsServer);

    _wsServer->start();

    // _serverThread = new QThread();
}


void
LAUNCH_Controller::initPython()
{
    RLP_LOG_INFO("")
    
    #ifdef SCAFFOLD_PYBIND_STATIC
    RlpCoreQTCOREmodule_bootstrap();
    RlpGuiQTGUImodule_bootstrap();

    RlpCoreDSmodule_bootstrap();
    RlpCoreUTILmodule_bootstrap();
    RlpCoreSEQmodule_bootstrap();
    RlpCoreNETmodule_bootstrap();
    RlpCoreCNTRLmodule_bootstrap();
    RlpCorePROCmodule_bootstrap();

    RlpGuiBASEmodule_bootstrap();
    RlpGuiPANEmodule_bootstrap();
    RlpGuiMENUmodule_bootstrap();
    RlpGuiGRIDmodule_bootstrap();
    RlpGuiWIDGETmodule_bootstrap();

    RlpRevlensDSmodule_bootstrap();
    RlpRevlensMEDIAmodule_bootstrap();

    RlpExtrevlensRLFFMPEGmodule_bootstrap();

    RlpProdLAUNCHmodule_bootstrap();
    #endif

    PY_Interp::init();

    // need this before FS_CNTRL is exported to register parent 
    PY_Interp::eval("import RlpCoreQTCORE");
    PY_Interp::eval("import RlpGuiQTGUI");
    
    PY_Interp::eval("import RlpCoreCNTRLmodule");

    // need this before VIEW is exported to register parent classes
    PY_Interp::eval("import RlpGuiBASEmodule");

    PY_Interp::eval("import RlpCoreUTILmodule");
    PY_Interp::eval("import RlpGuiPANEmodule");
    
    PY_Interp::eval("import RlpRevlensDSmodule");
    PY_Interp::eval("import RlpRevlensMEDIAmodule");


    PY_Interp::eval("import RlpProdLAUNCHmodule");

    PY_Interp::bindAppObject("RlpCoreUTILmodule", "APPGLOBALS", UTIL_AppGlobals::instance());
    PY_Interp::bindAppObject("RlpGuiPANEmodule", "VIEW", _view);
    PY_Interp::bindAppObject("RlpProdLAUNCHmodule", "CNTRL", this);

    PY_Interp::eval("import rlp.core.rlp_logging;rlp.core.rlp_logging._init_rlp_logging()");
}


void
LAUNCH_Controller::initArrangement()
{
    RLP_LOG_INFO("")

    GUI_Pane* rp = _view->rootPane();

    rp->setOrientation(O_VERTICAL);

    rp->split();
    rp->splitter(0)->setPosPixels(30);
    rp->pane(0)->setHeaderVisible(false);
    rp->splitter(0)->setActive(false);

    rp->updatePanes();

    // Main top menubar / toolbar
    //
    GUI_MenuBar* globalMenuBar = new GUI_MenuBar(rp->pane(0)->body());
    QObject::connect(
        globalMenuBar,
        &GUI_MenuBar::menuShown,
        _view->scene(),
        &GUI_Scene::requestMenuBarMenuShown
    );

    QObject::connect(
        globalMenuBar,
        &GUI_MenuBar::menuHidden,
        _view->scene(),
        &GUI_Scene::requestMenuBarMenuHidden
    );

    QObject::connect(
        _view->scene(),
        &GUI_Scene::menuPanesHidden,
        globalMenuBar,
        &GUI_MenuBar::onMenuPanesHidden
    );


    _view->registerTool(
        "PyConsole",
        &GUI_PyScripting::create
    );


    _menuBar = globalMenuBar;
}


void
LAUNCH_Controller::initLayout()
{
    RLP_LOG_DEBUG("")

    GUI_PanePtr p = _view->rootPane()->pane(1);

    QStringList appList = {"Launch"};

    for (auto toolName : appList)
    {
        GUI_ToolInfo* toolInfo = _view->tool(toolName);
        RLP_LOG_DEBUG("Loading Tool:" << toolName << toolInfo->data())

        QVariantMap sigInfo;
        sigInfo.insert("text", toolName);
        sigInfo.insert("data", toolInfo->data());

        p->header()->onActionItemSelected(sigInfo);
    }

    p->onTabActivateRequested("Launch");
}


void
LAUNCH_Controller::initPyScript()
{
    RLP_LOG_DEBUG(_startupPyScript);

    QFile f(_startupPyScript);
    if (f.exists())
    {
        PY_Interp::evalFile(_startupPyScript);
    } else
    {
        PY_Interp::eval("import " + _startupPyScript);
    }
    
}


void
LAUNCH_Controller::startup()
{
    RLP_LOG_DEBUG("")

    initPython();
    initArrangement();

    PY_Interp::eval("import revlens_prod.launcher;revlens_prod.launcher.startup()");

    emit startupReady();
}


void
LAUNCH_Controller::showWindow(bool /* unused */)
{
    RLP_LOG_INFO("")

    _view->show();
    _view->raise();
}


void
LAUNCH_Controller::quit(bool /* unused */)
{
    RLP_LOG_INFO("Exiting")

    exit(0);
}


void
LAUNCH_Controller::openTool(QString toolName)
{
    RLP_LOG_DEBUG(toolName)


    GUI_PanePtr p = _view->rootPane()->pane(1);

    GUI_ToolInfo* toolInfo = _view->tool(toolName);
    RLP_LOG_DEBUG("Loading Tool:" << toolName << toolInfo->data())

    QVariantMap sigInfo;
    sigInfo.insert("text", toolName);
    sigInfo.insert("data", toolInfo->data());

    p->header()->onActionItemSelected(sigInfo);
    
    p->onTabActivateRequested(toolName);
}


void
LAUNCH_Controller::registerMediaPlugin(MEDIA_Plugin* plugin)
{
    MEDIA_Factory::instance()->registerMediaPlugin(plugin);
}


void
LAUNCH_Controller::registerMediaLocator(MEDIA_Locator* locator)
{
    MEDIA_Factory::instance()->registerMediaLocator(locator);
}


void
LAUNCH_Controller::onWsServerReady(int port)
{
    RLP_LOG_DEBUG(port)

    QString ioServicePath = "rlp_io_service_start";

    QStringList args;
    args.append(QString("%1").arg(port)); // wsServer->serverPort()));

    RLP_LOG_DEBUG("Starting IOService: " << ioServicePath << args)

    bool result = QProcess::startDetached(ioServicePath, args);
    RLP_LOG_DEBUG(result)
}


void
LAUNCH_Controller::handleRemoteCall(QVariantMap msgObj)
{
    // LOG_Logging::pprint(msgObj);

    QString methodName = msgObj.value("method").toString();

    if (methodName == "register_service")
    {
        QVariantList argList = msgObj.value("args").toList();
        QString serviceName = argList.at(0).toString();
        QString serviceIdent = msgObj.value("ident").toString();

        RLP_LOG_DEBUG("Registering service " << serviceName << " - " << serviceIdent)

        _wsServer->registerService(serviceName, serviceIdent);

    }
    else if (methodName == "media.locate")
    {
        // LOG_Logging::pprint(msgObj);

        QString clientIdent = msgObj.value("ident").toString();
        QVariantMap session = msgObj.value("session").toMap();
        QString runId = session.value("run_id").toString();

        MEDIA_Factory* factory = MEDIA_Factory::instance();

        QVariantList margs = msgObj.value("args").toList();
        QVariantMap mediaInput = margs.at(0).toMap();

        RLP_LOG_DEBUG("Running media.locate:" << mediaInput)

        // We're expecting this to be called with an RPCFuture / callback
        // setup, so just send as-is

        factory->locate(mediaInput);
        factory->insertInitialMediaProperties(&mediaInput);

        QVariantMap result = factory->identifyMedia2(mediaInput); // , DS_Node::DATA_VIDEO);

        // LOG_Logging::pprint(result);

        QVariantMap resp;
        resp.insert("ident", clientIdent);
        resp.insert("run_id", runId);
        resp.insert("result", result);

        _wsServer->sendCallToClient(resp);
    }
    else if ((methodName == "append") &&
               (msgObj.contains("run_id")) &&
               (msgObj.contains("kwargs")))
    {
        QString runId = msgObj.value("run_id").toString();
        QString ident = msgObj.value("ident").toString();

        _runIdIdentMap.insert(runId, ident);

        _wsServer->sendCallToService("iosched", msgObj);
    }
    else if (methodName == "__logrecord__")
    {
        QString clientIdent = msgObj.value("ident").toString();
        QVariantMap result = msgObj.value("result").toMap();

        QString levelName = result.value("levelname").toString();
        if (levelName == "RESULT")
        {
            QVariantMap extra = result.value("extra").toMap();

            if (extra.contains("run_id"))
            {
                QString runId = extra.value("run_id").toString();

                if (_runIdIdentMap.contains(runId))
                {
                    QString clientIdent = _runIdIdentMap.value(runId);

                    QVariantMap resp;
                    resp.insert("ident", clientIdent);
                    resp.insert(result);

                    _wsServer->sendCallToClient(resp);

                    _runIdIdentMap.remove(runId);

                }
            }

        }
    }
}

