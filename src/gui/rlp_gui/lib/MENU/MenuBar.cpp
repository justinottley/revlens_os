
#include "RlpGui/BASE/Style.h"
#include "RlpGui/MENU/MenuBar.h"

RLP_SETUP_LOGGER(gui, GUI, MenuBar)

GUI_MenuBar::GUI_MenuBar(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _menuVisible(false),
    _visCount(0)
{
    _menus.clear();

    setHeight(30);
    setWidth(400);
    _layout = new GUI_HLayout(this);
}


GUI_MenuButton*
GUI_MenuBar::addMenu(QString menuName)
{
    GUI_MenuButton* menu = new GUI_MenuButton(_layout);

    connect(menu, &GUI_MenuButton::menuShown, this, &GUI_MenuBar::onMenuShown);
    connect(menu, &GUI_MenuButton::menuHidden, this, &GUI_MenuBar::onMenuHidden);

    menu->setText(menuName);

    _layout->addItem(menu, 3);
    _menus.insert(menuName, menu);

    setWidth(_layout->width());

    // RLP_LOG_DEBUG(menuName << width() << height() << position())

    return menu;
}


GUI_MenuButton*
GUI_MenuBar::addRightMenu(QString menuName)
{
    RLP_LOG_DEBUG(menuName)

    GUI_MenuButton* menu = new GUI_MenuButton(this);

    connect(menu, &GUI_MenuButton::menuShown, this, &GUI_MenuBar::onMenuShown);
    connect(menu, &GUI_MenuButton::menuHidden, this, &GUI_MenuBar::onMenuHidden);

    menu->setText(menuName);

    _menus.insert(menuName, menu);
    _rightItems.append(menu);

    onParentSizeChanged(width(), height());

    return menu;
}


void
GUI_MenuBar::addRightItem(GUI_ItemBase* item)
{
    RLP_LOG_DEBUG(item)

    item->setParentItem(this);
    _rightItems.append(item);

    onParentSizeChanged(width(), height());
}


void
GUI_MenuBar::addMenuButton(GUI_ButtonBase* button)
{
    RLP_LOG_DEBUG(button)

    connect(
        button,
        &GUI_ButtonBase::menuShown,
        this,
        &GUI_MenuBar::onMenuButtonShown
    );

    connect(
        button,
        &GUI_ButtonBase::menuHidden,
        this,
        &GUI_MenuBar::onMenuButtonHidden
    );

    _layout->addItem(qobject_cast<GUI_ItemBase*>(button));
}


void
GUI_MenuBar::setMenuVisibility(bool isVisible)
{
    MenuIterator it;

    for (it=_menus.begin(); it != _menus.end(); ++it)
    {
        it.value()->setShowMenuOnHover(isVisible);
    }

    QList<GUI_ItemBase*> litems = _layout->items();
    for (int i =0;  i != litems.size(); ++i)
    {
        QString itype = litems.at(i)->metaObject()->className();
        if (itype == "GUI_IconButton")
        {
            qobject_cast<GUI_ButtonBase*>(litems.at(i))->setShowMenuOnHover(isVisible);
        }
    }

}


void
GUI_MenuBar::onMenuShown(QString menuName)
{
    // RLP_LOG_DEBUG(menuName)

    _menuVisible = !_menuVisible;

    setMenuVisibility(true);

    // Hide other menus
    MenuIterator it;
    for (it=_menus.begin(); it != _menus.end(); ++it)
    {
        if (it.value()->text() != menuName)
        {
            if (it.value()->menu()->isVisible())
            {
                _visCount--;
            }
            it.value()->menu()->setVisible(false);
        }
    }

    QList<GUI_ItemBase*> litems = _layout->items();
    for (int i =0;  i != litems.size(); ++i)
    {
        QString itype = litems.at(i)->metaObject()->className();
        if (itype == "GUI_IconButton")
        {
            GUI_MenuPane* mp = qobject_cast<GUI_MenuPane*>(
                qobject_cast<GUI_ButtonBase*>(litems.at(i))->menu()
            );

            if (mp->isVisible())
            {
                _visCount--;
            }

            mp->setVisible(false);
        }
    }


    _visCount++;

    emit menuShown(menuName, _visCount);
}


void
GUI_MenuBar::onMenuHidden(QString menuName)
{
    // RLP_LOG_DEBUG("")

    setMenuVisibility(false);

    _visCount--;

    emit menuHidden(menuName, _visCount);
}


void
GUI_MenuBar::onMenuPanesHidden()
{
    // RLP_LOG_DEBUG("")

    setMenuVisibility(false);
    _visCount = 0;
}


void
GUI_MenuBar::onMenuButtonShown(QVariantMap md)
{
    RLP_LOG_DEBUG(md)

    // Hide other menus
    MenuIterator it;
    for (it=_menus.begin(); it != _menus.end(); ++it)
    {
        it.value()->menu()->setVisible(false);
    }

    setMenuVisibility(true);
    _visCount++;
}


void
GUI_MenuBar::onMenuButtonHidden(QVariantMap md)
{
    RLP_LOG_DEBUG(md)

    // Hide other menus
    MenuIterator it;
    for (it=_menus.begin(); it != _menus.end(); ++it)
    {
        it.value()->menu()->setVisible(false);
    }

    setMenuVisibility(false);
    _visCount--;
}



void
GUI_MenuBar::clear()
{
    RLP_LOG_DEBUG("")

    _layout->clear();
    _menus.clear();
    _rightItems.clear();
}




QRectF
GUI_MenuBar::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_MenuBar::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

    qreal xpos = 0;
    for (int i=0; i != _rightItems.size(); ++i)
    {
        GUI_ItemBase* ri = _rightItems.at(i);

        xpos += ri->width();
        ri->setPos(width - 20 - xpos, 3);
    }

    setWidth(width);
}


void
GUI_MenuBar::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(boundingRect());

    // Nothing
    // painter->setBrush(Qt::red);
    // painter->drawRect(boundingRect());
}