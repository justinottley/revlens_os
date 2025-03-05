
#include "RlpRevlens/GUI_TL2/BookendDragButton.h"

#include "RlpGui/BASE/Scene.h"

#include "RlpRevlens/GUI_TL2/Bookends.h"
#include "RlpRevlens/GUI_TL2/View.h"


RLP_SETUP_LOGGER(revlens, GUI_TL2, BookendDragButton)

const int GUI_TL2_BookendDragButton::BUTTON_WIDTH = 10;

GUI_TL2_BookendDragButton::GUI_TL2_BookendDragButton(ButtonSide side, GUI_TL2_Bookends* bookends):
    GUI_ItemBase(bookends),
    _side(side),
    _bookends(bookends),
    _col(QColor(80, 60, 60)),
    _hover(false),
    _colOffStart(QColor(50, 50, 50)),
    _colOffEnd(QColor(65, 50, 50)),
    _colOnStart(QColor(75, 75, 75)),
    _colOnEnd(QColor(90, 75, 75)),
    _xpos(0),
    _posFrame(-1)
{
    setWidth(GUI_TL2_BookendDragButton::BUTTON_WIDTH);
    setZValue(6);
    setAcceptHoverEvents(true);
    setParentItem(bookends);
}


float
GUI_TL2_BookendDragButton::colStartPos()
{
    float result = 0.0;
    if (_side == BTN_LEFT) {
        result = 0.0;
    } else {
        result = 0.4;
    }

    return result;
}


float
GUI_TL2_BookendDragButton::colEndPos()
{
    float result = 0.0;
    if (_side == BTN_LEFT) {
        result = 0.4;
    } else {
        result = 1.0;
    }

    return result;
}


QColor
GUI_TL2_BookendDragButton::colStart()
{
    QColor result;

    if (_side == BTN_LEFT) {
        if (_hover) {
            result =_colOnStart;
        } else {
            result = _colOffStart;
        }
    } else {
        if (_hover) {
            result = _colOnEnd;
        } else {
            result = _colOffEnd;
        }
    }

    return result;
}


QColor
GUI_TL2_BookendDragButton::colEnd()
{
    QColor result;

    if (_side == BTN_LEFT) {
        if (_hover) {
            result = _colOnEnd;
        } else {
            result = _colOffEnd;
        }
    } else {
        if (_hover) {
            result = _colOnStart;
        } else {
            result = _colOffStart;
        }
    }

    return result;
}


qreal
GUI_TL2_BookendDragButton::getPosAtFrame(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame)

    qreal result;
    if (_side == BTN_LEFT) {
        result = _bookends->view()->getXPosAtFrame(frame) + _bookends->view()->timelineXPosStart(); //  - width(); // (width() * 2);

    } else {
        result = _bookends->view()->getXPosAtFrame(frame + 1) + _bookends->view()->timelineXPosStart() + GUI_TL2_BookendDragButton::BUTTON_WIDTH + 1;
    }

    // RLP_LOG_DEBUG(frame << "pre:" << result << "xpos start:" << _bookends->view()->timelineXPosStart())

    result -= _bookends->view()->timelineXPosStart();
    return result;
}


void
GUI_TL2_BookendDragButton::setPosFrame(qlonglong frame)
{
    _posFrame = frame;
    _xpos = getPosAtFrame(_posFrame);

    // RLP_LOG_DEBUG((int)(_side) << "frame:" << frame << "xpos:" << _xpos)
    qreal setpos = _xpos;
    qreal minX = _bookends->view()->timelineXPosStart();
    qreal maxX = _bookends->view()->timelineWidth() + GUI_TL2_BookendDragButton::BUTTON_WIDTH;
    if (setpos < 0)
    {
        setpos = 0;
    } else if (setpos >= maxX)
    {
        setpos = maxX;
    }

    // RLP_LOG_DEBUG(setpos)

    setPos(setpos, 0);
}


void
GUI_TL2_BookendDragButton::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _hover = true;

    update();
}


void
GUI_TL2_BookendDragButton::hoverLeaveEvent(QHoverEvent* event)
{
    _hover = false;

    update();
}


void
GUI_TL2_BookendDragButton::mousePressEvent(QMouseEvent* event)
{
    event->accept();

    _inDrag = true;
}


void
GUI_TL2_BookendDragButton::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
    
    _inDrag = false;
    emit buttonFrameChanged(_side, _posFrame);
}


void
GUI_TL2_BookendDragButton::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    event->accept();

    QPointF tlPos = mapToItem(_bookends->view(), event->position());

    if (_inDrag) {
        qlonglong frame = _bookends->view()->getFrameAtXPos(tlPos.x() - _bookends->view()->timelineXPosStart());

        if ((frame < -1) || (frame > _bookends->view()->frameCount())) {
            return;
        }

        if (_side == BTN_LEFT) {
            frame += 1;
        }

        setPosFrame(frame);

        _bookends->update();
    }
}


void
GUI_TL2_BookendDragButton::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nheight)

    setHeight(_bookends->view()->timelineHeight() - 1);
    setPosFrame(_posFrame);
}


qreal
GUI_TL2_BookendDragButton::xpos() const
{
    // std::cout << "xpos(): " << _side << " " << _xpos << " frame: " << _posFrame << " " << _bookends->view()->getXPosAtFrame(0) << std::endl;
    return _xpos + width() - 1;
}


QRectF
GUI_TL2_BookendDragButton::boundingRect() const
{
    //
    // use _bookends->view()->timelineHeight()) as height to clip above the zoombar, also set painter
    // clipRegion to timelineClipRegion()
    //

    // return QRectF(xpos(), 0, width(), _bookends->view()->height()); 
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_BookendDragButton::paint(GUI_Painter* painter)
{
    // painter->save();

    // Turn on below to clip to theh timeline region
    //painter->setClipRegion(_bookends->view()->timelineClipFullHeightRegion());

    QRectF br = boundingRect();

    // RLP_LOG_DEBUG((int)_side << br << width() << height())

    QLinearGradient horGradient(br.topLeft(), br.topRight());
    
    horGradient.setColorAt(colStartPos(), colStart());
    horGradient.setColorAt(colEndPos(), colEnd());

    QBrush brush(horGradient);

    brush.setStyle(Qt::LinearGradientPattern);
    painter->setPen(QPen(QColor(40, 40, 40)));
    painter->setBrush(brush);

    painter->drawRect(boundingRect());

    // painter->restore();
}