
#include "RlpRevlens/GUI/MainWindowArrangement.h"

#include "RlpCore/PY/Interp.h"

#include "RlpGui/WIDGET/PyScripting.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI/FileBrowser.h"
#include "RlpRevlens/DISP/GLView.h"

#include "RlpRevlens/GUI/ViewerPane.h"


RLP_SETUP_LOGGER(revlens, GUI, MainWindowArrangement)

GUI_MainWindowArrangement::GUI_MainWindowArrangement():
    _presentationWindow(nullptr),
    _presentationViewer(nullptr)
{

}


void
GUI_MainWindowArrangement::init(GUI_View* view, CNTRL_MainController* cntrl)
{
    RLP_LOG_DEBUG("")


    _mainView = view;

    GUI_Pane* rp = view->rootPane();

    rp->setOrientation(O_VERTICAL);

    rp->split();
    rp->splitter(0)->setPosPixels(25);
    rp->pane(0)->setHeaderVisible(false);
    
    // rp->splitter(0)->setVisible(false);
    rp->splitter(0)->setActive(false);

    // Keyboard shortcuts
    //
     QObject::connect(
        view->scene(),
        &GUI_Scene::keyPressEventInternal,
        cntrl,
        &CNTRL_MainController::keyPressEventInternal
    );

    // Main top menubar / toolbar
    //
    _globalMenuBar = new GUI_MenuBar(rp->pane(0)->body());
    connect(
        _globalMenuBar,
        &GUI_MenuBar::menuShown,
        view->scene(),
        &GUI_Scene::requestMenuBarMenuShown
    );

    connect(
        _globalMenuBar,
        &GUI_MenuBar::menuHidden,
        view->scene(),
        &GUI_Scene::requestMenuBarMenuHidden
    );

    connect(view->scene(), &GUI_Scene::menuPanesHidden, _globalMenuBar, &GUI_MenuBar::onMenuPanesHidden);

    CNTRL_PluginManager* mgr = cntrl->getPluginManager();
    connect(mgr, &CNTRL_PluginManager::registerToolRequested, view, &GUI_View::registerToolGUI);

    
    view->registerTool(
        "Files",
        &GUI_FileBrowser::create
    );


    QVariantMap viewerToolMd;
    viewerToolMd.insert("movable", false);
    view->registerTool(
        "Viewer",
        &GUI_ViewerPane::create,
        viewerToolMd
    );


    QVariantMap timelineToolMd;
    timelineToolMd.insert("movable", false);
    view->registerTool(
        "Timeline",
        &GUI_TL2_View::create,
        timelineToolMd
    );


    view->registerTool(
        "PyConsole",
        &GUI_PyScripting::create
    );

    PY_Interp::bindAppObject("RlpRevlensGUImodule", "GUI", this);

    _arr = new GUI_ArrangementBase();
    _arr->init(rp->pane(1), view, cntrl);

    connect(
        _arr,
        &GUI_ArrangementBase::arrangementLoaded,
        this,
        &GUI_MainWindowArrangement::arrangementLoaded
    );
}


GUI_MenuButton*
GUI_MainWindowArrangement::addMenu(QString menuName)
{
    RLP_LOG_DEBUG(menuName)

    return _globalMenuBar->addMenu(menuName);
}


GUI_View*
GUI_MainWindowArrangement::createNewWindow()
{
    RLP_LOG_DEBUG("Creating new window")

    GUI_View* v = new GUI_View();

    // propagate selection changed to main view
    connect(
        v,
        &GUI_View::selectionChanged,
        _mainView,
        &GUI_View::emitSelectionChanged
    );

    // Ensure new window knows about all the existing tools
    //
    QStringList toolNames = _mainView->toolNames();
    for (int i=0; i!=toolNames.size(); ++i)
    {
        QString toolName = toolNames.at(i);
        v->registerToolInternal(toolName, _mainView->tool(toolName));
    }

    v->rootPane()->header()->initActButtonToolsPrivate();
    v->rootPane()->setHeaderVisible(true);
    v->resize(800, 600);
    v->show();

    QObject::connect(
        v->scene(),
        &GUI_Scene::keyPressEventInternal,
        CNTRL_MainController::instance(),
        &CNTRL_MainController::keyPressEventInternal
    );

    return v;
}


void
GUI_MainWindowArrangement::initPresentationWindow(bool showOverlayTimeline)
{
    if (_presentationWindow != nullptr)
    {
        RLP_LOG_WARN("Presentation window already initialized")
        return;
    }

    GUI_View* v = createNewWindow();

    QString toolName = "Viewer";

    GUI_ToolInfo* toolInfo = v->tool(toolName);
    RLP_LOG_DEBUG("Loading Tool:" << toolName << toolInfo->data())

    QVariantMap sigInfo;
    sigInfo.insert("text", toolName);
    sigInfo.insert("data", toolInfo->data());

    QQuickItem* item = v->rootPane()->header()->onActionItemSelected(sigInfo);

    v->setTabHeadersVisible(false);

    if (item != nullptr)
    {
        GUI_ViewerPane* vp = qobject_cast<GUI_ViewerPane*>(item);
        vp->setPresentationMode(true, showOverlayTimeline);
        _presentationViewer = vp;
    }
    
    _presentationWindow = v;

}


GUI_View*
GUI_MainWindowArrangement::presentationWindow()
{
    if (_presentationWindow == nullptr)
    {
        initPresentationWindow();
    }

    return _presentationWindow;
}


void
GUI_MainWindowArrangement::setPresentationWindowSize(int width, int height)
{
    _presentationWindow->setWidth(width);
    _presentationWindow->setHeight(height);

    _presentationViewer->onParentSizeChanged(width, height);
}


void
GUI_MainWindowArrangement::enablePresentationMode()
{
    RLP_LOG_DEBUG("")

    _presentationViewer->view()->enableFullscreen();
    _presentationWindow->showFullScreen();
}


void
GUI_MainWindowArrangement::disablePresentationMode()
{
    RLP_LOG_DEBUG("")

    _presentationViewer->view()->disableFullscreen();
    _presentationWindow->close();
}


void
GUI_MainWindowArrangement::_chooseDirectoryInternal(QString callId)
{
    // This rigmarole is to support calling the QWidget based QFileDialog
    // without messy python thread / GIL issues

    QString dir = QFileDialog::getExistingDirectory(
        nullptr,
        tr("Open Directory"),
        QDir::homePath(), 
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    emit directoryChosen(callId, dir);
}


void
GUI_MainWindowArrangement::chooseDirectory(QString callId)
{
    // This rigmarole is to support calling the QWidget based QFileDialog
    // without messy python thread / GIL issues

    QTimer::singleShot(10, this, [callId, this]() {
        _chooseDirectoryInternal(callId);
    });

}
