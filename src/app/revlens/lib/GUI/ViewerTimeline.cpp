

#include "RlpRevlens/GUI/ViewerTimeline.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(revlens, GUI, ViewerTimeline)

static const int X_OFFSET = 25;

GUI_ViewerTimeline::GUI_ViewerTimeline():
    DS_EventPlugin("ViewerTimeline", false),
    _chover(false),
    _lhover(false),
    _lxpos(0),
    _hmid(0),
    _wmax(0),
    _hframe(0),
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
    return _framecount * pos;
}


bool
GUI_ViewerTimeline::mousePressEventInternal(QMouseEvent* event)
{
    if (!_lhover)
    {
        return false;
    }

    qlonglong frameNum = getFrameNum(event->position().x());
    RLP_LOG_DEBUG(frameNum)

    #ifdef SCAFFOLD_IOS
    _hframe = frameNum;
    #endif

    emit startScrub(frameNum);

    return true;
}


bool
GUI_ViewerTimeline::mouseMoveEventInternal(QMouseEvent* event)
{
    if (!_chover)
    {
        return false;
    }

    if (event->position().x() >= INT_MAX)
    {
        // RLP_LOG_ERROR("invalid x pos")
        return false;
    }

    // RLP_LOG_DEBUG(event->position().x())

    qlonglong frameNum = getFrameNum(event->position().x());

    _hframe = frameNum;

    // RLP_LOG_DEBUG(frameNum)

    emit updateToFrame(frameNum);

    return true;
}


bool
GUI_ViewerTimeline::mouseReleaseEventInternal(QMouseEvent* event)
{
    qlonglong frameNum = getFrameNum(event->position().x());

    RLP_LOG_DEBUG(frameNum)

    #ifdef SCAFFOLD_IOS
    _hframe = frameNum;
    #endif

    emit endScrub(frameNum);

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

    _framecount = _controller->session()->frameCount();

    float pos = (float)_currframe / (float)_framecount;
    int wmax = wx - 70;
    _circpos = X_OFFSET + (pos * wmax);

    // RLP_LOG_DEBUG(_currframe << _framecount << pos << _circpos)

    painter->save();

    _hmid = wy - 100;
    _wmax = wmax;

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
        QString ctext = QString("%1").arg(_hframe);
        painter->drawText(_lxpos + 10, _hmid + 17, ctext);
    }
    painter->restore();
}