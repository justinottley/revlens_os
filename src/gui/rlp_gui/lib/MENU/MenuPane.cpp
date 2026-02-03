
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/BASE/Style.h"
// #include "RlpGui/LAYOUT/View.h"

#include "RlpGui/MENU/MenuPane.h"

RLP_SETUP_LOGGER(gui, GUI, MenuPaneScrollButton)
RLP_SETUP_LOGGER(gui, GUI, MenuPane)

const int GUI_MenuPaneScrollButton::HEIGHT = 15;
const int GUI_MenuPaneScrollButton::INC = 15;

GUI_MenuPaneScrollButton::GUI_MenuPaneScrollButton(
    GUI_MenuPane* parent,
    ScrollDirection direction
):
    GUI_ItemBase(parent),
    _direction(direction),
    _bgCol(GUI_Style::BgGrey)
{
    setAcceptHoverEvents(true);
    setWidth(parent->width());
    setHeight(GUI_MenuPaneScrollButton::HEIGHT);

    QString iconStr = ":feather/chevron-";
    if (_direction == GUI_MenuPaneScrollButton::MENU_SCROLL_DOWN)
    {
        iconStr += "up.svg";
    } else
    {
        iconStr += "down.svg";
    }

    _icon = new GUI_SvgIcon(iconStr, this, 18, 0, Qt::white);
    _icon->setPos(width() / 2, 0);
}


void
GUI_MenuPaneScrollButton::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")
    emit buttonPressed((int)_direction);
}


void
GUI_MenuPaneScrollButton::hoverEnterEvent(QHoverEvent* event)
{
    _bgCol = GUI_Style::IconBg;
    update();
}


void
GUI_MenuPaneScrollButton::hoverLeaveEvent(QHoverEvent* event)
{
    _bgCol = GUI_Style::BgGrey;
    update();
}


void
GUI_MenuPaneScrollButton::onParentSizeChanged(qreal width, qreal height)
{
    setWidth(width);
    setHeight(GUI_MenuPaneScrollButton::HEIGHT);

    _icon->setPos((width / 2) - 10, 0);
}



QRectF
GUI_MenuPaneScrollButton::boundingRect() const
{
    return QRectF(1, 1, width() - 2, height() - 2);
}


void
GUI_MenuPaneScrollButton::paint(GUI_Painter* painter)
{
    painter->setPen(_bgCol);
    painter->drawRect(boundingRect());

}


GUI_MenuPane::GUI_MenuPane(GUI_ItemBase* parent):
    GUI_MenuPaneBase(parent),
    _bgCol(GUI_Style::BgLightGrey),
    _scrollButtonsVisible(false),
    _widthPolicy(AutoWidth),
    _autoWidth(100),
    _itemHeight(0),
    _exclusiveSelect(false)
{
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    _itemWrapper = new GUI_ScrollArea(this);
    _itemWrapper->setOutlined(false);
    _itemWrapper->setHScrollBarVisible(false);
    _itemWrapper->setBgColour(GUI_Style::BgLightGrey);
    _itemWrapper->vhandle()->setHoverBgColour(Qt::gray);
    _itemWrapper->vhandle()->setBgColour(GUI_Style::BgGrey);

    _itemWrapper->setPos(0, scrollHOffset());

    _scrollTop = new GUI_MenuPaneScrollButton(
        this, GUI_MenuPaneScrollButton::MENU_SCROLL_DOWN);
    _scrollTop->setVisible(false);

    _scrollBottom = new GUI_MenuPaneScrollButton(
        this, GUI_MenuPaneScrollButton::MENU_SCROLL_UP);
    _scrollBottom->setVisible(false);

    connect(
        _scrollTop,
        &GUI_MenuPaneScrollButton::buttonPressed,
        this,
        &GUI_MenuPane::onScrollPressed
    );

    connect(
        _scrollBottom,
        &GUI_MenuPaneScrollButton::buttonPressed,
        this,
        &GUI_MenuPane::onScrollPressed
    );

    _items.clear();
    _focusedItems.clear();

    setWidth(120);
    setZValue(1);
    setVisible(false);

    if (_scene == nullptr)
    {
        RLP_LOG_ERROR("invalid scene!")
    }
    _scene->addMenuPane(this);

    onParentSizeChanged(parent->width(), parent->height());
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

    if (x + width() > pwidth)
    {
        x = (pwidth - width() - 5);
    }

    if (y + height() > pheight)
    {
        y = (pheight - height() - 10);
    }


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

    setHeight(_itemHeight + scrollHOffset() + 20);
    _itemWrapper->setHeight(height() - (scrollHOffset() * 2) + 10);
}


void
GUI_MenuPane::setPos(QPointF pos)
{
    setPos(pos.x(), pos.y());
}


void
GUI_MenuPane::updateDimensions()
{
    int widthPadding = 40;

    if (_widthPolicy == AutoWidth)
    {
        setWidth(_autoWidth + widthPadding);
        _itemWrapper->setWidth(_autoWidth + widthPadding);

    } else
    {
        setWidth(_fixedWidth + widthPadding);
        _itemWrapper->setWidth(_fixedWidth + widthPadding);
    }

    setHeight(_itemHeight + scrollHOffset() + 20);
    _itemWrapper->setHeight(height() - (scrollHOffset() * 2) + 10);
    _itemWrapper->setMaxChildHeight(_itemHeight);
}


void
GUI_MenuPane::updateScrolling()
{
    int vheight = scene()->view()->height();
    bool sshow = ((_itemHeight != 0) && (_itemHeight + 60 > vheight));

    _scrollButtonsVisible = sshow;
    _scrollTop->setVisible(sshow);
    _scrollBottom->setVisible(sshow);

    if (sshow)
    {
        setHeight(vheight - 60);
    } else
    {
        setHeight(_itemHeight + scrollHOffset() + 10);
        resetItemPositions();
    }

    _itemWrapper->setPos(0, scrollHOffset());
    _itemWrapper->setWidth(width());

    _scrollBottom->setPos(0, height() - scrollHOffset());
}


GUI_MenuItem*
GUI_MenuPane::addItem(QString entry, QVariantMap data, bool checkable)
{
    // RLP_LOG_DEBUG(entry << data << checkable)

    GUI_MenuItem* item = new GUI_MenuItem(entry, _itemWrapper->content());
    item->setCheckable(checkable);
    item->setData(data);
    item->setPos(0, _itemHeight);
    
    connect(item, &GUI_MenuItem::menuItemSelected, this, &GUI_MenuPane::onMenuItemSelected);
    connect(item, &GUI_MenuItem::menuShown, this, &GUI_MenuPane::onMenuItemMenuShown);
    connect(item, &GUI_MenuItem::menuItemHoverEntered, this, &GUI_MenuPane::onMenuItemHoverEntered);
    connect(item, &GUI_MenuItem::menuItemHoverExited, this, &GUI_MenuPane::onMenuItemHoverExited);
    connect(this, &GUI_MenuPane::menuShown, item, &GUI_MenuItem::onSiblingMenuShown);

    QFontMetrics fm(QApplication::font());
    int textWidth = fm.horizontalAdvance(entry);

    if (textWidth > _autoWidth)
    {
        _autoWidth = textWidth;
    }

    _itemHeight += item->height();
    _items.append(item);

    updateDimensions();

    return item;
}


void
GUI_MenuPane::addItem(GUI_ItemBase* item)
{
    item->setParentItem(_itemWrapper->content());
    item->setPos(0, _itemHeight);

    _itemHeight += item->height();
    _items.append(item);

    if (item->width() > _autoWidth)
    {
        _autoWidth = item->width();
    }

    updateDimensions();
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
GUI_MenuPane::addSeparator()
{
    GUI_MenuItem* item = new GUI_MenuItem("", _itemWrapper);
    item->setHeight(6);
    item->setSelectable(false);

    addItem(item);
}


void
GUI_MenuPane::clear()
{
    // RLP_LOG_VERBOSE("")

    for (int i = 0; i != _items.size(); ++i)
    {
        _items.at(i)->setVisible(false);
    }

    setHeight(0);

    _itemHeight = 0;
    _items.clear();

    updateDimensions();
}


void
GUI_MenuPane::resetItemPositions()
{
    _itemHeight = 0;
    for (int i = 0; i != _items.size(); ++i)
    {
        GUI_ItemBase* item = qobject_cast<GUI_ItemBase*>(_items.at(i));
        item->setPos(0, _itemHeight);
        _itemHeight += item->height();
    }
}


int
GUI_MenuPane::scrollHOffset()
{
    if (_scrollButtonsVisible)
    {
        return GUI_MenuPaneScrollButton::HEIGHT;
    }

    return 10;
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

    update();
}



void
GUI_MenuPane::onMenuItemHoverExited(QString name)
{
    // RLP_LOG_DEBUG(name)

    _focusedItems.remove(name);

    update();
}


void
GUI_MenuPane::onScrollPressed(int direction)
{
    int inc = 0;
    if ((GUI_MenuPaneScrollButton::ScrollDirection)direction == GUI_MenuPaneScrollButton::MENU_SCROLL_DOWN)
    {
        inc = GUI_MenuPaneScrollButton::INC;
    }
    else if ((GUI_MenuPaneScrollButton::ScrollDirection)direction == GUI_MenuPaneScrollButton::MENU_SCROLL_UP)
    {
        inc = -GUI_MenuPaneScrollButton::INC;
    }
    else
    {
        RLP_LOG_ERROR("Invalid scroll direction:" << direction)
    }

    for (int i=0; i != _items.size(); ++i)
    {
        GUI_MenuItem* item = qobject_cast<GUI_MenuItem*>(_items.at(i));
        item->setPos(0, item->y() + inc);
    }
}


void
GUI_MenuPane::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    updateScrolling();
}


void
GUI_MenuPane::paint(GUI_Painter* painter)
{
    painter->setPen(Qt::black);
    painter->setBrush(QBrush(_bgCol));
    painter->drawRect(boundingRect());
}
