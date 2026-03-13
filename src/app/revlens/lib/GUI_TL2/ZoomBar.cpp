

#include "RlpRevlens/GUI_TL2/ZoomBar.h"

#include "RlpRevlens/GUI_TL2/View.h"


RLP_SETUP_LOGGER(revlens, GUI_TL2, ZoomBar)

GUI_TL2_ZoomBar::GUI_TL2_ZoomBar(GUI_ItemBase* view):
    GUI_ItemBase(view),
    _view(qobject_cast<GUI_TL2_View*>(view))
{
    setHeight(20);
    setZValue(7);
    
    _bar = new GUI_TL2_ZoomBarBar(this);
    _btnDragLeft = new GUI_TL2_ZoomBarDragButton(GUI_TL2_ZoomBarDragButton::BTN_LEFT, this);
    _btnDragRight = new GUI_TL2_ZoomBarDragButton(GUI_TL2_ZoomBarDragButton::BTN_RIGHT, this);

    // _bar->setVisible(false);
    // _btnDragLeft->setVisible(false);
    // _btnDragRight->setVisible(false);

    connect(_btnDragLeft,
        SIGNAL(buttonMoved(int, float)),
        this,
        SLOT(onButtonMoved(int, float)));

    connect(_btnDragRight,
        SIGNAL(buttonMoved(int, float)),
        this,
        SLOT(onButtonMoved(int, float)));
    
    connect(_bar,
        SIGNAL(barMoved(int)),
        this,
        SLOT(onBarMoved(int)));
}



void
GUI_TL2_ZoomBar::onButtonMoved(int side, float xpos)
{
    // RLP_LOG_DEBUG(side << xpos)

    int maxWidth = _view->timelineWidth();
    int leftXPos = _btnDragLeft->position().x(); // xpos(false);
    int rightXPos = maxWidth + _btnDragRight->position().x(); // xpos(false);

    // float zoom = (float)(maxWidth - 1) / (float)(_bar->width() +  (_btnDragLeft->width() * 2));
    float zoom = (float)(maxWidth) / (float)(_bar->width() + (_btnDragLeft->width() * 2));

    // RLP_LOG_DEBUG("max width:" << maxWidth << "bar width: " << _bar->width() << " btnDragLeftwidth: " << (_btnDragLeft->width() * 2));

    emit zoomChanged(zoom, side, leftXPos, rightXPos);
}


void
GUI_TL2_ZoomBar::onBarMoved(int xDiff)
{
    // RLP_LOG_DEBUG(xDiff)
    emit panChanged(GUI_TL2_ZoomBarDragButton::BTN_NONE, xDiff);
}


void
GUI_TL2_ZoomBar::refresh()
{
    // RLP_LOG_DEBUG("")

    setWidth(_view->timelineWidth() - 1); // nwidth);
    setPos(_view->timelineXPosStart() + 1, position().y()); // return QRectF(xpos(), 0, width(), height()); // height());)

    // _btnDragLeft->refresh();
    _btnDragRight->refresh();
    _bar->refresh();
}


void
GUI_TL2_ZoomBar::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight << "pos:" << position())
    // setWidth(_view->timelineWidth() - 1); // nwidth);

    // setPos(0, 10);

    setPos(_view->timelineXPosStart(), nheight - height() - 10);// - 32); // why 32??

    refresh();
}


QRectF
GUI_TL2_ZoomBar::boundingRect() const
{
    return QRectF(
        0, // _view->timelineXPosStart(), // lpos.x(),
        0,
        width(), // _view->timelineWidth() - 1,
        height());
}


void
GUI_TL2_ZoomBar::paint(GUI_Painter* painter)
{
    QRectF br = boundingRect();
    // RLP_LOG_DEBUG(br)

    // TODO FIXME: ZoomBar Bar disabled
    QBrush brush(Qt::black); // (Qt::red); // black); // QColor(50, 150, 50);

    painter->setBrush(brush);
    painter->setPen(Qt::black);
    painter->drawRect(boundingRect());
}