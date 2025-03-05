
#include "RlpGui/BASE/FrameOverlay.h"

RLP_SETUP_LOGGER(gui, GUI, FrameOverlay)

GUI_FrameOverlay::GUI_FrameOverlay(int width, int height, GUI_ItemBase* parent):
    GUI_FrameBase(parent),
    _outlined(false),
    _opacity(0.75),
    _col(Qt::white),
    _text(""),
    _textX(5),
    _textY(15)
{
    setSize(QSizeF(width, height));
    setAcceptHoverEvents(true);
    // setZValue(19);
}



void
GUI_FrameOverlay::mousePressEvent(QMouseEvent* event)
{
    // GUI_FrameBase::mousePressEvent(event);
}


void
GUI_FrameOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    // GUI_FrameBase::mouseReleaseEvent(event);
}


void
GUI_FrameOverlay::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")
}


void
GUI_FrameOverlay::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")
}


void
GUI_FrameOverlay::onSizeRectChanged(QRectF rect)
{
    // RLP_LOG_DEBUG(rect)
    setSize(QSizeF(rect.width(), rect.height()));
}


QRectF
GUI_FrameOverlay::boundingRect() const
{
    return QRectF(0, 0, width() - 1, height() - 1);
}


void
GUI_FrameOverlay::paint(GUI_Painter* painter)
{
    painter->save();
    painter->setOpacity(_opacity);
    painter->setPen(_col);
    painter->setBrush(QBrush(_col));

    if (_outlined)
    {
        painter->setPen(Qt::white);
        painter->setBrush(QBrush(_col));
    }

    painter->drawRect(boundingRect());

    if (_text != "")
    {
        painter->setPen(Qt::white);
        painter->drawText(_textX, _textY, _text);
    }

    painter->restore();  
}
