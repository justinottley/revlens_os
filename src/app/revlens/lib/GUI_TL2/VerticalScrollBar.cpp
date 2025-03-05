

#include "RlpRevlens/GUI_TL2/VerticalScrollBar.h"

#include "RlpRevlens/GUI_TL2/View.h"


RLP_SETUP_LOGGER(revlens, GUI_TL2, VerticalScrollBarHandle)
RLP_SETUP_LOGGER(revlens, GUI_TL2, VerticalScrollBar)

GUI_TL2_VerticalScrollBarHandle::GUI_TL2_VerticalScrollBarHandle(GUI_TL2_VerticalScrollBar* vbar):
    GUI_ItemBase(vbar),
    _vbar(vbar),
    _hover(false),
    _posPercent(0)
{
    setAcceptHoverEvents(true);

    setWidth(20);
    setHeight(32);
}


void
GUI_TL2_VerticalScrollBarHandle::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _hover = true;
    event->accept();
    
    update();
}


void
GUI_TL2_VerticalScrollBarHandle::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _hover = false;
    event->accept();

    update();
}



void
GUI_TL2_VerticalScrollBarHandle::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    // RLP_LOG_DEBUG("VerticalScrollBarHandle");

    event->accept();

    _dragPosG = event->globalPosition().y();
    _dragPos = position().y(); // event->pos().y();
}


void
GUI_TL2_VerticalScrollBarHandle::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
}


void
GUI_TL2_VerticalScrollBarHandle::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")
    // RLP_LOG_DEBUG(event->pos().y() << " " << event->globalPosition().y() << " " << _dragPos)

    event->accept();
    
    qreal yposDelta = event->globalPosition().y() - _dragPosG;
    qreal npos = _dragPos + yposDelta;

    float posPercent = 1 - (npos / (_vbar->height() - 40)); // account for height of bar plus offset

    // RLP_LOG_DEBUG(npos << posPercent)

    if ((posPercent >= 0) && (posPercent <= 1.0)) {

        _posPercent = posPercent;
        setPos(20, npos);
        emit handlePosChanged(_posPercent);
    }
}


void
GUI_TL2_VerticalScrollBarHandle::refresh()
{
    return;
}


QRectF
GUI_TL2_VerticalScrollBarHandle::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_VerticalScrollBarHandle::paint(GUI_Painter* painter)
{
    QPen p(_vbar->colForeground());

    QColor colBg = _vbar->colBackground();
    if (_hover) {
        colBg = _vbar->colHighlight();
    }

    QBrush b(colBg);

    painter->setPen(p);
    painter->setBrush(b);
    painter->drawRect(boundingRect());

    // Button handle
    //

    // QBrush fgBrush(_vbar->colForeground());
    // painter->setBrush(fgBrush);

    // painter->drawRect(boundingRect());
}


// -------


const int GUI_TL2_VerticalScrollBar::AREA_WIDTH = 40;

GUI_TL2_VerticalScrollBar::GUI_TL2_VerticalScrollBar(GUI_TL2_View* view):
    GUI_ItemBase(view),
    _view(view),
    _colBackground(QColor(35, 35, 35)),
    _colForeground(QColor(65, 65, 65)),
    _colHighlight(QColor(75, 75, 75))

{
    setWidth(GUI_TL2_VerticalScrollBar::AREA_WIDTH);
    setHeight(view->timelineHeight());

    _handle = new GUI_TL2_VerticalScrollBarHandle(this);
    _handle->setPos(15, 0);
    connect(_handle, SIGNAL(handlePosChanged(float)), this, SLOT(onHandlePosChanged(float)));
}


void
GUI_TL2_VerticalScrollBar::onHandlePosChanged(float pos)
{
    //RLP_LOG_DEBUG(pos);
    
    emit scrollPosChanged(pos);
}



void
GUI_TL2_VerticalScrollBar::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nheight << _view->timelineHeight())
    // RLP_LOG_DEBUG("width:" << width() << "height:" << height())

    setHeight(nheight); // _view->timelineHeight());
    setPos(nwidth - GUI_TL2_VerticalScrollBar::AREA_WIDTH - 5, 0); // 15);

    if (_handle->posPercent() == 0)
    {
        _handle->setPos(15, nheight - 35);
    }
    _handle->refresh();
}



void
GUI_TL2_VerticalScrollBar::updatePos()
{
    // RLP_LOG_WARN("Doing nothing?")

    // setPos(_view->width() - _handle->width() - 2, 50);
    // _handle->refresh();
}




QRectF
GUI_TL2_VerticalScrollBar::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_VerticalScrollBar::paint(GUI_Painter* painter)
{
    // painter->setPen(QPen(_colForeground));
    // painter->setBrush(QBrush(Qt::red));
    // painter->drawRect(boundingRect());

    // painter->drawRect(boundingRect());

    painter->setBrush(QBrush(QColor(35, 35, 35)));
    painter->drawRect(7, height() - 30, 10, height());
}
