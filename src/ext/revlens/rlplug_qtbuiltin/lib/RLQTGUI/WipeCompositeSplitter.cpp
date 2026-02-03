
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"

#include "RlpExtrevlens/RLQTGUI/WipeCompositeSplitter.h"


RLP_SETUP_LOGGER(ext, RLQTGUI, WipeCompositeSplitter)

RLQTGUI_WipeCompositeSplitter::RLQTGUI_WipeCompositeSplitter(DISP_GLView* parent):
    GUI_ItemBase(parent),
    _display(parent),
    _enabled(true),
    _inHover(false),
    _inDrag(false),
    _posPercentX(-1)
{
    setAcceptHoverEvents(true);
    setWidth(6);
    setHeight(_display->height());
}


void
RLQTGUI_WipeCompositeSplitter::setPosPercent(float percent, bool emitSignal)
{
     _posPercentX = percent;
     int xpos = rint(percent * _display->width());
     setPos(xpos, 0);

     RLP_LOG_DEBUG(percent << xpos)

     if (emitSignal)
     {
        emit posChanged(_posPercentX);
     }

     update();
}


void
RLQTGUI_WipeCompositeSplitter::mousePressEvent(QMouseEvent *event)
{
    if (!_enabled)
    {
        return;
    }

    _dragPos = position().x();
    _dragPosG = event->globalPosition().x();

    _inDrag = true;
}


void
RLQTGUI_WipeCompositeSplitter::mouseReleaseEvent(QMouseEvent* event)
{
    if (!_enabled)
    {
        return;
    }

    _inDrag = false;

    // TODO FIXME: At the moment need to blow the lookahead to get metadata reset
    //

    // CNTRL_MainController* controller = CNTRL_MainController::instance();
    // CNTRL_Video* cv = controller->getVideoManager();

    // qlonglong cframe = cv->currentFrameNum();

    // RLP_LOG_DEBUG("Current frame:" << cframe)

    // cv->clearAll();
    // cv->updateToFrame(cframe, true);
}


void
RLQTGUI_WipeCompositeSplitter::mouseMoveEvent(QMouseEvent* event)
{
    if (!_enabled)
    {
        return;
    }

    if (_inDrag)
    {

        // NOTE: Need to use scenePos to get stable (non jittery / recursive positioning / blinking)
        // behavior
        //
        
        qreal diff = event->globalPosition().x() - _dragPosG;
        qreal pos = _dragPos + diff;

        // RLP_LOG_DEBUG(pos)

        setPos(pos, 0);
        _posPercentX = pos / (float)_display->width();

        // RLP_LOG_DEBUG("pos percent:" << _posPercentX)
        // setPos(pos, 0);
        // updatePos();

        emit posChanged(_posPercentX);
    }
}

void
RLQTGUI_WipeCompositeSplitter::hoverEnterEvent(QHoverEvent *event)
{
    if (!_enabled)
    {
        return;
    }

    _inHover = true;

    QApplication::setOverrideCursor(Qt::SizeHorCursor);

    update();
}


void
RLQTGUI_WipeCompositeSplitter::hoverLeaveEvent(QHoverEvent* event)
{
    if (!_enabled)
    {
        return;
    }

    QApplication::restoreOverrideCursor();

    event->setAccepted(true);
    _inHover = false;

    update();
}


void
RLQTGUI_WipeCompositeSplitter::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    setHeight(nheight);
}


QRectF
RLQTGUI_WipeCompositeSplitter::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
RLQTGUI_WipeCompositeSplitter::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")
    if (!_enabled)
    {
        return;
    }

    painter->setPen(GUI_Style::BgLightGrey);
    if (_inHover)
    {
        painter->setBrush(Qt::gray);
    } else
    {
        painter->setBrush(GUI_Style::BgLightGrey);
    }

    int width = 2;
    if (_inHover)
    {
        width = 5;
    }

    QRectF srect(0, 0, width, _display->height());
    painter->drawRect(srect);
}
