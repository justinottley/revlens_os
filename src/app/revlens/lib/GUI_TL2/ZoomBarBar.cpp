
#include "RlpRevlens/GUI_TL2/ZoomBarBar.h"

#include "RlpRevlens/GUI_TL2/ZoomBar.h"
#include "RlpRevlens/GUI_TL2/ZoomBarDragButton.h"

#include "RlpRevlens/GUI_TL2/View.h"


RLP_SETUP_LOGGER(revlens, GUI_TL2, ZoomBarBar)

GUI_TL2_ZoomBarBar::GUI_TL2_ZoomBarBar(GUI_ItemBase* bar):
    GUI_ItemBase(bar),
    _zoomBar(qobject_cast<GUI_TL2_ZoomBar*>(bar)),
    _inDrag(false)
{
    setHeight(20);
    setZValue(4);
    // setParentItem(bar);
}


float
GUI_TL2_ZoomBarBar::xpos() const
{
    //return _zoomBar->btnDragLeft()->xpos() + _zoomBar->btnDragLeft()->width();
    return _zoomBar->btnDragLeft()->x() + _zoomBar->btnDragLeft()->width();
}


/*
float
GUI_TL2_ZoomBarBar::width() const
{
    return _zoomBar->btnDragRight()->xpos() - xpos(); // - 2;
}
*/


void
GUI_TL2_ZoomBarBar::refresh()
{
    qreal rxpos = _zoomBar->btnDragRight()->position().x();
    qreal nwidth = rxpos - xpos();

    //  RLP_LOG_DEBUG("xpos:" << xpos() << "rpos:" << rxpos << "nwidth:" << nwidth)

    setPos(xpos(), 0);
    setWidth(nwidth);
}


void
GUI_TL2_ZoomBarBar::mousePressEvent(QMouseEvent* event)
{
    event->accept();

    _inDrag = true;
    _dragPos = position().x();
    _dragPosG = event->globalPosition().x();

    _leftPos = _zoomBar->btnDragLeft()->x(); // pos(false);
    _rightPos = _zoomBar->btnDragRight()->x(); // pos(false);
}


void
GUI_TL2_ZoomBarBar::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("");

    event->accept();
    _inDrag = false;

    _zoomBar->view()->setPanStart(_zoomBar->view()->pan());
}


void
GUI_TL2_ZoomBarBar::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")
    
    event->accept();

    if (_inDrag) {

        int xposDelta = event->globalPosition().x() - _dragPosG;
        qreal tlWidth = _zoomBar->view()->timelineWidth();
        qreal xposMax = _rightPos + xposDelta + GUI_TL2_ZoomBarDragButton::BUTTON_SIZE;

        // RLP_LOG_DEBUG("xposDelta:" << xposDelta << "leftPos:" << _leftPos << "rightPos:" << _rightPos << "tlwidth:" << tlWidth)

        if (((_leftPos + xposDelta) >= 0) && (xposMax < tlWidth))
        {

            _zoomBar->btnDragLeft()->setPos(_leftPos + xposDelta, 0);

            // to ensure the right button doesn't clamp to the end on refresh..
            _zoomBar->btnDragRight()->setXPosDelta(xposDelta);
            _zoomBar->btnDragRight()->setPos(_rightPos + xposDelta, 0);

            emit barMoved(xposDelta);

            refresh();
        }
    }
}


QRectF
GUI_TL2_ZoomBarBar::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_ZoomBarBar::paint(GUI_Painter* painter)
{

    QBrush brush(_zoomBar->colBackground());
    QPen pen(_zoomBar->colBackground());

    painter->setBrush(brush);
    painter->setPen(pen);
    
    painter->drawRect(boundingRect());
}
