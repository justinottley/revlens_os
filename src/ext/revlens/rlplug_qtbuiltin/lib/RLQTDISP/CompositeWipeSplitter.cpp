
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"

#include "RlpExtrevlens/RLQTDISP/CompositeWipeSplitter.h"


RLP_SETUP_LOGGER(ext, RLQTDISP, CompositeWipeSplitter)

RLQTDISP_CompositeWipeSplitter::RLQTDISP_CompositeWipeSplitter(DISP_GLView* parent):
    GUI_ItemBase(parent),
    _display(parent),
    _enabled(true),
    _inHover(false),
    _inDrag(false)
{
    setAcceptHoverEvents(true);
    setWidth(6);
    setHeight(_display->height());
}



void
RLQTDISP_CompositeWipeSplitter::mousePressEvent(QMouseEvent *event)
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
RLQTDISP_CompositeWipeSplitter::mouseReleaseEvent(QMouseEvent* event)
{
    if (!_enabled)
    {
        return;
    }

    _inDrag = false;

    // TODO FIXME: At the moment need to blow the lookahead to get metadata reset
    //

    CNTRL_MainController* controller = CNTRL_MainController::instance();
    CNTRL_Video* cv = controller->getVideoManager();

    qlonglong cframe = cv->currentFrameNum();

    RLP_LOG_DEBUG("Current frame:" << cframe)

    cv->clearAll();
    cv->updateToFrame(cframe, true);
}


void
RLQTDISP_CompositeWipeSplitter::mouseMoveEvent(QMouseEvent* event)
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
RLQTDISP_CompositeWipeSplitter::hoverEnterEvent(QHoverEvent *event)
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
RLQTDISP_CompositeWipeSplitter::hoverLeaveEvent(QHoverEvent* event)
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
RLQTDISP_CompositeWipeSplitter::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    setHeight(nheight);
}


QRectF
RLQTDISP_CompositeWipeSplitter::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
RLQTDISP_CompositeWipeSplitter::paint(GUI_Painter* painter)
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
