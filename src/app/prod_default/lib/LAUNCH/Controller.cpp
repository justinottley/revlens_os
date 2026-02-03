
#include "RlpProd/LAUNCH/Controller.h"
#include "RlpProd/LAUNCH/pymodule.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpCore/PY/Interp.h"
#include "RlpCore/QTCORE/QtCoreTypes.h"
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/WIDGET/PyScripting.h"


#include "RlpCore/QTCORE/QtCoreTypes.h"
#include "RlpCore/CNTRL/pymodule.h"
#include "RlpCore/UTIL/pymodule.h"
#include "RlpCore/NET/pymodule.h"
#include "RlpCore/PROC/pymodule.h"

#include "RlpGui/BASE/pymodule.h"
#include "RlpGui/PANE/pymodule.h"
#include "RlpGui/MENU/pymodule.h"
#include "RlpGui/GRID/pymodule.h"
#include "RlpGui/WIDGET/pymodule.h"

#include "RlpGui/QTGUI/QtGuiTypes.h"

RLP_SETUP_LOGGER(prod, LAUNCH, Controller)

LAUNCH_Controller::LAUNCH_Controller(GUI_View* view):
     CoreCntrl_ControllerBase(),
    _view(view)
{
    _auth = new CoreCntrl_Auth();
    _serverThread = new QThread();
}


void
LAUNCH_Controller::initPython()
{
    RLP_LOG_INFO("")
    
    #ifdef SCAFFOLD_PYBIND_STATIC
    RlpCoreQTCOREmodule_bootstrap();
    RlpGuiQTGUImodule_bootstrap();

    RlpCoreUTILmodule_bootstrap();
    RlpCoreNETmodule_bootstrap();
    RlpCoreCNTRLmodule_bootstrap();
    RlpCorePROCmodule_bootstrap();

    RlpGuiBASEmodule_bootstrap();
    RlpGuiPANEmodule_bootstrap();
    RlpGuiMENUmodule_bootstrap();
    RlpGuiGRIDmodule_bootstrap();
    RlpGuiWIDGETmodule_bootstrap();

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