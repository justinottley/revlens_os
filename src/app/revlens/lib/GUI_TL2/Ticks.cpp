
#include "RlpRevlens/GUI_TL2/Ticks.h"
#include "RlpRevlens/GUI_TL2/Playhead.h"
#include "RlpRevlens/GUI_TL2/View.h"


RLP_SETUP_LOGGER(revlens, GUI_TL2, Ticks)

GUI_TL2_Ticks::GUI_TL2_Ticks(GUI_TL2_View* view):
    GUI_ItemBase(view),
    _view(view),
    _tickHeight(6),
    _heightPolicy(HEIGHT_FULL),
    _linesVisible(false),
    _numVisible(false)
{
    setZValue(4);
}


int
GUI_TL2_Ticks::tickHeight() const
{
     if (_heightPolicy == HEIGHT_FIXED) {
        return _tickHeight;

    } else {
        return _view->timelineHeight();
    }
}


void
GUI_TL2_Ticks::refresh()
{
    setWidth(_view->timelineWidth() - 1);
    setHeight(_view->timelineHeight() - 1);
    setPos(_view->timelineXPosStart(), 0);
}


void
GUI_TL2_Ticks::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << nheight)

    refresh();
}


QRectF
GUI_TL2_Ticks::boundingRect() const
{
    return QRectF(0, 0, height(), width()); 
}


void
GUI_TL2_Ticks::paint(GUI_Painter* painter)
{
    if ((!_linesVisible) && (!_numVisible))
    {
        return;
    }

    // RLP_LOG_DEBUG("")

    painter->save();

    // painter->setRenderHint(QPainter::Antialiasing, true);

    // QColor tickColor(100, 100, 100);
    QColor tickColor(70, 70, 70);
    // QColor tickColor(160, 60, 60);

    QColor textColor(180, 180, 180);

    QPen textPen(textColor);
    QPen tpen = painter->pen();
    tpen.setWidth(1);
    
    QFont f = painter->font();
    f.setPixelSize(8);
    painter->setFont(f);


    tpen.setColor(tickColor);

    QBrush tickBrush = QBrush(tickColor, Qt::SolidPattern);
    // painter->setPen(tpen);
    painter->setBrush(tickBrush);

    float lastXPos = -11;
    float minSpacing = 10;
    float xposMax = _view->width();
    int ypos = 0;
    int theight = tickHeight();

    float lastTextXPos = -31;
    float minTextSpacing = 30;

    qlonglong frameStart = _view->frameStart();
    qlonglong frameEnd = _view->frameEnd();
    qlonglong currMouseFrame = _view->playhead()->currMouseFrame();

    bool playheadVisible = _view->playhead()->isVisible();

    for (qlonglong x = frameStart; x <= frameEnd; ++x) {

        float xpos = _view->getXPosAtFrame(x);

        // RLP_LOG_DEBUG("x: " << x << " xpos: " << xpos << " frame: " << _view->getFrameAtXPos(xpos))

        float xDiff = xpos - lastXPos;
        float textXDiff = xpos - lastTextXPos;
        
        // RLP_LOG_DEBUG(x << "xpos:" << xpos << "xdiff:" << xDiff << "lastXpos:" << lastXPos << "textXDiff:" << textXDiff)

        if ((xpos != lastXPos) && (xDiff >= minSpacing) && (xpos <= xposMax)) {

            // RLP_LOG_DEBUG("tick: frame: " << x << " " << xpos << " max: " << xposMax << " zoom: " << _view->zoom() << " ppf: " << _view->pixelsPerFrame());

            // painter->drawRect(xpos, ypos, 1, tickHeight);
            if (_linesVisible) {
                painter->setPen(tpen);
                painter->drawLine(xpos, ypos, xpos, ypos + theight);
            }

            if ((_numVisible) &&
                (x <= frameEnd) &&
                (textXDiff > minTextSpacing))
            {
                painter->setPen(textPen);

                if (x == currMouseFrame) {
                    if (!playheadVisible) {
                        painter->drawText(xpos + 4, 24, QString("%1").arg(x));
                    }
                } else
                {
                    painter->drawText(xpos + 4, 24, QString("%1").arg(x));
                }

                lastTextXPos = xpos;
            }

            lastXPos = xpos;
        }
    }

    painter->restore();
}