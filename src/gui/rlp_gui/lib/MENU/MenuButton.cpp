

#include "RlpGui/MENU/MenuButton.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(gui, GUI, MenuButton)

GUI_MenuButton::GUI_MenuButton(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _showMenuOnHover(false),
    _focused(false),
    _bgCol(GUI_Style::BgDarkGrey)
{
    setSize(QSizeF(100, 20));
    setAcceptHoverEvents(true);

    // setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    _menu = new GUI_MenuPane(this);
    _menu->setPos(position().x(), position().y() + height());
    _menu->setVisible(false);

    connect(_menu, &GUI_MenuPane::menuItemSelected, this, &GUI_MenuButton::onMenuItemSelected);
}


void
GUI_MenuButton::setText(QString text)
{
    _text = text;
    QFont currFont = QApplication::font();
    QFontMetrics fm(currFont);

    setWidth(fm.horizontalAdvance(text) + 10);
}


void
GUI_MenuButton::setBgColor(QColor col)
{
    _bgCol = col;
}


GUI_MenuItem*
GUI_MenuButton::addMenu(QString itemName, QVariantMap itemData)
{
    return _menu->addItem(itemName, itemData);
}
    

GUI_MenuItem*
GUI_MenuButton::addAction(QString itemName)
{
    return _menu->addItem(itemName);
}


void
GUI_MenuButton::addSeparator()
{
   _menu->addSeparator();
}






void
GUI_MenuButton::mousePressEvent(QMouseEvent *event)
{
    RLP_LOG_DEBUG("")

    _menu->setPos(position().x(), position().y() + height());
    qInfo() << "setting menu item vis to " << !_menu->isVisible();

    _menu->setVisible(!_menu->isVisible());

    RLP_LOG_DEBUG(_menu->isVisible())

    event->setAccepted(true);

    if (_menu->isVisible())
    {
        emit menuShown(_text);

    } else
    {
        emit menuHidden(_text);
    }
}


void
GUI_MenuButton::hoverEnterEvent(QHoverEvent *event)
{
    _focused = true;

    if ((_showMenuOnHover) && (!_menu->isVisible())) {

        _menu->setPos(position().x(), position().y() + height());
        _menu->setVisible(true);

        emit menuShown(_text);
    }

    update();
}


void
GUI_MenuButton::hoverLeaveEvent(QHoverEvent* event)
{
    _focused = false;
    update();
}


void
GUI_MenuButton::onMenuItemSelected(QVariantMap item)
{
    Q_UNUSED(item);

    _menu->setVisible(false);
}


void
GUI_MenuButton::setPos(qreal xpos, qreal ypos)
{
    GUI_ItemBase::setPos(xpos, ypos);
    // RLP_LOG_DEBUG("")

    _menu->setPos(position().x(), position().y() + height());
}


QRectF
GUI_MenuButton::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_MenuButton::paint(GUI_Painter* painter)
{
    painter->setPen(_bgCol);
    if (_focused)
    {
        painter->setBrush(GUI_Style::BgMidGrey);
    } else
    {
        painter->setBrush(GUI_Style::BgDarkGrey);
    }
    painter->drawRect(boundingRect());

    painter->setPen(Qt::white);
    painter->drawText(5, 15, _text);
}