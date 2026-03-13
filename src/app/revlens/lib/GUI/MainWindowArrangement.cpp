
#include "RlpRevlens/GUI/MainWindowArrangement.h"

#include "RlpCore/PY/Interp.h"

#include "RlpGui/WIDGET/PyScripting.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI/FileBrowser.h"
#include "RlpRevlens/DISP/GLView.h"

#include "RlpRevlens/GUI/StatusMessage.h"

#include "RlpRevlens/GUI_VIEW/Pane.h"


#include <qnanopainter.h>

RLP_SETUP_LOGGER(revlens, GUI, MainWindowArrangement)

GUI_MainWindowArrangement* GUI_MainWindowArrangement::_INSTANCE = nullptr;

GUI_MainWindowArrangement::GUI_MainWindowArrangement():
    _mainView(nullptr),
    _presentationWindow(nullptr),
    _presentationViewer(nullptr)
{
    _tools = new GUI_View::ToolInfoMap();

    _arr = new GUI_ArrangementLoader();
    connect(
        _arr,
        &GUI_ArrangementLoader::arrangementLoaded,
        this,
        &GUI_MainWindowArrangement::arrangementLoaded
    );
}


void
GUI_MainWindowArrangement::init(GUI_View* view)
{
    RLP_LOG_DEBUG("")

    #ifdef __MACH__
    #ifndef SCAFFOLD_IOS
    if (_mainView != nullptr)
    {
        // Fix for not painting new windows on macOS. Results in (permanently) slower performance for
        // new widgets, since this makes a new painter instance for every widget instead of reusing the
        // same instance. Ideally this could be fixed inside QNanoPainter instead.
        QNanoPainter::setNewInstance(true);
    }
    #endif
    #endif

    if (_mainView != nullptr)
    {
        QObject::disconnect(_mainView->scene(), &GUI_Scene::menuPanesHidden, nullptr, nullptr);
        QObject::disconnect(_globalMenuBar, &GUI_MenuBar::menuShown, nullptr, nullptr);
        QObject::disconnect(_globalMenuBar, &GUI_MenuBar::menuHidden, nullptr, nullptr);

        // _mainView->deleteLater();
        // _mainView = nullptr;

        _mainView->destroyScene();
        _mainView->setVisible(false);
    }


    if (view == nullptr)
    {
        RLP_LOG_INFO("Construcing new GUI_View")
        view = new GUI_View();
    }

    _mainView = view;

    view->registerToolInfoMap(_tools);

    connect(
        this,
        &GUI_MainWindowArrangement::registerGUIToolRequested,
        view,
        &GUI_View::registerToolGUI
    );


    connect(
        view,
        &GUI_View::pyGuiReady,
        this,
        &GUI_MainWindowArrangement::pyGuiReady
    );

    CNTRL_MainController* cntrl = CNTRL_MainController::instance();

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

    GUI_StatusMessage* sm = new GUI_StatusMessage(_globalMenuBar);

    connect(
        cntrl,
        &CNTRL_MainController::statusEvent,
        sm,
        &GUI_StatusMessage::onStatusEvent
    );

    _globalMenuBar->addRightItem(sm);

    CNTRL_PluginManager* mgr = cntrl->getPluginManager();
    connect(mgr, &CNTRL_PluginManager::registerToolRequested, view, &GUI_View::registerToolGUI);

    // This is/was intended to allow loading in a C++ only environment,
    // even without any python interpreter available
    // view->registerTool(
    //     "Files",
    //     &GUI_FileBrowser::create
    // );


    QVariantMap viewerToolMd;
    viewerToolMd.insert("movable", false);

    // view->registerTool(
    //     "Viewer",
    //     &GUI_ViewerPane::create,
    //     viewerToolMd
    // );

    // view->registerTool(
    //     "TLViewer",
    //     &GUI_ViewerTimelinePane::create,
    //     viewerToolMd
    // );

    view->registerTool(
        "Viewer",
        &GUI_VIEW_Pane::create,
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


    // _arr->init(rp->pane(1), view, cntrl);
}


void
GUI_MainWindowArrangement::registerToolGUI(QVariantMap toolInfoIn)
{
    // RLP_LOG_INFO(toolInfoIn)

    GUI_ToolInfo* toolInfo = new GUI_ToolInfo(toolInfoIn);
    toolInfo->setCreateFunc(&GUI_ItemBase::create);

    QString toolName = toolInfoIn.value("name").toString();

    _tools->insert(toolName, toolInfo);

    emit registerGUIToolRequested(toolInfoIn);

    // emit toolAdded(toolName);
}


void
GUI_MainWindowArrangement::loadArrangement(QVariantMap arrangementData, bool initView)
{
    RLP_LOG_DEBUG("")

    if (initView)
    {
        init();
    }


    if (arrangementData.contains("window"))
    {
        QVariantMap windowInfo = arrangementData.value("window").toMap();
        QVariantMap windowSizeInfo = windowInfo.value("size").toMap();
        
        int width = windowSizeInfo.value("width").toInt();
        int height = windowSizeInfo.value("height").toInt();

        RLP_LOG_DEBUG("Resizing Window:" << width << "x" << height)

        QRect screenRect = _mainView->screen()->availableGeometry();

        int xposOffset = (screenRect.width() - width) / 2;
        int yposOffset = (screenRect.height() - height) / 2;
        
        QPoint screenPos = QPoint(
            screenRect.x() + xposOffset,
            screenRect.y() + yposOffset
        );

        
        QScreen* screen = _mainView->screen();
        QSize screenSize = screen->availableGeometry().size();
        if (width > screenSize.width())
        {
            width = screenSize.width() - 10;
            screenPos = QPoint(0, 0);
        }

        if (height > screenSize.height())
        {
            height = screenSize.height() - 10;
            screenPos = QPoint(0, 0);
        }

        _mainView->setPosition(screenPos);
        _mainView->resize(width, height);
        _mainView->show();
    }

    QVariantList args;
    PY_Interp::call("revlens.gui.menu.clear_global_ui", args);

    QVariantMap arrData = arrangementData.value("arrangement").toMap();
    _arr->loadArrangement(mainPane(), arrData);
}


GUI_MenuButton*
GUI_MainWindowArrangement::addMenu(QString menuName)
{
    RLP_LOG_DEBUG(menuName)

    return _globalMenuBar->addMenu(menuName);
}


void
GUI_MainWindowArrangement::hideMenuPanes()
{
    RLP_LOG_DEBUG("")

    _mainView->scene()->hideMenuPanes();
}


GUI_View*
GUI_MainWindowArrangement::createNewWindow()
{
    RLP_LOG_DEBUG("Creating new window")

    #ifdef __MACH__
    #ifndef SCAFFOLD_IOS
    QNanoPainter::setNewInstance(true);
    #endif
    #endif


    GUI_View* v = new GUI_View();

    // propagate selection changed to main view
    connect(
        v,
        &GUI_View::selectionChanged,
        _mainView,
        &GUI_View::emitSelectionChanged
    );

    v->registerToolInfoMap(_tools);

    /*
    // Ensure new window knows about all the existing tools
    //
    QStringList toolNames = _mainView->toolNames();
    for (int i=0; i!=toolNames.size(); ++i)
    {
        QString toolName = toolNames.at(i);
        v->registerToolInternal(toolName, _mainView->tool(toolName));
    }
    */

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

    // It appears, once we make a new window, QNanoPainter sharing has to be turned
    // off after that point..??
    // QTimer::singleShot(10, this, &GUI_MainWindowArrangement::endNewWindow);

    return v;
}


void
GUI_MainWindowArrangement::initPresentationWindow(bool showTimeline)
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
        GUI_VIEW_Pane* vp = qobject_cast<GUI_VIEW_Pane*>(item);
        vp->setPresentationMode(true, showTimeline);
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
