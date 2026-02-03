
#include "RlpRevlens/GUI_TL2/Playhead.h"

#include "RlpRevlens/GUI_TL2/ViewBase.h"
#include "RlpRevlens/GUI_TL2/Ticks.h"
#include "RlpRevlens/GUI_TL2/Controller.h"


RLP_SETUP_LOGGER(revlens, GUI_TL2, Playhead)

GUI_TL2_Playhead::GUI_TL2_Playhead(GUI_TL2_ViewBase* view):
    GUI_ItemBase(view->timelineArea()),
    _view(view),
    _yOffset(5),
    _mousePosTextXOffset(4),
    _mousePosTextYOffset(24),
    _inDrag(false)
{
    setWidth(view->timelineWidth());
    setHeight(view->timelineHeight());

    setZValue(5);
    setAcceptHoverEvents(true);
}


void
GUI_TL2_Playhead::mouseMoveEventInternal(int xpos)
{
    // xpos -= _view->timelineXPosStart();
    _mouseFrame = _view->getFrameAtXPos(xpos);

    // RLP_LOG_DEBUG(xpos << ":" << _mouseFrame)

    if (_inDrag)
    {
        _view->onPlayheadScrubbing(xpos);
    }
}


void
GUI_TL2_Playhead::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")
    GUI_TL2_Controller::instance()->setFocusedTimelineView(_view);

    _inHover = true;
    event->accept();

    emit hoverPosChanged(event->position().x(), event->position().y());
}


void
GUI_TL2_Playhead::hoverMoveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(

    mouseMoveEventInternal(event->position().x());
    event->accept();

    update();

    emit hoverPosChanged(event->position().x(), event->position().y());
    // _view->ticks()->update();
}


void
GUI_TL2_Playhead::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = false;
    event->accept();

    emit hoverLeave();

    update();
}


void
GUI_TL2_Playhead::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) return;

    mouseMoveEventInternal(event->position().x());
    event->accept();
}


void
GUI_TL2_Playhead::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(event->pos().x())
    if (event->buttons() & Qt::RightButton)
    {
        emit rightClick(event->position());
        return;
    }

    if (!(event->buttons() & Qt::LeftButton)) return;

    event->accept();
    _inDrag = true;

    qreal xpos = event->position().x(); // - _view->timelineXPosStart();
    _view->onPlayheadStartScrub(xpos);
    
}


void
GUI_TL2_Playhead::mouseReleaseEvent(QMouseEvent* event)
{
    if (_inDrag)
    {
        qreal xpos = event->position().x(); // - _view->timelineXPosStart();
        _view->onPlayheadEndScrub(xpos);
    }

    event->accept();
    _inDrag = false;
}


void
GUI_TL2_Playhead::wheelEvent(QWheelEvent* event)
{
    // RLP_LOG_DEBUG(event)

    QPoint numPixels = event->pixelDelta();
    QPoint numDeg = event->angleDelta();

    if (!numPixels.isNull())
    {
        emit scrollWheelDelta(numPixels.y());

    } else if (!numDeg.isNull())
    {
        emit scrollWheelDelta(numDeg.y());
    }
    
}


void
GUI_TL2_Playhead::paintMousePos(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    qlonglong frameEnd = _view->frameEnd();
    
    if ((_mouseFrame < 1) || (_mouseFrame >= frameEnd))
    {
        return;
    }

    //
    // Mouse position playhead / frame num
    //
    QColor mposColor(200, 100, 100);
    
    
    float xposStart =  0; // _view->timelineXPosStart();
    float xposLeft = _view->getXPosAtFrame(_mouseFrame) + xposStart;
    float xposRight = _view->getXPosAtFrame(_mouseFrame + 1) + xposStart;

    painter->save();

    QFont f = painter->font();
    f.setBold(true);
    painter->setFont(f);

    painter->setPen(QPen(mposColor, 1));
    painter->setBrush(QBrush(mposColor, Qt::SolidPattern));

    painter->drawLine(xposLeft, 1, xposLeft, _view->timelineHeight() - 2);
    painter->drawLine(xposRight, 1, xposRight, _view->timelineHeight() - 2);

    painter->drawText(
        xposLeft + _mousePosTextXOffset,
        _mousePosTextYOffset,
        QString::number(_mouseFrame)
    );

    painter->restore();
}


void
GUI_TL2_Playhead::paintViewPos(GUI_Painter* painter)
{
    qlonglong currFrame = _view->currFrame();
    // RLP_LOG_DEBUG(currFrame);


    if ((currFrame < _view->controller()->inFrame() - 1) ||
        (currFrame > _view->controller()->outFrame() - 1))
    {
        return;
    }


    // painter->setRenderHint(QPainter::Antialiasing, true);
    QColor playheadPenColor(Qt::gray);

    QPen playheadPen = QPen(playheadPenColor, 1);
    QBrush playheadBrush = QBrush(Qt::white, Qt::SolidPattern);
    
    int xpos = _view->getXPosAtFrame(currFrame + 1); // + _view->timelineXPosStart();
    int yMaxHeight = _view->timelineHeight();

    int yOffset = _yOffset;

    QPoint playheadPoints[8] = {
        QPoint(xpos - 4, yOffset),
        QPoint(xpos - 1, yOffset + 4),
        QPoint(xpos - 1, yMaxHeight - 4),
        QPoint(xpos - 4, yMaxHeight),
        QPoint(xpos + 4, yMaxHeight),
        QPoint(xpos, yMaxHeight - 4),
        QPoint(xpos, yOffset + 4),
        QPoint(xpos + 4, yOffset)
        
    };

    painter->setBrush(playheadBrush);
    painter->setPen(playheadPen);

    painter->drawPolygon(playheadPoints, 8);

}


void
GUI_TL2_Playhead::refresh()
{
    // RLP_LOG_DEBUG("")

    setWidth(_view->timelineWidth());
    setHeight(_view->timelineHeight());
    setPos(0, 0);
}


void
GUI_TL2_Playhead::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    setWidth(_view->timelineWidth() - 1);
    setHeight(_view->timelineHeight() - 1);
}


QRectF
GUI_TL2_Playhead::boundingRect() const
{
    return QRectF(0, 0, width(), height());

    // return QRectF(0, 0, 0, 0);
    // return _view->timelineRect();
}


void
GUI_TL2_Playhead::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("");

    // painter->setBrush(QBrush(Qt::red));
    // painter->drawRect(boundingRect());

    // painter->save();
    // painter->setClipRect(_view->timelineClipRect(), Qt::IntersectClip);

    if (_inHover)
    {
        paintMousePos(painter);
    }

    paintViewPos(painter);

    // painter->restore();
    painter->setPen(Qt::transparent);
}