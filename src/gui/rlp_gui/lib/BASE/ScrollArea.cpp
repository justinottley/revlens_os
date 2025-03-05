
#include <cmath>

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"


#include "RlpGui/BASE/ScrollArea.h"


RLP_SETUP_LOGGER(core, GUI, ScrollBarHandle)
RLP_SETUP_LOGGER(core, GUI, VerticalScrollBarHandle)
RLP_SETUP_LOGGER(core, GUI, HorizontalScrollBarHandle)
RLP_SETUP_LOGGER(core, GUI, ScrollArea)
RLP_SETUP_LOGGER(core, GUI, ScrollAreaContent)


GUI_ScrollBarHandle::GUI_ScrollBarHandle(GUI_ScrollArea* parent):
    GUI_ItemBase(parent),
    _hover(false),
    _posPercent(0.0),
    _opacity(1.0),
    _autoScroll(true)
{
    setSize(QSizeF(16, 16));
    setZValue(parent->z() + 1); // bit of a cheat to prevent drawing (text) over scrollbar handle
    setAcceptHoverEvents(true);
}


void
GUI_ScrollBarHandle::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _hover = true;
    event->accept();

    update();
}


void
GUI_ScrollBarHandle::hoverLeaveEvent(QHoverEvent* event)
{
    _hover = false;
    event->accept();

    update();
}


void
GUI_ScrollBarHandle::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    event->accept();
    _dragPosG = event->globalPosition();
    _dragPos = position();
}


void
GUI_ScrollBarHandle::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    event->accept();
}


QRectF
GUI_ScrollBarHandle::boundingRect() const
{
    return QRectF(
        1,
        1,
        width() - 1,
        height()
    );
}


void
GUI_ScrollBarHandle::resetPosition()
{
    // RLP_LOG_DEBUG("")

    _posPercent = 0.0;
    // RLP_LOG_DEBUG(pos().x() << " " << pos().y() << " " << parentItem()->position().y())
    //setPos(pos().x(), parentItem()->position().y());
}


void
GUI_ScrollBarHandle::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG(boundingRect() << position())

    // RLP_LOG_DEBUG(pos());
    // RLP_LOG_DEBUG(boundingRect());
    
    if (_hover) {

        painter->setPen(GUI_Style::BgGrey);
        painter->setBrush(GUI_Style::BgGrey);

    } else {
        painter->setPen(GUI_Style::BgLightGrey);
        painter->setBrush(GUI_Style::BgLightGrey);

    }

    // painter->setBrush(Qt::red);

    painter->setOpacity(_opacity);
    painter->drawRect(boundingRect());
}


// ------------------


GUI_VerticalScrollBarHandle::GUI_VerticalScrollBarHandle(GUI_ScrollArea* parent):
    GUI_ScrollBarHandle(parent)
{
    
}


void
GUI_VerticalScrollBarHandle::setYPosDelta(qreal ypos)
{
    // RLP_LOG_DEBUG(ypos << height() << position().y())

    int yypos = ypos + _dragPos.y() + 1;
    int theight = parentItem()->height(); // boundingRect().height();
    // float posPercent = (float)(theight - yypos) / (float)(theight - height());
    float posPercent = (float)yypos / (float)(theight - height());

    // RLP_LOG_DEBUG(ypos << yypos << "height:" << height() << posPercent)
    // RLP_LOG_DEBUG(posPercent << "ypos:" << ypos << "yypos:" << yypos << "theight:" << theight << "xpos:" << pos().x())
    

    if ((posPercent >= 0) && (posPercent <= 1.2)) {

        _posPercent = posPercent;
        setPos(pos().x(), yypos);

        emit handlePosChanged(_posPercent, yypos, true);
    }
    
    // Clip at top
    if (yypos < 0)
    {
        _posPercent = 0.0;
        setPos(pos().x(), 0);

        emit handlePosChanged(_posPercent, ypos, true);
    }
}


void
GUI_VerticalScrollBarHandle::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(event->position())
    
    event->accept();
    
    // NOTE: Not stable unless globalPosition() is used (instead of pos() which is unstable)
    //
    qreal yposDelta = event->globalPosition().y() - _dragPosG.y();
    setYPosDelta(yposDelta);
}


void
GUI_VerticalScrollBarHandle::onSizeRatioChanged(qreal nheight, qreal maxHeight)
{
    // RLP_LOG_DEBUG("")
    
    qreal sheight = nheight;

    if (maxHeight > nheight)
    {
        qreal handleHeight = nheight * (nheight / maxHeight);
        sheight = handleHeight;
    }

    if (sheight < MIN_HEIGHT)
    {
        // RLP_LOG_WARN("Clamping height to MIN:" << MIN_HEIGHT)
        sheight = MIN_HEIGHT;
    }
    
    setHeight(sheight);
}


void
GUI_VerticalScrollBarHandle::resetPosition()
{

    GUI_ScrollBarHandle::resetPosition();

    setPos(pos().x(), 0);

}



// 
// -------
//


GUI_HorizontalScrollBarHandle::GUI_HorizontalScrollBarHandle(GUI_ScrollArea* parent):
    GUI_ScrollBarHandle(parent)
{
    
}


void
GUI_HorizontalScrollBarHandle::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(event->position())
    
    event->accept();
    
    // NOTE: Not stable unless position() is used (instead of pos() which is unstable)
    //
    qreal xpos = event->globalPosition().x() - _dragPosG.x(); // event->globalPosition().x() - parentItem()->position().x() - _dragPos.x();

    setXPosDelta(xpos);
}

void
GUI_HorizontalScrollBarHandle::setXPosDelta(qreal xpos)
{
    int xxpos = xpos + _dragPos.x() + 1;
    int twidth = parentItem()->width();
    float posPercent = (float)xxpos / (float)(twidth - width());

    // RLP_LOG_DEBUG(position().x() << xpos << xxpos << posPercent)

    if ((posPercent >= 0) && (posPercent <= 1.0)) {

        // RLP_LOG_DEBUG("RUNNING UPDATE: " << xpos << " " << posPercent)

        _posPercent = posPercent;

        // Move / scroll to position
        setPos(xxpos, pos().y());
        
        update();

        emit handlePosChanged(_posPercent, xpos, true);
    }
}


void
GUI_HorizontalScrollBarHandle::onSizeRatioChanged(qreal znwidth, qreal maxWidth)
{
    qreal nwidth = parentItem()->width();

    // RLP_LOG_DEBUG(nwidth << maxWidth)
    
    if (maxWidth > nwidth) {

        qreal handleWidth = nwidth * (nwidth / maxWidth);


        setWidth(handleWidth);

        if (_autoScroll)
        {
            // autoscroll?
            //
            int newX = nwidth - handleWidth;

            setPos(newX, pos().y());
        }
        

    } else {

        setWidth(nwidth);
    }
    
}


void
GUI_HorizontalScrollBarHandle::resetPosition()
{
    // RLP_LOG_DEBUG("y: " << pos().y())

    GUI_ScrollBarHandle::resetPosition();
    setPos(0, pos().y());
}



//
// ---------------
//

GUI_ScrollAreaContent::GUI_ScrollAreaContent(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
}


QRectF
GUI_ScrollAreaContent::boundingRect() const
{
    return QRectF(0, 0, width() - 2, height() - 2);
}


void
GUI_ScrollAreaContent::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")
    

    // painter->setBrush(QBrush(Qt::green));
    // painter->drawRect(boundingRect());
}


// -------------
// -------------
// -------------



GUI_ScrollArea::GUI_ScrollArea(QQuickItem* parent, qreal width, qreal height):
    GUI_ItemBase(parent),
    _maxChildHeight(0),
    _maxChildWidth(0),
    _autoHide(true),
    _inHover(false),
    _outlined(true),
    _hScrollBarVisible(true),
    _vhandlePosOffset(0),
    _bgCol(Qt::black)
{
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);
    
    _vhandle = new GUI_VerticalScrollBarHandle(this);
    _vhandle->setOpacity(0.5);
    _vhandle->setPos(width - _vhandle->width(), 0);

    _hhandle = new GUI_HorizontalScrollBarHandle(this);
    _hhandle->setOpacity(0.5);
    _hhandle->setPos(0, height - _hhandle->height() - 2);

    _contentWrapper = new GUI_ItemBase(this);
    _contentWrapper->setFlag(QQuickItem::ItemClipsChildrenToShape, true);
    _content = new GUI_ScrollAreaContent(_contentWrapper);

    connect(
        _vhandle,
        &GUI_VerticalScrollBarHandle::handlePosChanged,
        this,
        &GUI_ScrollArea::onVHandlePosChanged
    );

    connect(
        _hhandle,
        &GUI_HorizontalScrollBarHandle::handlePosChanged,
        this,
        &GUI_ScrollArea::onHHandlePosChanged
    );

    setSize(QSizeF(width, height));

    updateHHandleVis();
    updateVHandleVis();
}


/*
QVariant
GUI_ScrollArea::itemChange(
    QGraphicsItem::GraphicsItemChange change
    const QVariant &value)
{
    if (change == QGraphicsItem::Item)

}
*/


void
GUI_ScrollArea::onSizeChangedInternal()
{
    // RLP_LOG_DEBUG("")

    // _vhandle->setPos(width() - 20 - _vhandle->width(), 0);
    // _vhandle->setPos(10, 0);
}


void
GUI_ScrollArea::onVHandlePosChanged(float posPercent)
{
    qreal fHeight = -(_maxChildHeight - height()) * posPercent;

    // RLP_LOG_DEBUG(posPercent << fMaxHeight << "height:" << height() << "fHeight:" << fHeight)

    // _content->setPos(_content->pos().x(), -_maxChildHeight * posPercent);
    _content->setPos(_content->pos().x(), fHeight);
    emit contentPosChanged(_content->pos().x(), fHeight);
}


void
GUI_ScrollArea::setYPosPercent(float posPercent)
{
    // RLP_LOG_DEBUG(posPercent)

    onVHandlePosChanged(posPercent);

    int h = height() - _vhandle->height();
    qreal ypos = posPercent * h;

    // RLP_LOG_DEBUG(posPercent << ypos)

    _vhandle->setPos(_vhandle->pos().x(), ypos);
}


void
GUI_ScrollArea::onHHandlePosChanged(float posPercent, bool clamp)
{
    // RLP_LOG_DEBUG("current pos: " << _content->pos().x() << " incoming pos percent: " << posPercent << " width: " << width() << " max: " << _maxChildWidth <<  " new pos: " << -_maxChildWidth * posPercent)
    
    if (_maxChildWidth > width())
    {
        int newX = int(std::rint(_maxChildWidth - width()) * posPercent);

        // RLP_LOG_DEBUG("NEW X: " << newX)

        if ((clamp) && (newX < 10)) {
            newX = 0;

        } else {
            newX += 20;
        }
        

        _content->setPos(-newX, _content->pos().y());
        emit contentPosChanged(-newX, _content->pos().y());
    }


}


void
GUI_ScrollArea::onParentSizeChanged(qreal width, qreal height)
{
    int hoffset = 0; // 20;

    height -= (hoffset + _heightOffset);

    // RLP_LOG_DEBUG(width << " " << height << "max child height:" << _maxChildHeight << "height offset:" << _heightOffset)

    QSizeF ns(width - _widthOffset, height);
    setSize(ns);

    _content->setWidth(width - 20);
    _content->setHeight(height - hoffset);
    
    _contentWrapper->setWidth(width - 20);
    _contentWrapper->setHeight(height - hoffset);

    _vhandle->setPos(width - _vhandlePosOffset - _vhandle->width(), _vhandle->pos().y());
    _vhandle->onSizeRatioChanged(height, _maxChildHeight);

    // RLP_LOG_DEBUG("vhandle pos:" << _vhandle->pos().x());

    _hhandle->setPos(_hhandle->pos().x(), height - _hhandle->height() - 2);
    _hhandle->onSizeRatioChanged(width, _maxChildWidth);

    // RLP_LOG_DEBUG("autohide: " << _autoHide << " maxW: " << _maxChildWidth << " w: " << width)
    // RLP_LOG_DEBUG("autohide: " << _autoHide << " maxH: " << _maxChildHeight << " h: " << height)


    if (_autoHide) {

        updateVHandleVis();
        updateHHandleVis();

    } else {

        _hhandle->setVisible(true);
        _vhandle->setVisible(true);
    }

    emit sizeChanged(width, height);
}


void
GUI_ScrollArea::onChildPosChanged(GUI_ItemBase* item, qreal xpos, qreal ypos)
{
    RLP_LOG_DEBUG(item << " " << xpos << " " << ypos)

    qreal childMaxYPos = ypos + item->boundingRect().height();
    if (childMaxYPos > _maxChildHeight) {
        _maxChildHeight = childMaxYPos;

        _vhandle->onSizeRatioChanged(height(), _maxChildHeight);
    }
}


void
GUI_ScrollArea::setSAHeight(qreal height)
{
    setHeight(height);
    _hhandle->setPos(_hhandle->pos().x(), height - _hhandle->height() - 2);
}


void
GUI_ScrollArea::setMaxChildHeight(qreal nheight)
{
    int heightDiff = abs(_maxChildHeight - nheight);

    // RLP_LOG_DEBUG("curr max height: " << _maxChildHeight << " new max height: " << nheight << "height" << height())

    _maxChildHeight = nheight;
    _vhandle->onSizeRatioChanged(height(), _maxChildHeight);

    updateVHandleVis();
}


void
GUI_ScrollArea::setMaxChildWidth(qreal nwidth)
{
    qreal widthDiff = _maxChildWidth - nwidth;

    // RLP_LOG_DEBUG("max width: " << nwidth << " curr width: " << width() << "widthdiff:" << widthDiff)

    _maxChildWidth = nwidth;
    _hhandle->onSizeRatioChanged(nwidth, _maxChildWidth);

    updateHHandleVis();

    // RLP_LOG_DEBUG("WIDTHDIFF: " << widthDiff)

    if ((widthDiff > 0) && (widthDiff < 30))
    {
        // RLP_LOG_DEBUG("RESET POSITION?")
        // resetPosition();

    } else {
        
        // RLP_LOG_DEBUG("AUTO SCROLLING!!!!!!!!!!")
        if (_hhandle->autoScroll())
        {
            onHHandlePosChanged(1.0, false);
        }
        
    }
}

void
GUI_ScrollArea::updateHHandleVis()
{
    // RLP_LOG_DEBUG(_maxChildWidth << width())

    if ((_autoHide) && (_maxChildWidth < (width() - 30))) {
        _hhandle->setVisible(false);
        _hhandle->resetPosition();

        _content->setPos(0, _content->pos().y());
        emit contentPosChanged(0, _content->pos().y());
        
        
    } else if (_hScrollBarVisible) {

        // RLP_LOG_DEBUG("Showing horizontal scrollbar")
        _hhandle->setVisible(true);
    }
}


void
GUI_ScrollArea::updateVHandleVis()
{
    // RLP_LOG_DEBUG("")

    if ((_autoHide) && (_maxChildHeight < (height() - 10)))
    {
        // RLP_LOG_DEBUG("Hiding Vertical Scrollbar")

        _vhandle->setVisible(false);
        _vhandle->resetPosition();

        _content->setPos(_content->pos().x(), 0);
        emit contentPosChanged(_content->pos().x(), 0);
        
    } else
    {
        // RLP_LOG_DEBUG("Showing Vertical SCrollbar")
        _vhandle->setVisible(true);
    }
}


void
GUI_ScrollArea::setAutoHide(bool autoHide)
{
    _autoHide = autoHide;
    setMaxChildHeight(_maxChildHeight); // to reset scrollbars if necessary
    setMaxChildWidth(_maxChildWidth);
}


void
GUI_ScrollArea::setHScrollBarVisible(bool isVisible)
{
    // RLP_LOG_DEBUG(isVisible)

     _hScrollBarVisible = isVisible;
     if (!_hScrollBarVisible)
     {
        _hhandle->setVisible(false);
     }
}


void
GUI_ScrollArea::resetPosition()
{
    // RLP_LOG_DEBUG("")

    _vhandle->resetPosition();
    _hhandle->resetPosition();
    
    onVHandlePosChanged(0.0);
    onHHandlePosChanged(0.0);

}


void
GUI_ScrollArea::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(event);
}


void
GUI_ScrollArea::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG(event);
}



void
GUI_ScrollArea::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    // setFocus();
    _inHover = true;

    emit focusChanged(Qt::MouseFocusReason, _inHover);
    update();
}


void
GUI_ScrollArea::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    // clearFocus();
    _inHover = false;

    emit focusChanged(Qt::MouseFocusReason, _inHover);

    update();
}


void
GUI_ScrollArea::scrollBy(int ypos)
{
    qreal nypos = _vhandle->pos().y() - (ypos * 0.25);
    float posPercent = nypos / (float)(height() - _vhandle->height());
    
    if (posPercent < 0)
    {
        posPercent = 0;
    }


    if ((posPercent >= 0) && (posPercent <= 1.2))
    {
        setYPosPercent(posPercent);
    }
}


void
GUI_ScrollArea::wheelEvent(QWheelEvent* event)
{

    QPoint numPixels = event->pixelDelta();
    QPoint numDeg = event->angleDelta();

    // RLP_LOG_DEBUG(event->position() << event->globalPosition() << numPixels << numDeg)


    if (!numPixels.isNull()) {
        scrollBy(numPixels.y());
    }

    else if (!numDeg.isNull())
    {
        // Windows
        scrollBy(numDeg.y());
    }
}


QRectF
GUI_ScrollArea::boundingRect() const
{
    // return QRectF(0, 0, width() + _vhandle->width(), height() + _hhandle->height());
    return QRectF(0, 0, width() -1, height() - 1);
}


void
GUI_ScrollArea::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")
    if (_outlined)
    {
        QPen p(Qt::black);

        if (_inHover) {
            p.setWidth(5);
        } else {
            p.setWidth(1);
        }

        painter->setPen(p);
        painter->setBrush(_bgCol);
        painter->drawRect(boundingRect());
    }

}
