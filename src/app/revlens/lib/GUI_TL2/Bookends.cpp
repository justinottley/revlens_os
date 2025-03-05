
#include "RlpRevlens/GUI_TL2/Bookends.h"
#include "RlpRevlens/GUI_TL2/View.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, Bookends)

GUI_TL2_Bookends::GUI_TL2_Bookends(GUI_TL2_View* view):
    GUI_ItemBase(view),
    _view(view)
{
    setZValue(6);

    _btnDragLeft = new GUI_TL2_BookendDragButton(
        GUI_TL2_BookendDragButton::BTN_LEFT,
        this);
    _btnDragLeft->setPosFrame(view->frameStart());

    connect(
        _btnDragLeft,
        SIGNAL(buttonFrameChanged(int, qlonglong)),
        this,
        SLOT(onButtonFrameChanged(int, qlonglong))
    );

    _btnDragRight = new GUI_TL2_BookendDragButton(
        GUI_TL2_BookendDragButton::BTN_RIGHT,
        this);

    connect(
        _btnDragRight,
        SIGNAL(buttonFrameChanged(int, qlonglong)),
        this,
        SLOT(onButtonFrameChanged(int, qlonglong))
    );

    _btnDragRight->setPosFrame(view->frameCount());
}


void
GUI_TL2_Bookends::refresh()
{
    // RLP_LOG_DEBUG("")

    setWidth(_view->timelineWidth() + (GUI_TL2_BookendDragButton::BUTTON_WIDTH * 2));
    setHeight(_view->timelineHeight() - 1);
    setPos(_view->timelineXPosStart() - GUI_TL2_BookendDragButton::BUTTON_WIDTH - 1, 0); // 10: width of bookend drag button

    _btnDragLeft->setPosFrame(_btnDragLeft->posFrame()); // setXPos(_btnDragLeft->getPosAtFrame(_btnDragLeft->posFrame()));
    _btnDragRight->setPosFrame(_btnDragRight->posFrame());
}


void
GUI_TL2_Bookends::onZoomChanged()
{
    // RLP_LOG_DEBUG("")

    refresh();
    update();
}


void
GUI_TL2_Bookends::onPanChanged()
{
    refresh();
}


void
GUI_TL2_Bookends::onButtonFrameChanged(int side, qlonglong frame)
{
    RLP_LOG_DEBUG(side << " " << frame);

    if (side == (int)GUI_TL2_BookendDragButton::BTN_LEFT) {

        RLP_LOG_DEBUG("re-emitting inFrameChanged");

        emit inFrameChanged(frame);

    }  else if (side == (int)GUI_TL2_BookendDragButton::BTN_RIGHT) {

        RLP_LOG_DEBUG("re-emitting outFrameChanged");

        emit outFrameChanged(frame);
    }
}


QRectF
GUI_TL2_Bookends::boundingRect() const
{
    return QRectF(0, 0, width(), height()); // _view->timelineWidth(), _view->height()); // _view->timelineHeight());
}


void
GUI_TL2_Bookends::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    refresh();

    //setWidth(nwidth);
    //setHeight(nheight);

    _btnDragLeft->onParentSizeChanged(nwidth, nheight);
    _btnDragRight->onParentSizeChanged(nwidth, nheight);
}


void
GUI_TL2_Bookends::paint(GUI_Painter* painter)
{
    // grey outside area bypassed
    // return;

    painter->save();
    //painter->setClipRegion(_view->timelineClipFullHeightRegion());
    
    QColor outsideCol(100, 100, 100);
    painter->setBrush(QBrush(outsideCol));
    painter->setPen(QPen(QColor(40, 40, 40)));
    
    int frameStartXPos = 0; // _view->getXPosAtFrame(_view->frameStart()) - buttonWidth();
    int bookendLXPos = _btnDragLeft->xpos();
    int frameEndXPos = _view->getXPosAtFrame(_view->frameEnd());
    int bookendRXPos = _btnDragRight->xpos();

    // RLP_LOG_DEBUG(frameEndXPos << " " << bookendRXPos)

    QRectF leftOutsideRect(frameStartXPos, 0, (bookendLXPos - frameStartXPos), _view->height()); // _view->timelineHeight());
    QRectF rightOutsideRect(bookendRXPos, 0, (frameEndXPos - bookendRXPos + buttonWidth()), _view->height()); //  _view->timelineHeight());

    painter->setOpacity(0.8);
    if (bookendLXPos > frameStartXPos) {
        painter->drawRect(leftOutsideRect);
    }
    
    painter->drawRect(rightOutsideRect);
    painter->restore();
    
}