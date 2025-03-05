

#include "RlpGui/BASE/ButtonBase.h"
#include "RlpGui/BASE/Scene.h"

RLP_SETUP_LOGGER(gui, GUI, ButtonBase)

GUI_ButtonBase::GUI_ButtonBase(QQuickItem* parent):
    GUI_ItemBase(parent),
    _menu(nullptr),
    _isToggled(false),
    _isOutlined(false),
    _showMenuOnHover(false)
{
}


void
GUI_ButtonBase::hoverMoveEvent(QHoverEvent* event)
{
    // RLP_LOG_INFO("")

    _moveTime = QTime::currentTime();
    _movePos = event->position();
}


void
GUI_ButtonBase::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    emit hoverEnter();

    _inHover = true;

    QTimer::singleShot(1000, this, &GUI_ButtonBase::checkForToolTip);

    update();
}


void
GUI_ButtonBase::hoverLeaveEvent(QHoverEvent* event)
{
    emit hoverLeave();

    clearToolTip();

    _inHover = false;

    update();
}
