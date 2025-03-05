
#include "RlpCore/PY/Interp.h"

#include "RlpGui/PANE/ViewQuick.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/PANE/Pane.h"
#include "RlpGui/PANE/PaneSplitter.h"

RLP_SETUP_LOGGER(gui, GUI, View)

GUI_View::GUI_View():
    QQuickWindow(),
    _uuid(QUuid::createUuid().toString()),
    _scene(nullptr),
    _rootPane(nullptr),
    _tabHeadersVisible(true),
    _splittersVisible(true),
    _focusItem(nullptr)
{
    RLP_LOG_DEBUG("")

    #ifdef __MACH__
    // macOS - set OpenGL compatibility profile for shaders to work
    QSurfaceFormat f;
    f.setProfile(QSurfaceFormat::CompatibilityProfile);
    setFormat(f);
    #endif

    connect(this, &GUI_View::sceneGraphInitialized, this, &GUI_View::handleSceneGraphInitialized, Qt::DirectConnection);
    connect(this, &GUI_View::afterAnimating, this, &GUI_View::handleAfterAnimating);
    connect(this, &GUI_View::beforeFrameBegin, this, &GUI_View::handleBeforeFrameBegin, Qt::DirectConnection);
    connect(this, &GUI_View::afterFrameEnd, this, &GUI_View::handleAfterFrameEnd, Qt::DirectConnection);

    _tools.clear();
    initScene();
}


void
GUI_View::initScene()
{
    RLP_LOG_DEBUG("")

    GUI_Scene* scene = new GUI_Scene(contentItem());
    setScene(scene);

    GUI_Pane* pane = new GUI_Pane(nullptr);
    pane->setName("ROOT");
    scene->addItem(pane);

    pane->initPane();

    _rootPane = pane;
}


void
GUI_View::setScene(GUI_Scene* scene)
{
    Q_ASSERT(_scene == nullptr);

    _scene = scene;
    _scene->setView(this);
    _scene->setParent(contentItem());
    _scene->setParentItem(contentItem());

    connect(
        _scene,
        &GUI_Scene::updateRequested,
        this,
        &GUI_View::forceUpdate
    );
}


void
GUI_View::_emitSelectionChangedPrivate()
{
    RLP_LOG_DEBUG("")
    emit selectionChanged(_selInfo);
}


void
GUI_View::emitSelectionChanged(QVariantMap info)
{
    RLP_LOG_DEBUG("")

    // RLP_LOG_DEBUG(info)
    _selInfo = info;
    QTimer::singleShot(100, this, &GUI_View::_emitSelectionChangedPrivate);
}


void
GUI_View::forceUpdate()
{
    // RLP_LOG_DEBUG("")
    // update();
}


void
GUI_View::resizeEvent(QResizeEvent* event)
{
    QQuickWindow::resizeEvent(event);

    _scene->setSize(QSizeF(event->size().width(), event->size().height()));
}


void
GUI_View::keyPressEvent(QKeyEvent* event)
{
    QQuickWindow::keyPressEvent(event);

    if (!event->isAccepted())
    {
        _scene->keyPressEvent(event);
    }
}


void
GUI_View::keyReleaseEvent(QKeyEvent* event)
{
    QQuickWindow::keyReleaseEvent(event);

    if (!event->isAccepted())
    {
        _scene->keyReleaseEvent(event);
    }
}


void
GUI_View::registerTool(QString toolName, WidgetCreateFunc func, QVariantMap toolInfoMd)
{
    // RLP_LOG_INFO(toolName)

    toolInfoMd.insert("item_type", "tool");

    GUI_ToolInfo* toolInfo = new GUI_ToolInfo(toolInfoMd);
    toolInfo->setCreateFunc(func);

    _tools.insert(toolName, toolInfo);
    
    emit toolAdded(toolName);
}


void
GUI_View::registerToolInternal(QString toolName, GUI_ToolInfo* tool)
{
    RLP_LOG_DEBUG(toolName)
    _tools.insert(toolName, tool);
}


void
GUI_View::registerToolGUI(QVariantMap toolInfoIn)
{
    // RLP_LOG_INFO(toolInfoIn)

    GUI_ToolInfo* toolInfo = new GUI_ToolInfo(toolInfoIn);
    toolInfo->setCreateFunc(&GUI_ItemBase::create);

    QString toolName = toolInfoIn.value("name").toString();

    _tools.insert(toolName, toolInfo);

    emit toolAdded(toolName);
}



void
GUI_View::setTabHeadersVisible(bool visible)
{
    RLP_LOG_DEBUG(visible)

    _tabHeadersVisible = visible;

    _rootPane->setHeaderVisible(visible, true /* recursive */);
}


void
GUI_View::setSplittersVisible(bool visible)
{
    // RLP_LOG_DEBUG(visible)

    _splittersVisible = visible;
    // _rootPane->setSplittersVisible(visible, true /* recursive */);
    GUI_PaneSplitter::AUTOHIDE = !visible;

    update();
}


void
GUI_View::setFocusItem(QQuickItem* item)
{
    //RLP_LOG_DEBUG(item)
    _focusItem = item;
}


void
GUI_View::clearFocusItem(QQuickItem* item)
{
    if (_focusItem == item)
    {
        //RLP_LOG_DEBUG("Clearing" << item)
        _focusItem = nullptr;
    }
}


GUI_PaneFrame*
GUI_View::createFloatingPane(int pwidth, int pheight, bool obscuredModal)
{
    // RLP_LOG_DEBUG(pwidth << " " << pheight)

    hideAllMenuPanes();

    FloatingPanePosition pos = FloatingPosCenter;

    GUI_PaneFrame* p = new GUI_PaneFrame(pwidth, pheight);
    p->setResizeToParent(false);

    if (obscuredModal)
    {
        // RLP_LOG_DEBUG("ADDING OBSCURE MODAL")

        GUI_FrameOverlay* fo = new GUI_FrameOverlay(width(), height(), nullptr);
        p->setOverlay(fo);

        /*
        connect(
            _scene,
            &GUI_Scene::sceneRectChanged,
            fo,
            &GUI_FrameOverlay::onSizeRectChanged
        );
        */
        _scene->addItem(fo);

    }

    _scene->addItem(p);

    p->initPane();
    p->setBgCol(QColor(30, 30, 30)); // Qt::black); // Grey);

    if (obscuredModal)
    {
        p->setCloseButtonVisible(false);
    }


    if (pos == FloatingPosCenter)
    {
        int posX = (width() - pwidth) / 2;
        int posY = (height() - pheight) / 2;

        // TODO FIXME: width / height issues are preventing this
        // from working properly
        // p->setPos(posX, posY);
        //
        p->setPos(posX, posY);

        RLP_LOG_DEBUG("width: " << width() << " height: " << height() << " Pane position: " << posX << " " << posY)
    }


    _scene->requestFloatingPaneOpened();

    return p;

}


void
GUI_View::hideAllMenuPanes()
{
    _scene->hideMenuPanes();
}


void
GUI_View::handleAfterAnimating()
{
    PY_Interp::releaseGIL();
}

void
GUI_View::handleBeforeFrameBegin()
{
    PY_Interp::acquireGIL(_uuid);
}


void
GUI_View::handleAfterFrameEnd()
{
    PY_Interp::releaseGIL();
    emit frameReady();
}


void
GUI_View::handleSceneGraphInitialized()
{
    if (PY_Interp::initThreadState(_uuid))
    {
        RLP_LOG_DEBUG("Python Render Thread State Initialized")
        emit pyGuiReady();
    }
}