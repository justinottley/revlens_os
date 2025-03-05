

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/MENU/MenuPane.h"
#include "RlpGui/MENU/MenuItem.h"

#include "RlpGui/PANE/Pane.h"
#include "RlpGui/PANE/PaneBody.h"
#include "RlpGui/PANE/PaneHeader.h"

#include "RlpGui/PANE/View.h"

#include "RlpGui/BASE/ToolInfo.h"


RLP_SETUP_LOGGER(gui, GUI, PaneHeader)

GUI_PaneHeader::GUI_PaneHeader(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _height(25),
    _bgColor(QColor(60, 60, 60)),
    _xoffset(0),
    _xpan(0),
    _active(false)
{
    setHeight(25);
    setZValue(1); // apparently needed to draw menus over body?

    _tabHeaders.clear();
    initActButton();
}


void
GUI_PaneHeader::initActButton()
{
    GUI_Scene* gscene = scene(); // qobject_cast<GUI_Scene*>(scene());

    _actButton = new GUI_SvgButton(
        ":/feather/lightgrey/more-horizontal.svg",
        this,
        20,
        3
    );
    _actButton->icon()->setOutlined(true);
    _actButton->icon()->setOutlinedBgColour(GUI_Style::TabBg);
    _actButton->setZ(1);
    _actButton->setPos(pane()->width() - 30, 1);


    connect(
        _actButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_PaneHeader::onActionPressed
    );


    connect(
        _actButton->menu(),
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_PaneHeader::onActionItemSelected
    );


    _panLeftButton = new GUI_SvgButton(":/feather/lightgrey/arrow-left.svg",
        this,
        20,
        3
    );
    _panLeftButton->icon()->setOutlined(true);
    _panLeftButton->icon()->setOutlinedBgColour(GUI_Style::TabBg);
    _panLeftButton->icon()->setYOffset(1);
    _panLeftButton->setZ(1);
    _panLeftButton->setPos(0, 1);
    _panLeftButton->setVisible(false);

    connect(
        _panLeftButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_PaneHeader::onPanLeft
    );

    _panRightButton = new GUI_SvgButton(":/feather/lightgrey/arrow-right.svg",
        this,
        20,
        3
    );

    _panRightButton->icon()->setOutlined(true);
    _panRightButton->icon()->setOutlinedBgColour(GUI_Style::TabBg);
    _panRightButton->icon()->setYOffset(1);
    _panRightButton->setZ(1);
    _panRightButton->setPos(pane()->width() - 50, 1);
    _panRightButton->setVisible(false);

    connect(
        _panRightButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_PaneHeader::onPanRight
    );


    connect(
        qobject_cast<GUI_View*>(gscene->view()),
        &GUI_View::toolAdded,
        this,
        &GUI_PaneHeader::onToolAdded
    );


    GUI_MenuItem* actionsHeaderItem = _actButton->menu()->addItem("Pane");
    actionsHeaderItem->setSelectable(false);

    GUI_MenuItem* splitHItem = _actButton->menu()->addItem("Split ---");
    splitHItem->insertDataValue("item_type", "pane");
    splitHItem->insertDataValue("action", "split");
    splitHItem->insertDataValue("orientation", "V");

    GUI_MenuItem* splitVItem = _actButton->menu()->addItem("Split |");
    splitVItem->insertDataValue("item_type", "pane");
    splitVItem->insertDataValue("action", "split");
    splitVItem->insertDataValue("orientation", "H");

    GUI_MenuItem* mergeItem = _actButton->menu()->addItem("Merge");
    mergeItem->insertDataValue("item_type", "pane");
    mergeItem->insertDataValue("action", "merge");

    _actButton->menu()->addItem("");

    GUI_MenuItem* createHeaderItem = _actButton->menu()->addItem("Tools");
    createHeaderItem->setSelectable(false);


    initActButtonToolsPrivate();
}



void
GUI_PaneHeader::initActButtonToolsPrivate()
{
    // RLP_LOG_DEBUG("")

    GUI_Scene* gscene = scene();
    GUI_View* sview =  qobject_cast<GUI_View*>(gscene->view());
    
    QStringList toolNames = sview->toolNames();
    for (int ti=0; ti != toolNames.size(); ++ti)
    {

        GUI_ToolInfo* toolInfo = sview->tool(toolNames.at(ti));
        QVariantMap toolData = toolInfo->data();

        bool isSelectable = true;
        if (toolData.contains("selectable"))
        {
            isSelectable = toolData.value("selectable").toBool();
        }

        if (isSelectable)
        {
            GUI_MenuItem* toolItem = _actButton->menu()->addItem(toolNames.at(ti));
            toolItem->insertData(toolData);
            toolItem->insertDataValue("item_type", "tool");
        }
    }
}


GUI_TabHeader*
GUI_PaneHeader::addTabHeader(QString tabName, int idx)
{
    int currTabIndex = tabIndex(tabName);

    // NOTE: reusing tabs instead of destructing them.
    // Helps fix crashing on Windows..
    // 
    GUI_TabHeader* tabHeader = nullptr;

    if (currTabIndex != -1)
    {
        tabHeader = _tabHeaders.at(currTabIndex);
    } else
    {
        tabHeader = new GUI_TabHeader(tabName, this);
    }
    
    tabHeader->setPos(_xoffset, 0);
    tabHeader->setActive(true);
    tabHeader->setVisible(true);

    _xoffset += tabHeader->width();

    if (_tabHeaders.size() > 0)
    {
        _tabHeaders.at(_tabHeaders.size() - 1)->setActive(false);
    }
    
    if (idx == -1)
    {
        _tabHeaders.push_back(tabHeader);
    } else
    {
        RLP_LOG_DEBUG("Inserting tabheader at index" << tabHeader)
        _tabHeaders.insert(idx, tabHeader);
    }
    

    return tabHeader;
}


void
GUI_PaneHeader::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

    if (width > 0)
    {
        setWidth(width);
    }

    if ((width - 25) < _xoffset)
    {
        _panLeftButton->setVisible(true);
        _panRightButton->setVisible(true);

    } else
    {
        _panLeftButton->setVisible(false);
        _panRightButton->setVisible(false);
        _xpan = 0;

    }
}


void
GUI_PaneHeader::onToolAdded(QString toolName)
{
    // RLP_LOG_DEBUG(toolName)

    GUI_Scene* gscene = scene();
    GUI_View* gview = qobject_cast<GUI_View*>(gscene->view());

    GUI_ToolInfo* toolInfo = gview->tool(toolName);
    QVariantMap toolData = toolInfo->data();

    bool isSelectable = true;
    if (toolData.contains("selectable"))
    {
        isSelectable = toolData.value("selectable").toBool();
    }


    if (isSelectable)
    {
        GUI_MenuItem* menuItem = _actButton->menu()->addItem(toolName);
        menuItem->insertData(toolInfo->data());
        menuItem->insertDataValue("item_type", "tool");
    }
}


void
GUI_PaneHeader::onActionPressed(QVariantMap /* metadata */)
{
    RLP_LOG_DEBUG("")

    // _actButton->onSceneLayoutChanged(); // to ensure the menu is visible
    _actButton->toggleMenu();
    
    GUI_Scene* gscene = qobject_cast<GUI_Scene*>(scene());
    gscene->requestPaneMenuToggled(!_actButton->menu()->isVisible());

    // gscene->view()->toolsMenu()->setPos(QCursor::pos()); // QCursor::pos());
    // gscene->view()->toolsMenu()->setVisible(true); // setPos(position().x(), position().y() + 20);
}


QQuickItem*
GUI_PaneHeader::onActionItemSelected(QVariantMap sigData)
{
    QQuickItem* result = nullptr;

    QString itemName = sigData.value("text").toString();

    int existingTabIdx = pane()->getTabIndex(itemName);
    if (existingTabIdx != -1)
    {
        // RLP_LOG_DEBUG("Found existing tab:" << itemName << "at" << existingTabIdx)
        
        pane()->setActiveTab(existingTabIdx);
        updateTabHeaders();
        return result;
    }

    QVariantMap itemData = sigData.value("data").toMap();

    // RLP_LOG_INFO(itemName << " " << itemData)

    QString itemType = itemData.value("item_type").toString();
    if (itemType == "tool")
    {

        GUI_TabPtr nt = pane()->addTab(itemName);

        GUI_Scene* gscene = scene(); // qobject_cast<GUI_Scene*>(scene());
        GUI_View* gview = qobject_cast<GUI_View*>(gscene->view());

        GUI_ToolInfo* toolInfo = gview->tool(itemName);

        nt->pane()->setMetadata("name", itemName);

        // WidgetCreateFunc toolCreateFunc = toolInfo.value("creator").value<WidgetCreateFunc>();
        // toolCreateFunc(nt->pane(), itemData);

        QQuickItem* item = toolInfo->func()(nt->pane(), itemData);

        if (item->inherits("GUI_ItemBase"))
        {
            GUI_ItemBase* iitem = qobject_cast<GUI_ItemBase*>(item);
            iitem->setMetadata("tab.name", itemName);

            connect(
                gscene,
                &GUI_Scene::tabVisibilityChanged,
                iitem,
                &GUI_ItemBase::onParentVisibilityChanged
            );

            iitem->setPaintBackground(iitem->paintBackground());

        }

        return item;


    } else if (itemType == "pane")
    {

        QString paneAction = itemData.value("action").toString();

        if (paneAction == "split")
        {

            QString orientation = itemData.value("orientation").toString();

            if (orientation == "V")
            {

                pane()->setOrientation(O_VERTICAL);
                if (pane()->split(true))
                {
                    pane()->splitter(0)->setPosPercent(0.5);
                }
                
            } else if (orientation == "H")
            {

                pane()->setOrientation(O_HORIZONTAL);
                if (pane()->split(true))
                {
                    pane()->splitter(0)->setPosPercent(0.5);
                }
                

            } else
            {
                RLP_LOG_ERROR("Invalid orientation for split: " << orientation)
            }
        } else if (paneAction == "merge")
        {
            
            RLP_LOG_INFO(parentItem()->parentItem())
            QString pClassName = parentItem()->parentItem()->metaObject()->className();

            if (parentItem()->parentItem() == nullptr)
            {
                RLP_LOG_ERROR("Invalid parent, cannot merge")
                return result;
            }

            
            if (pClassName != "GUI_PaneBody")
            {
                RLP_LOG_ERROR("parent item is not a GUI_PaneBody, cannot merge");
                return result;
            }

            GUI_PaneBody* pbody = qobject_cast<GUI_PaneBody*>(parentItem()->parentItem());

            pbody->pane()->removePane(pane());


        }
    }
    
    return result;

}


void
GUI_PaneHeader::moveTabHeader(QString tabName, QString moveBeforeName)
{
    RLP_LOG_DEBUG("Move" << moveBeforeName << "before" << tabName)

    int currIdx = tabIndex(tabName);
    int moveIdx = tabIndex(moveBeforeName);

    RLP_LOG_DEBUG("Move tab at" << moveIdx << "to" << currIdx);

    if (moveIdx != -1)
    {
        GUI_TabHeader* th = _tabHeaders.takeAt(moveIdx);
        _tabHeaders.insert(currIdx, th);
    }

    updateTabHeaders();
}


void
GUI_PaneHeader::onPanLeft(QVariantMap md)
{
    RLP_LOG_DEBUG("")

    _xpan += 50;
    updateTabHeaders();
}


void
GUI_PaneHeader::onPanRight(QVariantMap md)
{
    RLP_LOG_DEBUG("")

    _xpan -= 50;
    updateTabHeaders();
}


int
GUI_PaneHeader::tabIndex(QString tabName)
{
    RLP_LOG_DEBUG(tabName)

    for (int i=0; i != _tabHeaders.size(); ++i)
    {
        if (_tabHeaders.at(i)->name() == tabName)
        {
            return i;
        }
    }
    return -1;
}


GUI_Pane*
GUI_PaneHeader::pane() const
{
    // RLP_LOG_DEBUG(parentItem())

    return dynamic_cast<GUI_Pane*>(parentItem());
}



void
GUI_PaneHeader::setBgColor(QColor col)
{
    _bgColor = col;
}


void
GUI_PaneHeader::removeAt(int i)
{
    RLP_LOG_DEBUG(_tabHeaders.size() << i)

    if (_tabHeaders.size() > i)
    {
        _tabHeaders.removeAt(i);
        RLP_LOG_DEBUG("OK")
    }
}


void
GUI_PaneHeader::updateTabHeaders()
{
    // RLP_LOG_DEBUG(_tabHeaders.size())
    // RLP_LOG_DEBUG(pane())
    // RLP_LOG_DEBUG(pane()->width())

    _xoffset = _xpan;
    if (_panLeftButton->isVisible())
        _xoffset += 25;

    for (int ti=0; ti != _tabHeaders.size(); ++ti)
    {
        _tabHeaders.at(ti)->setFirst(false);

        if (_tabHeaders.at(ti)->isVisible())
        {
            _tabHeaders.at(ti)->setPos(_xoffset, 0);
            _xoffset += _tabHeaders.at(ti)->width();    
        }
        
    }

    for (int ti=0; ti != _tabHeaders.size(); ++ti)
    {
        if (_tabHeaders.at(ti)->isVisible())
        {
            _tabHeaders.at(ti)->setFirst(true);
            break;
        }
    }

    int xoff = 23;
    if (pane()->isLast() && !pane()->isSingle())
    {
        xoff = 30;
    }


    _actButton->setPos(pane()->width() - xoff, 1);
    _panRightButton->setPos(pane()->width() - xoff - xoff, 1);

    // RLP_LOG_DEBUG("xoffset:" << _xoffset)
    update();
}


QRectF
GUI_PaneHeader::boundingRect() const
{
    // RLP_LOG_DEBUG(pane()->width() << _height);

    return QRectF(0, 0, pane()->width(), _height);
}



void
GUI_PaneHeader::paint(GUI_Painter* painter)
{
    QPen p(Qt::black);
    p.setWidth(2);
    painter->setPen(p); // Qt::black);
    // painter->setPen(GUI_Style::BgDarkGrey);
    painter->setBrush(GUI_Style::TabBg);

    
    painter->drawRect(boundingRect());

    /*
    if ((pane()->tabCount() == 0) &&
        (pane()->name().length() > 0)) {
        
        painter->drawText(
            10,
            15,
            pane()->name()
        );
        
    }
    */

    // GUI_ItemBase::paint(painter);
    // _actButton->paint(painter);

}