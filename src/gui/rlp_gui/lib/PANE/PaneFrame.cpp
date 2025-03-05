
#include "RlpGui/PANE/PaneFrame.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/PANE/SvgButton.h"



RLP_SETUP_LOGGER(gui, GUI, PaneFrame)

GUI_PaneFrame::GUI_PaneFrame(int width, int height):
    GUI_FrameBase(nullptr, "", O_VERTICAL),
    _borderSize(5),
    _overlay(nullptr),
    _bgCol(GUI_Style::BgDarkGrey),
    _text(""),
    _textBold(false),
    _isPersistent(false)
{
    setSize(QSizeF(width, height));

    _headerHeight = 25;


    setAcceptHoverEvents(true);
    //setFlag(QGraphicsItem::ItemIsMovable, true);
    //setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    // setZValue(20);

    // NOTE: this object needs to be added to the scene first before
    // initPane() is called..
}


void
GUI_PaneFrame::initPane()
{
    // RLP_LOG_DEBUG("")

    _btnClose = new GUI_SvgButton(
        ":/feather/lightgrey/x-circle.svg",
        this
    );
    _btnClose->setPos(width() - _btnClose->width() - 10, 6);
    
    connect(
        _btnClose,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_PaneFrame::onCloseRequested
    );

    _pane = new GUI_Pane(this);
    _pane->setPos(_borderSize, _borderSize + _headerHeight);
    _pane->setOrientation(O_VERTICAL);
    _pane->initPane();
    _pane->setHeaderVisible(false);
    _pane->body()->setPaintBackground(false);
    _pane->setSize(size());

    // Not needed?
    // p->setFlag(QGraphicsItem::ItemIsMovable, true);
    // p->setFlag(QGraphicsItem::ItemIsSelectable, true);
}


void
GUI_PaneFrame::setCloseButtonVisible(bool isVisible)
{
    if (isVisible)
    {
        _btnClose->setVisible(true);
    }
    else
    {
        _btnClose->setVisible(false);
    }
}


void
GUI_PaneFrame::setWidth(qreal width)
{
    GUI_FrameBase::setWidth(width);

    _btnClose->setPos(width - _btnClose->width() - 5, 8);
}


void
GUI_PaneFrame::setBgCol(QColor col)
{
    _bgCol = col;
    _pane->setBgColor(col);
}


void
GUI_PaneFrame::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    _dragStartPosG = event->globalPosition();
    _dragStartPos = position();
}



void
GUI_PaneFrame::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    QPointF newPos = _dragStartPosG - event->globalPosition();
    QPointF pos = _dragStartPos - newPos;

    setPos(pos.x(), pos.y());
}


void
GUI_PaneFrame::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    GUI_FrameBase::mouseReleaseEvent(event);
}


void
GUI_PaneFrame::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")
}


void
GUI_PaneFrame::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")
}


void
GUI_PaneFrame::onCloseRequested(QVariantMap /* metadata */)
{
    // RLP_LOG_DEBUG("")

    if (_overlay != nullptr)
    {
        scene()->removeItem(_overlay);
    }

    if (_isPersistent)
    {
        hideItem();
        return;
    }


    GUI_Scene* gscene = qobject_cast<GUI_Scene*>(scene());

    gscene->requestFloatingPaneClosed();

    scene()->removeItem(this);

    gscene->requestUpdate();
}


QRectF
GUI_PaneFrame::boundingRect() const
{
    return QRectF(
        0, 0, width(), height());

        // (width() + (_borderSize * 2) - 2), (height() + _headerHeight + (_borderSize * 2) - 2));
}


void
GUI_PaneFrame::paint(GUI_Painter* painter)
{
    // qInfo() << boundingRect();

    painter->setPen(GUI_Style::BgLightGrey);
    painter->setBrush(_bgCol);

    painter->drawRect(boundingRect());

    if (_text.size() > 0)
    {
        painter->setPen(Qt::white);

        painter->save();
        QFont f = painter->font();
        f.setPixelSize(GUI_Style::FontPixelSizeDefault + 6);
        painter->setFont(f);
        painter->drawText(10, 22, _text);
        painter->restore();
    }

}