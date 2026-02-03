
#include "RlpRevlens/GUI/TabletWindowArrangement.h"

#include "RlpGui/PANE/View.h"
#include "RlpGui/WIDGET/PyScripting.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/PluginManager.h"

// Built-in Tools
//
#include "RlpRevlens/GUI_VIEW/Pane.h"
#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI/FileBrowser.h"

RLP_SETUP_LOGGER(revlens, GUI, TabletWindowArrangement)


GUI_TabletWindowArrangement* GUI_TabletWindowArrangement::_INSTANCE = nullptr;

GUI_TabletWindowArrangement::GUI_TabletWindowArrangement()
{
    _tools = new GUI_View::ToolInfoMap();
    _arr = new GUI_ArrangementLoader();
    connect(
        _arr,
        &GUI_ArrangementLoader::arrangementLoaded,
        this,
        &GUI_TabletWindowArrangement::arrangementLoaded
    );
}


void
GUI_TabletWindowArrangement::init()
{
    RLP_LOG_DEBUG("")

    _mainView = new GUI_View();

    _mainView->registerToolInfoMap(_tools);

    connect(
        this,
        &GUI_TabletWindowArrangement::registerGUIToolRequested,
        _mainView,
        &GUI_View::registerToolGUI
    );

    connect(
        _mainView,
        &GUI_View::pyGuiReady,
        this,
        &GUI_TabletWindowArrangement::pyGuiReady
    );

    GUI_Pane* rp = _mainView->rootPane();
    rp->setOrientation(O_HORIZONTAL);
    rp->split();
    rp->splitter(0)->setPosPixels(50);
    rp->splitter(0)->setActive(false);

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();

    // Keyboard shortcuts
    //
    QObject::connect(
        _mainView->scene(),
        &GUI_Scene::keyPressEventInternal,
        cntrl,
        &CNTRL_MainController::keyPressEventInternal
    );


    CNTRL_PluginManager* mgr = cntrl->getPluginManager();
    connect(
        mgr,
        &CNTRL_PluginManager::registerToolRequested,
        _mainView,
        &GUI_View::registerToolGUI
    );

    _mainView->registerTool(
        "Files",
        &GUI_FileBrowser::create
    );


    QVariantMap viewerToolMd;
    viewerToolMd.insert("movable", false);

    _mainView->registerTool(
        "Viewer",
        &GUI_VIEW_Pane::create,
        viewerToolMd
    );

    QVariantMap timelineToolMd;
    timelineToolMd.insert("movable", false);
    _mainView->registerTool(
        "Timeline",
        &GUI_TL2_View::create,
        timelineToolMd
    );

    _mainView->registerTool(
        "PyConsole",
        &GUI_PyScripting::create
    );
}


void
GUI_TabletWindowArrangement::registerToolGUI(QVariantMap toolInfoIn)
{
    // RLP_LOG_INFO(toolInfoIn)

    GUI_ToolInfo* toolInfo = new GUI_ToolInfo(toolInfoIn);
    toolInfo->setCreateFunc(&GUI_ItemBase::create);

    QString toolName = toolInfoIn.value("name").toString();
    RLP_LOG_DEBUG(toolName)

    _tools->insert(toolName, toolInfo);

    emit registerGUIToolRequested(toolInfoIn);

    // emit toolAdded(toolName);
}


void
GUI_TabletWindowArrangement::hideMenuPanes()
{
    RLP_LOG_DEBUG("")

    _mainView->scene()->hideMenuPanes();
}


void
GUI_TabletWindowArrangement::loadArrangement(QVariantMap arrangementData, bool initView)
{
    RLP_LOG_DEBUG("")

    // QVariantList args;
    // PY_Interp::call("revlens.gui.clear_global_ui", args);

    QVariantMap arrData = arrangementData.value("arrangement").toMap();
    _arr->loadArrangement(mainPane(), arrData);
}