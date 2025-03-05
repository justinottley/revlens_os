
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/BASE/Style.h"
// #include "RlpGui/LAYOUT/View.h"

#include "RlpGui/MENU/MenuPane.h"

RLP_SETUP_LOGGER(gui, GUI, MenuPane)

GUI_MenuPane::GUI_MenuPane(GUI_ItemBase* parent):
    GUI_MenuPaneBase(parent),
    _widthPolicy(AutoWidth),
    _autoWidth(100),
    _itemHeight(10),
    _exclusiveSelect(false)
{
    _focusedItems.clear();

    setWidth(120);
    setZValue(1);
    setVisible(false);

    if (_scene == nullptr)
    {
        RLP_LOG_ERROR("invalid scene!")
    }
    _scene->addMenuPane(this);
}


GUI_MenuPane::~GUI_MenuPane()
{
    // RLP_LOG_DEBUG("")
    _scene->removeMenuPane(this);
}


void
GUI_MenuPane::setPos(qreal x, qreal y)
{
    // RLP_LOG_DEBUG(x << " " << y << " scenePos: " << scenePos())
    // Can only show the pane within the limits of the window.
    // Move pane inside window if it is too far
    //

    qreal pwidth = parentItem()->width();
    qreal pheight = parentItem()->height();
    // (x << " " << y << "width:" << width() << "parent width:" << pwidth)

    
    if (x + width() > pwidth)
    { // sceneSize.width()) {
        
        // RLP_LOG_DEBUG("PANE OCCU, parent:" << parentItem()->metaObject()->className())
        
        // RLP_LOG_DEBUG("moving pane to inside window: " << x)
        x = (pwidth - width() - 5);
    }

    if (y + height() > pheight)
    {
        y = (pheight - height() - 10);
    }

    // GUI_ItemBase::setPos(-500, y); // 100, y);
    // GUI_ItemBase::setPos(x, y); // 100, y);
    GUI_ItemBase::setPos(x, y);
}


void
GUI_MenuPane::setFilterString(QString filterStr)
{
    // RLP_LOG_DEBUG(filterStr)

    _filterStr = filterStr;

    _itemHeight = 0;
    int visItems = 0;
    for (int i=0; i != _items.size(); ++i)
    {
        GUI_MenuItem* item = qobject_cast<GUI_MenuItem*>(_items.at(i));
        if (filterStr == "")
        {
            item->setPos(0, 20 * visItems);
            item->setVisible(true);

            _itemHeight += item->height();
            visItems++;
        }
        else if (!item->text().contains(filterStr))
        {
            item->setVisible(false);
        } else
        {
            item->setPos(0, 20 * visItems);
            item->setVisible(true);

            _itemHeight += item->height();
            visItems++;
        }
    }

    setHeight(_itemHeight + 10);
}


void
GUI_MenuPane::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

}


void
GUI_MenuPane::setPos(QPointF pos)
{
    setPos(pos.x(), pos.y());
}


GUI_MenuItem*
GUI_MenuPane::addItem(QString entry, QVariantMap data, bool checkable)
{
    // RLP_LOG_DEBUG(entry << data << checkable)

    GUI_MenuItem* item = new GUI_MenuItem(entry, this);
    item->setCheckable(checkable);
    item->setData(data);
    item->setPos(0, _itemHeight); // 20 * (childItems().size() - 1));
    
    connect(item, &GUI_MenuItem::menuItemSelected, this, &GUI_MenuPane::onMenuItemSelected);
    connect(item, &GUI_MenuItem::menuShown, this, &GUI_MenuPane::onMenuItemMenuShown);
    connect(item, &GUI_MenuItem::menuItemHoverEntered, this, &GUI_MenuPane::onMenuItemHoverEntered);
    connect(item, &GUI_MenuItem::menuItemHoverExited, this, &GUI_MenuPane::onMenuItemHoverExited);
    connect(this, &GUI_MenuPane::menuShown, item, &GUI_MenuItem::onSiblingMenuShown);

    QFontMetrics fm(QApplication::font());
    int textWidth = fm.horizontalAdvance(entry);

    if (textWidth > _autoWidth) {
        _autoWidth = textWidth;
    }

    _itemHeight += item->height();

    if (_widthPolicy == AutoWidth) {
        setWidth(_autoWidth + 30);

    } else {
        setWidth(_fixedWidth + 30);
    }

    setHeight(_itemHeight + 10);

    _items.append(item);

    return item;
}


GUI_MenuItem*
GUI_MenuPane::item(QString name)
{
    for (int i=0; i != _items.size(); ++i)
    {
        GUI_MenuItem* itm = qobject_cast<GUI_MenuItem*>(_items.at(i));
        if (itm->text() == name)
        {
            return itm;
        }
    }

    return nullptr;
}


void
GUI_MenuPane::addItem(GUI_ItemBase* item)
{
    item->setParentItem(this);
    // item->setPos(0, 20 * (childItems().size() - 1));
    item->setPos(0, _itemHeight); // 20 * (childItems().size() - 1));

    _itemHeight += item->height();

    setHeight(_itemHeight + 10);

    _items.append(item);
}


void
GUI_MenuPane::addSeparator()
{
    GUI_MenuItem* item = new GUI_MenuItem("", this);
    item->setHeight(6);
    item->setSelectable(false);

    addItem(item);
}


void
GUI_MenuPane::clear()
{
    // RLP_LOG_VERBOSE("")

    QList<QQuickItem*> items = childItems();

    for (int i=0; i != items.size(); ++i) {
        delete items.at(i);
    }

    setHeight(0);
    _itemHeight = 10;
    _items.clear();
}


void
GUI_MenuPane::onMenuItemSelected(QVariantMap menuData)
{
    // RLP_LOG_DEBUG(menuData)

    if (_exclusiveSelect)
    {
        GUI_MenuItem* item = menuData.value("item").value<GUI_MenuItem*>();
        for (int i=0; i != _items.size(); ++i)
        {
            // RLP_LOG_DEBUG("TURNING OFF CHCEK ON" << i)
            GUI_MenuItem* mi = qobject_cast<GUI_MenuItem*>(_items.at(i));
            if (mi && mi->isCheckable())
            {
                mi->setChecked(false);
            }
        }

        if (item->isCheckable())
        {
            item->setChecked(true);
        }
    }

    emit menuItemSelected(menuData);

    qobject_cast<GUI_Scene*>(scene())->hideMenuPanes(nullptr);    
}


void
GUI_MenuPane::onMenuItemMenuShown(QString name)
{
    // RLP_LOG_DEBUG("")

    emit menuShown(name);
}



void
GUI_MenuPane::onMenuItemHoverEntered(QString name)
{
    // RLP_LOG_DEBUG(name)

    _focusedItems.insert(name);

    qobject_cast<GUI_Scene*>(scene())->requestUpdate();
}



void
GUI_MenuPane::onMenuItemHoverExited(QString name)
{
    // RLP_LOG_DEBUG(name)

    _focusedItems.remove(name);

    qobject_cast<GUI_Scene*>(scene())->requestUpdate();
}


/*
int
GUI_MenuPane::width() const
{
    if (_widthPolicy == AutoWidth) {
        return _autoWidth;
    }

    return _fixedWidth;
}
*/

QRectF
GUI_MenuPane::boundingRect() const
{
    // RLP_LOG_DEBUG(width() << height())

    return QRectF(0, 0, width() - 1, height() - 1); // width(), height() + 15);
}


void
GUI_MenuPane::paint(GUI_Painter* painter)
{
    painter->setPen(Qt::black);
    painter->setBrush(QBrush(GUI_Style::BgLightGrey));
    painter->drawRect(boundingRect());
}
