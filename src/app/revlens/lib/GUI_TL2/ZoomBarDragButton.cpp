

#include "RlpRevlens/GUI_TL2/ZoomBarDragButton.h"

#include "RlpRevlens/GUI_TL2/ZoomBar.h"
#include "RlpRevlens/GUI_TL2/View.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, ZoomBarDragButton)

const int GUI_TL2_ZoomBarDragButton::BUTTON_SIZE = 20;

GUI_TL2_ZoomBarDragButton::GUI_TL2_ZoomBarDragButton(ButtonSide side, GUI_TL2_ZoomBar* zoomBar):
    GUI_ItemBase(zoomBar),
    _side(side),
    _zoomBar(zoomBar),
    _inDrag(false),
    _dragPos(0),
    _xposDelta(0)
{
    setWidth(GUI_TL2_ZoomBarDragButton::BUTTON_SIZE);
    setHeight(GUI_TL2_ZoomBarDragButton::BUTTON_SIZE);

    setZValue(4);
    // setParentItem(zoomBar);

}


qreal
GUI_TL2_ZoomBarDragButton::xposMax()
{
    return _zoomBar->view()->timelineWidth() - width() - 2;
}


qreal
GUI_TL2_ZoomBarDragButton::xposOffset()
{
    qreal rposMax = 0;
    if (_side == GUI_TL2_ZoomBarDragButton::BTN_RIGHT) {
        rposMax = xposMax();
    }

    return position().x() - rposMax;
}




void
GUI_TL2_ZoomBarDragButton::setXPos(int newpos)
{
    // RLP_LOG_WARN(newpos)

    // clamp to prevent drag past end
    //
    if ((_side == BTN_RIGHT) && (newpos == 0)) {
        // RLP_LOG_DEBUG("Restting right drag button to max")
        newpos = xposMax();

    }

    // RLP_LOG_DEBUG("Side:" << (int)_side << " setting position to" << newpos)

    setPos(newpos, 0);

    update();
}



void
GUI_TL2_ZoomBarDragButton::refresh()
{
    qreal mx = _zoomBar->view()->timelineXPosStart() + _zoomBar->bar()->width();
    if ((position().x() + (GUI_TL2_ZoomBarDragButton::BUTTON_SIZE * 2)) >= _zoomBar->view()->timelineWidth())
    {
        _xposDelta = 0; // clamp right
    }

    if ((_side == GUI_TL2_ZoomBarDragButton::BTN_RIGHT) &&
        (_xposDelta == 0) &&
        (_zoomBar->view()->zoom() == 1.0))
    {
        setPos(xposMax(), 0);
    }
}


void
GUI_TL2_ZoomBarDragButton::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    event->accept();

    _inDrag = true;
    _dragPosG = event->globalPosition().x();
    _dragPos = position().x();
    // _dragPos = event->pos().x() - _xpos;
}


void
GUI_TL2_ZoomBarDragButton::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
    
    _inDrag = false;
}


void
GUI_TL2_ZoomBarDragButton::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();
    
    if (_inDrag) {
        // int newpos = event->pos().x() - _dragPos;

        _xposDelta = event->globalPosition().x() - _dragPosG;
        qreal newpos = _dragPos + _xposDelta;
        // RLP_LOG_DEBUG("pos.x(): " << event->pos().x() << " newpos: " << newpos <<  "timelineXPosStart: " << _zoomBar->view()->timelineXPosStart());

        qreal xpm = xposMax();
        qreal xpo = newpos - xpm;

        // clamp to prevent drag past end
        //
        if ((_side == BTN_LEFT) && (newpos < 0))
        {
            newpos = 0;
        } else if ((_side == BTN_RIGHT) && (xpo > 0))
        {
            // RLP_LOG_DEBUG("CLAMPING RIGHT")
            newpos = xpm;
        }

        // RLP_LOG_DEBUG(_side << newpos << xpo)

        setPos(newpos, 0);
        // ((_side == BTN_RIGHT) && (newpos > 0))) {


        // _xpos = newpos;

        // std::cout << "xpos: " << _xpos << " xposOffset: " << _xposOffset << std::endl;
        emit buttonMoved(_side, newpos); // xpos());

    }
}


QRectF
GUI_TL2_ZoomBarDragButton::boundingRect() const
{
    return QRectF(0, 0, width(), width());
}


void
GUI_TL2_ZoomBarDragButton::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG((int)(_side) << width() << height() << "pos:" << position())


    if (_side == BTN_LEFT)
    {
        QBrush brush(_zoomBar->colBackground());
        painter->setBrush(brush);
    } else {
        QBrush brush(_zoomBar->colBackground());
        painter->setBrush(brush);
    }

    // QBrush brush(Qt::green); // _zoomBar->colBackground());
    QPen pen(_zoomBar->colBackground());

    
    painter->setPen(pen);
    painter->drawRect(boundingRect());


    QBrush fgBrush(_zoomBar->colForeground());

    int xp = 0;
    painter->setBrush(fgBrush);

    // Button handle
    //
    painter->drawRect(QRect(xp + 3, 1, 3, 14));
    painter->drawRect(QRect(xp + 7, 1, 3, 14));
    painter->drawRect(QRect(xp + 11, 1, 3, 14));

}

