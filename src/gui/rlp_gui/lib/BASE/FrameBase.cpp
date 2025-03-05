

#include "RlpGui/BASE/FrameBase.h"
#include "RlpGui/BASE/Scene.h"

RLP_SETUP_LOGGER(core, GUI, FrameBase)

GUI_FrameBase::GUI_FrameBase(
    GUI_ItemBase* parent,
    QString name,
    SplitOrientation o):
        GUI_ItemBase(parent),
        _name(name),
        _orientation(o),
        _doResizeToParent(true)
{
    _focusGroup = new GUI_FocusGroup(this);

    setAcceptHoverEvents(true); // for focus
}


GUI_FrameBase::~GUI_FrameBase()
{
    // RLP_LOG_DEBUG("")
}


void
GUI_FrameBase::_setOrientation(SplitOrientation o)
{
    // qInfo() << "setOrientation(): " << o;
    
    _orientation = o;
}


void
GUI_FrameBase::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(metaObject()->className() << width << height)

    if (!_doResizeToParent)
    {
        return;
    }


    if (width != 0)
    {
        setWidth(width);
    }

    if (height != 0)
    {
        setHeight(height);
    }
}


void
GUI_FrameBase::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    qobject_cast<GUI_Scene*>(scene())->setFocusedPane(this);
}


void
GUI_FrameBase::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")
}


void
GUI_FrameBase::onTabKeyPressed(QKeyEvent* kevent)
{
    RLP_LOG_DEBUG("")

    _focusGroup->onTabKeyPressed(kevent);

    emit tabKeyPressed();
}