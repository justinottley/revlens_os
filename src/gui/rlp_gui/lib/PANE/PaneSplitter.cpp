

#include "RlpGui/BASE/Style.h"

#include "RlpGui/PANE/PaneSplitter.h"
#include "RlpGui/PANE/Pane.h"
#include "RlpGui/BASE/Scene.h"


const int GUI_PaneSplitter::SIZE = 5;
bool GUI_PaneSplitter::AUTOHIDE = false;

RLP_SETUP_LOGGER(gui, GUI, PaneSplitter)

GUI_PaneSplitter::GUI_PaneSplitter(
    GUI_ItemBase* parent,
    int paneStartIdx):
        GUI_ItemBase(parent),
        _paneStartIndex(paneStartIdx),
        _parentPane(dynamic_cast<GUI_Pane*>(parent)),
        _focused(false),
        _pos(0.5),
        _posPixels(-1),
        _active(true),
        _drawLine(true)
{
    setAcceptHoverEvents(true);
}


void
GUI_PaneSplitter::setPosPercent(float pos, bool doUpdatePanes)
{
    _pos = pos;

    // for paint / refresh issue
    onParentSizeChanged(parentItem()->width(), parentItem()->height());

    if (doUpdatePanes) {
        updatePanes();
    }
}


void
GUI_PaneSplitter::setPosPixels(int pixels)
{
    // RLP_LOG_DEBUG(pixels)

    _posPixels = pixels;
    setPosFromPixels();

    onParentSizeChanged(parentItem()->width(), parentItem()->height());
    updatePanes();
}


void
GUI_PaneSplitter::setPosFromPixels()
{
    if (_posPixels < 0)
    {
        return;
    }


    qreal ph = _parentPane->height();
    qreal pw = _parentPane->width();

    // RLP_LOG_DEBUG(ph << pw)

    if ((orientation() == O_VERTICAL) && (ph > 0))
    {
        _pos = (float)_posPixels / (float)_parentPane->height();
    }
    else if ((orientation() == O_HORIZONTAL) && (pw > 0))
    {
        _pos = (float)_posPixels / (float)_parentPane->width();
    }


    GUI_PanePtr sp = _parentPane->pane(_paneStartIndex);
    sp->setScaleFactor(_pos - minPos());
                
    GUI_PanePtr np = _parentPane->pane(_paneStartIndex + 1);
    np->setScaleFactor(maxPos() - _pos);

    // RLP_LOG_DEBUG(_pos);

}


void
GUI_PaneSplitter::updatePos()
{
    // RLP_LOG_DEBUG(this << "PRE:" << _pos)

    setPosFromPixels();

    // RLP_LOG_DEBUG(_pos)

    if (orientation() == O_VERTICAL) {
        setPos(0, ((_parentPane->height() * _pos)));

    } else if (orientation() == O_HORIZONTAL) {
        setPos(_parentPane->width() * _pos, 0);

    }
    
}


void
GUI_PaneSplitter::updatePanes()
{
    // RLP_LOG_DEBUG(_pos << " maxPos: " << maxPos())

    GUI_PanePtr sp = _parentPane->pane(_paneStartIndex);
    sp->setScaleFactor(_pos - minPos());
                
    GUI_PanePtr np = _parentPane->pane(_paneStartIndex + 1);
    np->setScaleFactor(maxPos() - _pos);
    

    if (orientation() == O_VERTICAL) {

        qreal ppos = _parentPane->height() * _pos;
        if (_posPixels > 0)
        {
            ppos = _posPixels;
        }

        setPos(0, ppos);

        np->setPos(0, ppos + GUI_PaneSplitter::SIZE);

    } else if (orientation() == O_HORIZONTAL) {

        qreal ppos = _parentPane->width() * _pos;
        if (_posPixels > 0)
        {
            ppos = _posPixels;
        }
        setPos(ppos, 0);

        np->setPos(ppos + GUI_PaneSplitter::SIZE, 0);
    }
    
    sp->updatePanes(_paneStartIndex);
    np->updatePanes(_paneStartIndex + 1);

    update();
}


float
GUI_PaneSplitter::minPos()
{
    if (_paneStartIndex == 0) {
        return 0;
        
    } else {
        return _parentPane->splitter(_paneStartIndex - 1)->splitterPos();
    }
}


float
GUI_PaneSplitter::maxPos()
{
    if ((_paneStartIndex + 1) == _parentPane->splitterCount()) {
        return 1.0;
        
    } else {
        
        return _parentPane->splitter(_paneStartIndex + 1)->splitterPos();
    }
}


SplitOrientation
GUI_PaneSplitter::orientation() const
{
    return _parentPane->orientation();
}


void
GUI_PaneSplitter::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(width << height)

    if (orientation() == O_VERTICAL) {
        setWidth(width);
        setHeight(SIZE);
    } else if (orientation() == O_HORIZONTAL) {
        setWidth(SIZE);
        setHeight(height);
    }

}


void
GUI_PaneSplitter::mousePressEvent(QMouseEvent *event)
{
    if (!_active)
    {
        return;
    }

    // RLP_LOG_DEBUG("")

    _posPixels = -1;

    _inDrag = true;
    _dragStartPosG = event->globalPosition();
    _dragStartPos = position();

    // RLP_LOG_DEBUG(_dragStartPosG << _dragStartPos)
}


void
GUI_PaneSplitter::mouseMoveEvent(QMouseEvent* event)
{
    if (_inDrag) {

        // qInfo() << event->pos().y() << " " << event->pos().y();

        float pos = 0;

        // NOTE: Need to use scenePos to get stable (non jittery / recursive positioning / blinking)
        // behavior
        //
        if (orientation() == O_VERTICAL) {
            
            qreal newPos = _dragStartPosG.y() - event->globalPosition().y();
            // RLP_LOG_DEBUG(newPos)
            qreal newY = _dragStartPos.y() - newPos;

            pos = (float)newY / (float)_parentPane->height();

            // RLP_LOG_DEBUG("new: " << newY << "height:" << _parentPane->height() << "newPosPercent:" << pos)
            // RLP_LOG_DEBUG(newPos);

            // pos = ((float)event->pos().y() - ((float)_parentPane->position().y())) / (float)_parentPane->height();
            
            // RLP_LOG_DEBUG(event->pos().y());

        } else if (orientation() == O_HORIZONTAL) {
            // pos = ((float)event->pos().x() - (float)_parentPane->position().x()) / (float)_parentPane->width();

            qreal newPos = _dragStartPosG.x() - event->globalPosition().x();
            // RLP_LOG_DEBUG(newPos)
            qreal newX = _dragStartPos.x() - newPos;

            pos = (float)newX / (float)_parentPane->width();

        }

        setPosPercent(pos);
        updatePos();

    }
}


void
GUI_PaneSplitter::mouseReleaseEvent(QMouseEvent* event)
{
    _inDrag = false;
}


void
GUI_PaneSplitter::hoverEnterEvent(QHoverEvent *event)
{
    if (!_active)
    {
        return;
    }

    // RLP_LOG_DEBUG("")

    _focused = true;

    if (orientation() == O_VERTICAL) {
        // QApplication::setOverrideCursor(Qt::SizeVerCursor);
        
    } else if (orientation() == O_HORIZONTAL) {
        // QApplication::setOverrideCursor(Qt::SizeHorCursor);
    }

    update();
}


void
GUI_PaneSplitter::hoverLeaveEvent(QHoverEvent* event)
{
    
    if (!_active)
    {
        return;
    }


    QApplication::restoreOverrideCursor();

    #ifdef SCAFFOLD_WASM
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    #endif

    event->setAccepted(true);
    _focused = false;

    update();
}


QRectF
GUI_PaneSplitter::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_PaneSplitter::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(_active)

    

    QColor col = GUI_Style::BgLightGrey;

    if ((GUI_PaneSplitter::AUTOHIDE) && (!_focused))
    {
        col = GUI_Style::BgDarkGrey;
    }
    else if (!_active)
    {
        col = GUI_Style::BgDarkGrey;

    } else if (_focused) {
        col = Qt::gray;

    } else {
        col = GUI_Style::BgLightGrey;
    }

    painter->setPen(col);
    painter->setBrush(col);
    // painter->setBrush(QBrush(QColor(Qt::red)));

    // RLP_LOG_DEBUG(boundingRect())
    QRectF br = boundingRect();
    painter->drawRect(br);


    QPen p(Qt::black);
    p.setWidth(1);
    painter->setPen(p);


    if (_drawLine && 
        (GUI_PaneSplitter::AUTOHIDE) &&
        (!_focused))
    {
        if (orientation() == O_VERTICAL)
        {
            QRectF br1 = br.adjusted(0, 4, 0, 0);
            painter->drawLine(br1.topLeft(), br1.topRight());
        } else 
        {
            QRectF br1 = br.adjusted(4, 0, 0, 0);
            painter->drawLine(br1.topLeft(), br1.bottomLeft());
        }
    }

}
