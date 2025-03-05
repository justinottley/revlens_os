

#include "RlpRevlens/GUI_TL2/Splitter.h"

#include "RlpRevlens/GUI_TL2/View.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, Splitter)

const int GUI_TL2_Splitter::BUTTON_WIDTH = 10;

GUI_TL2_Splitter::GUI_TL2_Splitter(GUI_ItemBase* view):
    GUI_ItemBase(view),
    _view(qobject_cast<GUI_TL2_View*>(view)),
    _hover(false)
{
    setWidth(GUI_TL2_Splitter::BUTTON_WIDTH);
    setHeight(_view->timelineHeight() - 1);

    setZValue(10);
    setAcceptHoverEvents(true);
}


void
GUI_TL2_Splitter::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG("")

    setHeight(_view->timelineHeight() - 1);
}


void
GUI_TL2_Splitter::hoverEnterEvent(QHoverEvent* event)
{
    _hover = true;
    
    update();
}


void
GUI_TL2_Splitter::hoverLeaveEvent(QHoverEvent* event)
{
    _hover = false;

    update();
}


void
GUI_TL2_Splitter::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("GUI_TL2_Splitter");

    event->accept();
    _inDrag = true;
    _dragPosG = event->globalPosition().x(); // event->pos().x(); //  - _xpos;
    _dragPos = position().x();
}


void
GUI_TL2_Splitter::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("GUI_TL2_Splitter");

    event->accept();
    _inDrag = false;
}


void
GUI_TL2_Splitter::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("");

    event->accept();

    if (_inDrag) {

        qreal xposDelta = event->globalPosition().x() - _dragPosG;

        qreal npos = _dragPos + xposDelta;
        setPos(npos, 0);

        emit splitterMoved(npos);
    }
}


void
GUI_TL2_Splitter::resizeEvent(QResizeEvent* event)
{
    std::cout << "GUI_TL2_Splitter::resizeEvent()" << std::endl;
}


QRectF
GUI_TL2_Splitter::boundingRect() const
{
    return QRectF(0, 0, width(), height()); // _view->height());
}




void
GUI_TL2_Splitter::paint(GUI_Painter* painter)
{
    QColor col = QColor(50, 50, 50);
    if (_hover) {
        col = QColor(80, 80, 80);
    }
    painter->setPen(Qt::black);
    painter->setBrush(QBrush(col));
    painter->drawRect(boundingRect());
}