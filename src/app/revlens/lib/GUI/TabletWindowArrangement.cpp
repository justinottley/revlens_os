
#include "RlpRevlens/GUI/TabletWindowArrangement.h"

#include "RlpGui/PANE/View.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/PluginManager.h"

// Built-in Tools
//
#include "RlpGui/WIDGET/PyScripting.h"
#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI/FileBrowser.h"
#include "RlpRevlens/GUI/ViewerPane.h"


RLP_SETUP_LOGGER(revlens, GUI, TabletWindowArrangement)

GUI_TabletWindowArrangement::GUI_TabletWindowArrangement()
{
}


void
GUI_TabletWindowArrangement::init(GUI_View* view, CNTRL_MainController* cntrl)
{
    RLP_LOG_DEBUG("")

    _mainView = view;

    // Keyboard shortcuts
    //
    QObject::connect(
        view->scene(),
        &GUI_Scene::keyPressEventInternal,
        cntrl,
        &CNTRL_MainController::keyPressEventInternal
    );


    CNTRL_PluginManager* mgr = cntrl->getPluginManager();
    connect(mgr, &CNTRL_PluginManager::registerToolRequested, view, &GUI_View::registerToolGUI);

    view->registerTool(
        "Files",
        &GUI_FileBrowser::create
    );


    view->registerTool(
        "Viewer",
        &GUI_ViewerPane::create
    );


    view->registerTool(
        "Timeline",
        &GUI_TL2_View::create
    );


    view->registerTool(
        "PyConsole",
        &GUI_PyScripting::create
    );

    PY_Interp::bindAppObject("RlpRevlensGUImodule", "GUI", this);

    _arr = new GUI_ArrangementBase();
    // _arr->init(view->rootPane(), view, cntrl);

    connect(
        _arr,
        &GUI_ArrangementBase::arrangementLoaded,
        this,
        &GUI_TabletWindowArrangement::arrangementLoaded
    );

}