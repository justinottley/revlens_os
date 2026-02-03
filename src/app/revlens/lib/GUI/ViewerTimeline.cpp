
#include "RlpRevlens/GUI/ViewerTimeline.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(revlens, GUI, ViewerTimeline)

static const int X_OFFSET = 25;

GUI_ViewerTimeline::GUI_ViewerTimeline():
    DS_EventPlugin("ViewerTimeline", false),
    _fnumStyle(FNUM_SESSION),
    _chover(false),
    _lhover(false),
    _lxpos(0),
    _hmid(0),
    _wmax(0),
    _windowWidth(20),
    _hframe(0),
    _frameIn(-1),
    _frameOut(-1),
    _fgcol(QColor(250, 150, 150))
{
    #ifdef SCAFFOLD_IOS
    _lhover = true;
    _chover = true;
    #endif

    _controller = CNTRL_MainController::instance();

    connect(
        this,
        SIGNAL(startScrub(qlonglong)),
        _controller->getVideoManager(),
        SLOT(onScrubStart(qlonglong))
    );

    connect(
        this,
        SIGNAL(updateToFrame(qlonglong)),
        _controller->getVideoManager(),
        SLOT(updateToFrameAsync(qlonglong))
    );

    connect(
        this,
        SIGNAL(updateToFrame(qlonglong)),
        _controller->getAudioManager(),
        SLOT(updateToFrame(qlonglong))
    );

    connect(
        this,
        SIGNAL(endScrub(qlonglong)),
        _controller->getVideoManager(),
        SLOT(onScrubEnd(qlonglong))
    );

    connect(
        this,
        SIGNAL(endScrub(qlonglong)),
        _controller->getAudioManager(),
        SLOT(onScrubEnd(qlonglong))
    );
}


DS_EventPlugin*
GUI_ViewerTimeline::duplicate()
{
    GUI_ViewerTimeline* pl = new GUI_ViewerTimeline();
    pl->setMainController(_controller);
    return pl;
}


qlonglong
GUI_ViewerTimeline::getFrameNum(int xpos)
{
    float pos = (xpos - X_OFFSET) / (float)_wmax;

    if (_frameIn >= 0)
    {
        return ((_frameOut - _frameIn) * pos) + _frameIn;
    }

    return _frameCount * pos;
}


void
GUI_ViewerTimeline::setFrameInOut(qlonglong frameIn, qlonglong frameOut)
{
    RLP_LOG_DEBUG(frameIn << "-" << frameOut)

    _frameIn = frameIn;
    _frameOut = frameOut;
    _frameCount = frameOut - frameIn + 1;
}


bool
GUI_ViewerTimeline::mousePressEventInternal(QMouseEvent* event)
{
    int xpos = event->position().x();
    int ypos = event->position().y();

    QRectF br = timelineBoundingRect();
    if (!br.contains((qreal)xpos, (qreal)ypos))
    {
        // RLP_LOG_DEBUG("OUTSIDE")
        return false;
    }

    // if (!_lhover)
    // {
    //     return false;
    // }

    if (_lhover)
    {
        qlonglong frameNum = getFrameNum(xpos);
        RLP_LOG_DEBUG(frameNum)

        #ifdef SCAFFOLD_IOS
        _hframe = frameNum;
        #endif

        emit startScrub(frameNum);

    }
    

    return true;
}


bool
GUI_ViewerTimeline::mouseMoveEventInternal(QMouseEvent* event)
{
    int xpos = event->position().x();
    int ypos = event->position().y();

    QRectF br = timelineBoundingRect();
    if (!br.contains((qreal)xpos, (qreal)ypos))
    {
        return false;
    }


    if (!_chover)
    {
        return false;
    }

    if (xpos >= INT_MAX)
    {
        // RLP_LOG_ERROR("invalid x pos")
        return false;
    }

    // RLP_LOG_DEBUG(event->position().x())

    qlonglong frameNum = getFrameNum(xpos);

    _hframe = frameNum;

    // RLP_LOG_DEBUG(frameNum)

    emit updateToFrame(frameNum);

    return true;
}


bool
GUI_ViewerTimeline::mouseReleaseEventInternal(QMouseEvent* event)
{
    int xpos = event->position().x();
    int ypos = event->position().y();

    QRectF br = timelineBoundingRect();
    if (!br.contains((qreal)xpos, (qreal)ypos))
    {
        return false;
    }


    if (_chover || _lhover)
    {
        qlonglong frameNum = getFrameNum(xpos);

        RLP_LOG_DEBUG(frameNum)

        #ifdef SCAFFOLD_IOS
        _hframe = frameNum;
        #endif

        emit endScrub(frameNum);

    }

    return true;
}


bool
GUI_ViewerTimeline::hoverMoveEventInternal(QHoverEvent* event)
{
    #ifdef SCAFFOLD_IOS
    return true;
    #endif

    _display->setCursor(Qt::CrossCursor);

    int xpos = event->position().x();
    int ypos = event->position().y();

    QRectF br = timelineBoundingRect();
    if (!br.contains((qreal)xpos, (qreal)ypos))
    {
        // RLP_LOG_DEBUG("OUTSIDE")
        return false;
    }

    // RLP_LOG_DEBUG(ypos << _hmid)

    bool chover = _chover;

    if ((ypos > _hmid - 10) && (ypos < _hmid + 10) && 
        (xpos > _circpos - 10) && (xpos < _circpos + 10))
    {
        _chover = true;
    } else
    {
        _chover = false;
    }

    bool lhover = _lhover;

    if (!_chover)
    {
        
        if ((ypos > _hmid - 6) && (ypos < _hmid + 6) &&
            (xpos > 30) && (xpos < _wmax))
        {
            _lhover = true;
            _chover = true;
            _lxpos = xpos;

        } else
        {
            _lhover = false;
        }
    }

    if (_chover || _lhover)
    {
        _hframe = getFrameNum(xpos);
    }

    if ((chover != _chover) || _lhover || (_lhover != lhover))
    {
        _display->update();
    }

    return true;
}


QRectF
GUI_ViewerTimeline::timelineBoundingRect()
{
    return QRectF(10, _hmid - 20, _windowWidth - 20, 40);
}


void
GUI_ViewerTimeline::paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata)
{
    if (!_enabled)
    {
        return;
    }


    qreal wx = metadata.value("window.width").value<qreal>();
    qreal wy = metadata.value("window.height").value<qreal>();

    _currframe = metadata.value("video.currframe").toLongLong();
    #ifdef SCAFFOLD_IOS
    _hframe = _currframe;
    #endif

    float pos = 0;
    if (_frameIn < 0)
    {
        _frameCount = _controller->session()->frameCount();
        pos = (float)_currframe / (float)_frameCount;
    } else
    {
        pos = float(_currframe - _frameIn) / _frameCount;
    }

    
    int wmax = wx - 70;
    _circpos = X_OFFSET + (pos * wmax);

    // RLP_LOG_DEBUG(_currframe << _frameCount << pos << _circpos)

    painter->save();

    _hmid = wy - 80; // 100;
    _wmax = wmax;
    _windowWidth = wx;

    QPen bp(GUI_Style::BgDarkGrey);
    QBrush bb(GUI_Style::BgDarkGrey);

    QRectF bgr(10, _hmid - 20, wx - 20, 40);

    painter->setPen(bp);
    painter->setBrush(bb);
    painter->setOpacity(0.6);
    painter->drawRect(bgr);

    QPen p(QColor(220, 220, 220));
    // QRectF circ(_circpos, _hmid - 5, 10.0, 10.0);

    QBrush wb(QColor(220, 220, 220));

    p.setWidth(2);
    painter->setPen(p);
    painter->setBrush(wb);

    painter->setOpacity(1.0);

    if (_lhover)
    {
        painter->setPen(_fgcol);
        painter->setBrush(QBrush(_fgcol));
    } else {
        painter->setPen(p);
        painter->setBrush(wb);
    }

    painter->drawLine(30, _hmid, wx - 40, _hmid);

    if (_chover)
    {
        painter->setPen(_fgcol);
        painter->setBrush(QBrush(_fgcol));
    } else {
        painter->setPen(p);
        painter->setBrush(wb);
    }
    painter->drawEllipse(_circpos, _hmid - 1, 10.0, 10.0);

    if (_chover || _lhover)
    {
        QString ctext;
        if (_fnumStyle == FNUM_SESSION)
        {
            ctext = QString("%1").arg(_hframe);

        } else if (_fnumStyle == FNUM_MEDIA)
        {
            ctext = QString("%1").arg(_hframe - _frameIn + 1);

        }

        painter->drawText(_lxpos + 10, _hmid + 17, ctext);
    }
    painter->restore();
}