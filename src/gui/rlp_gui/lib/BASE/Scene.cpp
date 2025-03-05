

#include "RlpGui/BASE/Scene.h"

#include "RlpGui/BASE/FrameBase.h"
#include "RlpGui/BASE/MenuPaneBase.h"
#include "RlpGui/BASE/GLItem.h"
#include "RlpGui/BASE/ToolTip.h"

RLP_SETUP_LOGGER(gui, GUI, Scene)

GUI_Scene::GUI_Scene(QQuickItem* parent):
    QQuickItem(parent),
    _focusedPane(nullptr),
    _dragItem(nullptr)
{
    RLP_LOG_DEBUG("")

    _glItems.clear();

    _toolTip = new GUI_ToolTip(nullptr);
    _toolTip->setVisible(false);
    addItem(_toolTip);

    connect(this, &GUI_Scene::widthChanged, this, &GUI_Scene::onSizeChangedInternal);
    connect(this, &GUI_Scene::heightChanged, this, &GUI_Scene::onSizeChangedInternal);
}


void
GUI_Scene::onSizeChangedInternal()
{
    // RLP_LOG_DEBUG("")

    emit sizeChanged(width(), height());
}


void
GUI_Scene::keyPressEvent(QKeyEvent* event)
{
    // RLP_LOG_DEBUG(event)

    QQuickItem::keyPressEvent(event);

    if (!event->isAccepted()) {
        // RLP_LOG_DEBUG("EVENT NOT ACCEPTED")

        emit keyPressEventInternal(event);
    }
}


void
GUI_Scene::keyReleaseEvent(QKeyEvent* event)
{
    QQuickItem::keyReleaseEvent(event);

    if (!event->isAccepted()) {
        // RLP_LOG_DEBUG("EVENT NOT ACCEPTED")

        emit keyReleaseEventInternal(event);
    }
}


bool
GUI_Scene::event(QEvent* event)
{
    // RLP_LOG_DEBUG(event)

    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (((ke->key() == Qt::Key_Tab) || (ke->key() == Qt::Key_Backtab))
             && (_focusedPane != nullptr))
        {
            _focusedPane->onTabKeyPressed(ke);
        }
    }
    
    return QQuickItem::event(event);
}


void
GUI_Scene::setFocusedPane(GUI_FrameBase* pane)
{
    // RLP_LOG_DEBUG(pane)

    _focusedPane = pane;
}


void
GUI_Scene::addItem(GUI_ItemBase* item)
{
    // RLP_LOG_DEBUG(item)

    item->setScene(this);
    // item->setParent(this);
    item->setParentItem(this);

    connect(this, &GUI_Scene::sizeChanged, item, &GUI_ItemBase::onParentSizeChanged);
}


void
GUI_Scene::removeItem(GUI_ItemBase* item)
{
    // RLP_LOG_DEBUG(item)

    item->setParentItem(nullptr);
    item->setParent(nullptr);
}


void
GUI_Scene::addMenuPane(GUI_ItemBase* pane)
{
    _menuPanes.push_back(pane);
    addItem(pane);
}


void
GUI_Scene::removeMenuPane(GUI_ItemBase* pane)
{
    // RLP_LOG_DEBUG("")

    int pi = -1;
    for (int i=0; i != _menuPanes.size(); ++i)
    {
        if (_menuPanes.at(i) == pane)
        {
            pi = i;
            break;
        }
    }

    if (pi >= 0)
    {
        _menuPanes.removeAt(pi);
    }
}


void
GUI_Scene::hideMenuPanes(GUI_ItemBase* skipPane)
{
    // RLP_LOG_DEBUG("")

    for (int i=0; i != _menuPanes.size(); ++i) {

        if ((skipPane == nullptr) ||
            (_menuPanes.at(i) != skipPane))
        {
            _menuPanes.at(i)->setVisible(false);
        }
        
    }

    emit menuPanesHidden();
    emit updateRequested();
}


void
GUI_Scene::registerGLItem(GUI_GLItem* item)
{
    RLP_LOG_DEBUG("")

    _glItems.push_back(item);
}


void
GUI_Scene::initializeGL()
{
    RLP_LOG_DEBUG("")

    for (int i=0; i != _glItems.size(); ++i) {

        RLP_LOG_DEBUG(_glItems.at(i))
        
        _glItems.at(i)->initGL();
    }
}


void
GUI_Scene::showToolTip(QString text, int x, int y)
{
    // RLP_LOG_DEBUG(text << x << y)

    _toolTip->setText(text);
    _toolTip->setPos(x, y);

    _toolTip->setVisible(true);
}


void
GUI_Scene::hideToolTip()
{
    _toolTip->setVisible(false);
}


void
GUI_Scene::setDragItem(GUI_ItemBase* item)
{
    RLP_LOG_DEBUG(item)

    if (_dragItem != nullptr)
    {
        clearDragItem();
    }

    _dragItem = item;
}


void
GUI_Scene::clearDragItem()
{
    // RLP_LOG_DEBUG("")

    if (_dragItem != nullptr)
    {
        _dragItem->setParentItem(nullptr);
        delete _dragItem;
        _dragItem = nullptr;
    }
}