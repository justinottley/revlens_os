

#include "RlpGui/MENU/MenuItem.h"

#include "RlpGui/MENU/MenuPane.h"

#include "RlpCore/UTIL/Text.h"

#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"


RLP_SETUP_LOGGER(gui, GUI, MenuItem)

GUI_MenuItem::GUI_MenuItem(QString text, GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _text(text),
    _focused(false),
    _pressed(false),
    _selectable(true),
    _checkable(false),
    _checked(false),
    _isMenu(false)
{
    setHeight(20);
    setAcceptHoverEvents(true);
    // setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    int iconSize = height();

    _checkIcon = new GUI_SvgIcon(
        ":/feather/lightgrey/check.svg",
        this,
        iconSize
    );
    _checkIcon->setPos(0, 0);
    _checkIcon->setBgColour(QColor(200, 200, 200));
    _checkIcon->setVisible(false);

    _arrowIcon = new GUI_SvgIcon(
        ":/feather/lightgrey/chevron-right.svg",
        this,
        iconSize
    );

    _arrowIcon->setBgColour(QColor(200, 200, 200));
    _arrowIcon->setVisible(false);

    _menu = new GUI_MenuPane(this);
    _menu->setPos(position().x(), position().y() + height());
    _menu->setVisible(false);
}


void
GUI_MenuItem::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    if (nwidth > 0)
    {
        setWidth(nwidth);
    }

    _arrowIcon->setPos(width() - 20, 0);
}


void
GUI_MenuItem::mousePressEvent(QMouseEvent* event)
{
    if (_isMenu) {
        return;
    }

    if ((_selectable) && (_text != "")) {
        _pressed = true;
    }

}


void
GUI_MenuItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (_isMenu) {
        return;
    }
    
    if ((_selectable) && (_text != ""))
    {
        selectItem();
    }
    
}


void
GUI_MenuItem::selectItem()
{
    RLP_LOG_DEBUG("")

    _pressed = false;
    _focused = false;

    if (_checkable)
    {
        setChecked(!_checked);
    }

    QVariantMap sigData;
    QVariant v;
    v.setValue(this);

    sigData.insert("text", _text);
    sigData.insert("data", _data);
    sigData.insert("item", v);

    emit triggered();
    emit menuItemSelected(sigData);
}


void
GUI_MenuItem::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(_text)

    if ((_selectable) && (_text != "")) {
        _focused = true;
    }
    
    if (_isMenu) {

        // RLP_LOG_DEBUG(_text << "SHOWING MENU")

        QPointF spos = mapToScene(position());
        qreal nypos = parentItem()->position().y() + position().y();

        // RLP_LOG_DEBUG(position() << spos << nypos)

        _menu->setPos(spos.x() + width(), nypos);
        _menu->setVisible(true);

        event->setAccepted(true);

        emit menuShown(_text);
    }

    emit menuItemHoverEntered(_text);

    updateItem();
}


void
GUI_MenuItem::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(_text)

    emit menuItemHoverExited(_text);

    if ((_selectable) && (_text != "")) {
        _focused = false;

        QTimer::singleShot(500, this, &GUI_MenuItem::hideMenu);
    }

    updateItem();
}


void
GUI_MenuItem::hideMenu()
{
    // RLP_LOG_DEBUG(_text)

    if ((!_focused) && (_isMenu) && (_menu->isVisible()) && (!_menu->isFocused()))
    {
        // RLP_LOG_DEBUG("hiding menu " << _text << _menu->isFocused())

        _menu->setVisible(false);
    }


}


QRectF
GUI_MenuItem::boundingRect() const
{
    return QRectF(1, 1, parentItem()->boundingRect().width() - 2, 20); // height());
}


void
GUI_MenuItem::setText(QString text)
{
    _text = text;

    emit menuItemTextChanged(text);
}

void
GUI_MenuItem::setCheckable(bool isCheckable)
{
    _checkable = isCheckable;
}


void
GUI_MenuItem::setChecked(bool isChecked)
{
    if (!_checkable)
    {
        RLP_LOG_WARN("Item not chcheckable")
        return;
    }

    _checked = isChecked;
    if (_checked)
    {
        _checkIcon->setVisible(true);
    }
    else
    {
        _checkIcon->setVisible(false);
    }
}


GUI_MenuItem*
GUI_MenuItem::addItem(QString itemName, QVariantMap itemData)
{
    _isMenu = true;

    GUI_MenuItem* item = _menu->addItem(itemName, itemData);
    
    _arrowIcon->setVisible(true);


    return item;
}


void
GUI_MenuItem::onSiblingMenuShown(QString name)
{
    // RLP_LOG_DEBUG(name)

    if (name != _text) {
        _menu->setVisible(false);
    }
}


void
GUI_MenuItem::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(_text << _focused << height())

    painter->setPen(GUI_Style::BgDarkGrey);
    painter->setBrush(GUI_Style::BgDarkGrey);

    if (_pressed) {

        painter->setBrush(Qt::red);
        painter->setPen(Qt::red);
        painter->drawRect(boundingRect());

    } else if (_focused) {
        painter->setPen(Qt::gray);
        painter->setBrush(Qt::gray);

        painter->drawRect(boundingRect());

    } else if (!_selectable) {

        painter->setBrush(GUI_Style::BgMidGrey);
        painter->setPen(GUI_Style::BgMidGrey);

        painter->drawRect(boundingRect());

    } 

    int iconSize = height();
    int xoffset = 10 + iconSize;

    if (_selectable) {
        painter->setPen(Qt::white);

    } else {
        xoffset = 15;
        painter->setPen(GUI_Style::FgGrey);
    }
    
    // painter->drawText(xoffset, 15, _text);
    painter->drawText(20, 15, _text);


}