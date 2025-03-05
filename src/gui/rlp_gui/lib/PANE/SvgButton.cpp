
#include "RlpGui/PANE/SvgButton.h"

#include "RlpGui/MENU/MenuPane.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"

#include "RlpCore/UTIL/Text.h"

RLP_SETUP_LOGGER(gui, GUI, SvgButton)
RLP_SETUP_LOGGER(gui, GUI, SvgToggleButton)

GUI_SvgButton::GUI_SvgButton(
    QString filename,
    QQuickItem* parent,
    int size,
    int padding,
    QColor fgCol):
        GUI_ButtonBase(parent),
        _icon(nullptr),
        _overlayText(""),
        _overlayPosX(0),
        _overlayPosY(0),
        _bgCol(GUI_Style::BgMidGrey),
        _size(size + padding),
        _menuXPosOffset(0),
        _menuYPosOffset(0)
{
    // RLP_LOG_DEBUG(filename)

    setSize(QSizeF(_size, _size));
    setAcceptHoverEvents(true);

    if (filename != "")
    {
        _icon = new GUI_SvgIcon(filename, this, size, padding, fgCol);
        setSvgIcon(_icon);
    }
    
    // _icon->setFlag(QGraphicsItem::ItemStacksBehindParent);

    // NOTE: lifted from MenuButton
    //
    _menu = new GUI_MenuPane(this);
    _menu->setPos(position().x(), position().y() + _size);
    _menu->setVisible(false);

    connect(
        _menu,
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_SvgButton::onMenuItemSelected
    );
}


void
GUI_SvgButton::connectToMenu()
{
    // RLP_LOG_DEBUG("")

    connect(
        this, &GUI_SvgButton::buttonPressed,
        this, &GUI_SvgButton::toggleMenu
    );
}


void
GUI_SvgButton::initIcon(GUI_SvgIcon* icon)
{
    icon->setZValue(-1); // draw behind parent

    connect(
        icon,
        &GUI_SvgIcon::buttonPressed,
        this,
        &GUI_SvgButton::buttonPressed
    );

    connect(
        icon,
        &GUI_SvgIcon::hoverChanged,
        this,
        &GUI_SvgButton::onIconHoverChanged
    );
}


void
GUI_SvgButton::setSvgIcon(GUI_SvgIcon* icon)
{
    _icon = icon;
    initIcon(icon);
}


void
GUI_SvgButton::setOutlined(bool outlined)
{
    RLP_LOG_DEBUG("")

    // GUI_ButtonBase::setOutlined(outlined);

    if (_icon != nullptr)
    {
        _icon->setOutlined(outlined);
        _icon->update();
    }
    
}


void
GUI_SvgButton::setOverlayText(QString text)
{
    // RLP_LOG_DEBUG("")

    _overlayText = text;

    setWidth(_size + UTIL_Text::getWidth(text) + _overlayPosX);
}


void
GUI_SvgButton::setOverlayTextPos(int x, int y)
{
    // RLP_LOG_DEBUG(x << y)

    _overlayPosX = x;
    _overlayPosY = y;

    setWidth(_size + UTIL_Text::getWidth(_overlayText) + _overlayPosX);
    setHeight(_size + _overlayPosY);
}


void
GUI_SvgButton::setIconPos(int x, int y)
{
    if (_icon != nullptr)
    {
        // RLP_LOG_DEBUG(x << y)
        _icon->setPos(x, y);

    } else {
        RLP_LOG_WARN("invalid icon")
    }
    
}


void
GUI_SvgButton::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_INFO("")

    event->setAccepted(true);

    QVariantMap md = _metadata;
    md.insert("toggled", _isToggled);
    md.insert("button", event->button());

    emit buttonPressed(md);
}


void
GUI_SvgButton::hoverEnterEvent(QHoverEvent* event)
{
    GUI_ButtonBase::hoverEnterEvent(event);

    // RLP_LOG_INFO("")

    if (_icon != nullptr)
        _icon->hoverEnterEvent(event);

    emit hoverChanged(true);
}


void
GUI_SvgButton::hoverLeaveEvent(QHoverEvent* event)
{
    GUI_ButtonBase::hoverLeaveEvent(event);

    // RLP_LOG_INFO("")

    if (_icon != nullptr)
        _icon->hoverLeaveEvent(event);

    emit hoverChanged(false);
}


void
GUI_SvgButton::toggleMenu(QVariantMap /* mdata */)
{
    // RLP_LOG_DEBUG("")

    qobject_cast<GUI_Scene*>(scene())->hideMenuPanes(_menu);

    QPointF pos = mapToScene(position());
    QPointF ppos = mapToScene(parentItem()->position());

    // NOTE: weird calculation determined empircally
    // _menu->setPos(pos.x() - position().x() - width(), ppos.y() + _size); // ppos.x(), ppos.y() + _size);

    _menu->setPos(
        pos.x() - position().x() - width() + _menuXPosOffset,
        pos.y() + _size + _menuYPosOffset
    ); // ppos.x(), ppos.y() + _size);

    _menu->setVisible(!_menu->isVisible());
}


void
GUI_SvgButton::onIconHoverChanged(bool inHover)
{
    if (inHover)
    {
        emit hoverEnter();
    } else {
        emit hoverLeave();
    }
}


void
GUI_SvgButton::onMenuItemSelected(QVariantMap sigData)
{
    Q_UNUSED(sigData)

    _menu->setVisible(false);
}


QRectF
GUI_SvgButton::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_SvgButton::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(_icon->filename())

    // painter->setBrush(QBrush(Qt::red));
    // painter->drawRect(boundingRect());

    // if (_drawOutline) {

    //     painter->setPen(GUI_Style::BgDarkGrey);
    //     painter->setBrush(_bgCol);
    //     painter->drawRect(boundingRect());
    // }

    if (_overlayText.size() > 0)
    {
        QFont f = painter->font();
        f.setPixelSize(GUI_Style::FontPixelSizeDefault - 3);
        f.setBold(true);

        painter->setFont(f);
        painter->setPen(QPen(Qt::white));
        painter->drawText(_overlayPosX, _overlayPosY + 5, _overlayText);
    }
}


// ------------------------------------------------------------


GUI_SvgToggleButton::GUI_SvgToggleButton(
    QString filenameOn,
    QString filenameOff,
    QQuickItem* parent,
    int size,
    int padding,
    QColor fgCol):
        GUI_SvgButton(filenameOn, parent, size, padding, fgCol)
{
    _onIcon = new GUI_SvgIcon(filenameOff, this, size, padding, fgCol);
    _onIcon->setVisible(false);

    initIcon(_onIcon);
}


void
GUI_SvgToggleButton::setToggled(bool toggled)
{
    GUI_SvgButton::setToggled(toggled);

    // RLP_LOG_DEBUG(toggled)

    if (!toggled)
    {
        _icon->setVisible(true);
        _onIcon->setVisible(false);
    } else
    {
        _icon->setVisible(false);
        _onIcon->setVisible(true);
    }
}