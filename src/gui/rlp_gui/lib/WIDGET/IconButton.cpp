
#include "RlpCore/UTIL/Text.h"

#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/MENU/MenuPane.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"


RLP_SETUP_LOGGER(gui, GUI, IconButton)

GUI_IconButton::GUI_IconButton(
    QString filename,
    QQuickItem* parent,
    int size,
    int padding):
        GUI_ButtonBase(parent),
        _svgIcon(nullptr),
        _svgIconRight(true),
        _svgIconShowOnHover(false),
        _padding(padding),
        _iconHOffset(0),
        _iconYOffset(0),
        _textHOffset(0),
        _textYOffset(3),
        _menuXOffset(0),
        _menuYOffset(28),
        _text(""),
        _filename(filename),
        _drawOutline(false),
        _outlineBgCol(GUI_Style::BgMidGrey),
        _hoverCol(GUI_Style::BgLightGrey),
        
        _toggled(false)
{
    // RLP_LOG_DEBUG(filename)

    setWidth(size + 10);
    setHeight(size + 10);

    setAcceptHoverEvents(true);

    if (filename != "") {
        _icon = QImage(filename).scaledToHeight(size, Qt::SmoothTransformation);
    
    }
    // _icon.setSize(_size, _size);

    
    // NOTE: lifted from MenuButton
    //
    GUI_MenuPane* m = new GUI_MenuPane(this);
    
    m->setZValue(parent->z());
    m->setPos(position().x(), position().y() + height());
    m->setVisible(false);
    _menu = m;

    connect(
        m,
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_IconButton::onMenuItemSelected
    );


}


void
GUI_IconButton::setSvgIconPath(QString path, int size, bool changeColour)
{
    if (size == -1)
    {
        size = height();
    }

    _svgIcon = new GUI_SvgIcon(path, this, size, _padding, GUI_Style::IconBg, changeColour);
    connect(
        _svgIcon,
        &GUI_SvgIcon::buttonPressed,
        this,
        &GUI_IconButton::onButtonPressed
    );

    qreal nwidth = width() + _svgIcon->width();
    setWidth(nwidth);

    if (_svgIconRight)
    {
        _svgIcon->setPos(nwidth - _svgIcon->width() - _padding, 0);
    }
}


void
GUI_IconButton::setSvgIconShowOnHover(bool ssh)
{
    _svgIconShowOnHover = ssh;
    _svgIcon->setVisible(false);
}

void
GUI_IconButton::setText(QString text)
{
    // RLP_LOG_DEBUG(text)

    _text = text;
    qreal nwidth = 10 + UTIL_Text::getWidth(text) + _padding;

    if (_svgIcon != nullptr)
    {
        nwidth += _svgIcon->width();
    }

    if (_icon.width() > 0)
    {
        nwidth += _icon.width();
    }

    setWidth(nwidth);

    if ((_svgIcon != nullptr) && (_svgIconRight))
    {
        _svgIcon->setPos(nwidth - _svgIcon->width(), _svgIcon->y());
    }
}


void
GUI_IconButton::setToggled(bool isToggled)
{
    _toggled = isToggled;
    update();
}


void
GUI_IconButton::onButtonPressed(QVariantMap metadata)
{
    GUI_MenuPane* menu = qobject_cast<GUI_MenuPane*>(_menu);

    if (menu->itemCount() > 0)
    {
        // toggleMenu();
        // _menu->onSceneLayoutChanged();

        // RLP_LOG_DEBUG("TOGGLE MENU - " << position().x() << position().y() + height())

        // qreal gposy = mapToGlobal(position()).y();
        QPointF spos = mapToScene(position());
        QPointF ppos = mapToScene(parentItem()->position());

        // RLP_LOG_DEBUG(spos)
        // TODO FIXME: not positioning quite right, but why???
        // QPointF gpos = mapToScene(position());
        // menu->setPos(gpos.x(), gpos.y() + 25); // + height() + 25);

        // QPointF spos = mapFromItem(scene(), position());
        // RLP_LOG_DEBUG(ppos)

        // NOTE: weird solution determined empirically
        menu->setPos(ppos.x() + _menuXOffset, spos.y() + _menuYOffset); // + height() + 25);

        menu->setVisible(!menu->isVisible());

        if (menu->isVisible())
        {
            emit menuShown(metadata);
        } else
        {
            emit menuHidden(metadata);
        }

 
    } else
    {
        emit buttonPressed(metadata);
    }
}


// -----

void
GUI_IconButton::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    event->setAccepted(true);

    onButtonPressed(_metadata);
}


void
GUI_IconButton::hoverEnterEvent(QHoverEvent* event)
{
    if (_svgIconShowOnHover)
    {
        _svgIcon->setVisible(true);
    }

    GUI_ButtonBase::hoverEnterEvent(event);

    if (_showMenuOnHover)
    {
        GUI_MenuPane* menu = qobject_cast<GUI_MenuPane*>(_menu);
        if (!menu->isVisible())
        {
            qobject_cast<GUI_Scene*>(scene())->hideMenuPanes();
            onButtonPressed(_metadata);
        }
        
    }
}


void
GUI_IconButton::hoverLeaveEvent(QHoverEvent* event)
{
    if (_svgIconShowOnHover)
    {
        _svgIcon->setVisible(false);
    }

    if (_svgIconShowOnHover)
    {
        QTimer::singleShot(1500, this, &GUI_IconButton::hideMenu);
    }
    

    GUI_ButtonBase::hoverLeaveEvent(event);
}


void
GUI_IconButton::hideMenu()
{
    if (_menu->isVisible() && (!qobject_cast<GUI_MenuPane*>(_menu)->isFocused()))
    {
        _menu->setVisible(false);
    }
}


void
GUI_IconButton::onMenuItemSelected(QVariantMap itemData)
{
    Q_UNUSED(itemData)

    _menu->setVisible(false);
}




GUI_MenuItem*
GUI_IconButton::addAction(QString itemName)
{
    return qobject_cast<GUI_MenuPane*>(_menu)->addItem(itemName);
}


QRectF
GUI_IconButton::boundingRect() const
{
    return QRectF(
        0,
        0,
        width() - 1, // + _textHOffset + (_padding * 2),
        height() - 1 // + _textYOffset + (_padding * 2)
    );
}


void
GUI_IconButton::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(_inHover)
    // RLP_LOG_DEBUG(_filename << " " << _text)

    if (_drawOutline)
    {
        QPen p(Qt::black);
        p.setWidth(1);

        painter->setPen(p);
        painter->setBrush(_outlineBgCol);
        painter->drawRect(boundingRect());
    }
    
    if ((_inHover) || (_toggled))
    {
        painter->setPen(Qt::black);
        painter->setBrush(_hoverCol);
        painter->drawRect(boundingRect());
    }

    if (_icon.width() > 0)
    {
        painter->drawImage(
            _padding + _iconHOffset,
            _padding + _iconYOffset + 1,
            &_icon
        );

    }
    
    if (_text.length() > 0)
    {

        int iconWidth = _icon.width();
        if ((_svgIcon != nullptr) && (!_svgIconRight))
        {
            iconWidth += _svgIcon->width();
        }
    
        painter->setPen(Qt::white);
        painter->drawText(
            iconWidth + 5 + _textHOffset,
            17 + _textYOffset,
            _text
        );
    }

}