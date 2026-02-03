
#include "RlpRevlens/GUI_TLT/View.h"

#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"

RLP_SETUP_LOGGER(revlens, GUI_TLT, View)


GUI_TLT_View::GUI_TLT_View(CNTRL_MainController* cntrl, GUI_ItemBase* parent):
    GUI_TL2_ViewBase(cntrl, parent),
    _frameStart(1),
    _frameEnd(1),
    _frameCount(1),
    _showMediaToolTip(false),
    _bookendsOn(false)
{
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    _contextMenu = new GUI_MenuPane(this);
    _contextMenu->setVisible(false);

    connect(
        _contextMenu,
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_TLT_View::onContextMenuItemSelected
    );

    _contextMenu->addItem("Show Clip Details", QVariantMap(), true);

    _playhead = new GUI_TL2_Playhead(this);
    _playhead->setYOffset(0);
    _playhead->setMousePosTextXOffset(15);
    _playhead->setMousePosTextYOffset(20);

    _mmgr = new GUI_TLT_MediaManager(this);

    _syncHandler = new GUI_TL2_SyncHandler(this);

    connect(
        _playhead,
        &GUI_TL2_Playhead::rightClick,
        this,
        &GUI_TLT_View::onPlayheadRightClick
    );

    connect(
        cntrl,
        &CNTRL_MainController::noticeEvent,
        _mmgr,
        &GUI_TLT_MediaManager::onNoticeEvent
    );

    connect(
        this,
        &GUI_TLT_View::startScrub,
        _controller->getVideoManager(),
        &CNTRL_Video::onScrubStart
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
        &GUI_TLT_View::endScrub,
        _controller->getVideoManager(),
        &CNTRL_Video::onScrubEnd
    );

    connect(
        this,
        &GUI_TLT_View::endScrub,
        _controller->getAudioManager(),
        &CNTRL_Audio::onScrubEnd
    );

    connect(
        cntrl->session().get(),
        &DS_Session::propertyChanged,
        this,
        &GUI_TLT_View::onSessionPropertyChanged
    );

    connect(
        this,
        SIGNAL(updateToFrame(qlonglong)),
        _syncHandler,
        SLOT(onUpdateToFrame(qlonglong))
    );

    connect(
        this,
        SIGNAL(startScrub(qlonglong)),
        _syncHandler,
        SLOT(onScrubStart(qlonglong))
    );

    connect(
        this,
        SIGNAL(endScrub(qlonglong)),
        _syncHandler,
        SLOT(onScrubEnd(qlonglong))
    );

    // connect(
    //     cntrl->session().get(),
    //     &DS_Session::frameCountChanged,
    //     this,
    //     &GUI_TLT_View::onSessionFrameCountChanged
    // );


    setWidth(parent->width());
    setHeight(30);
}


GUI_TLT_View::~GUI_TLT_View()
{
    RLP_LOG_DEBUG("")

    delete _contextMenu;
    delete _playhead;
    delete _mmgr;
}


GUI_TLT_View*
GUI_TLT_View::new_GUI_TLT_View(GUI_ItemBase* parent)
{
    CNTRL_MainController* cntrl = CNTRL_MainController::instance();
    return new GUI_TLT_View(cntrl, parent);
}


void
GUI_TLT_View::setPixelsPerFrame(float width)
{
    // RLP_LOG_DEBUG("width:" << width << "framecount:" << _frameCount)

    _pixelsPerFrame = width / (float)_frameCount;

    _mmgr->refresh();
}


qlonglong
GUI_TLT_View::getFrameAtXPos(int xpos)
{
    // RLP_LOG_DEBUG(xpos << "ppf:" << _pixelsPerFrame << "fs:" << _frameStart)
    return floor(((float)xpos / _pixelsPerFrame) + _frameStart);
}


float
GUI_TLT_View::getXPosAtFrame(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame)
    return (frame - _frameStart) * _pixelsPerFrame;
}


void
GUI_TLT_View::syncUpdate(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum)

    _currFrame = frameNum - 1;

    updateInternal();
}


void
GUI_TLT_View::setInFrame(qlonglong frame)
{
    // RLP_LOG_DEBUG(frame)

    _frameStart = frame;
    _frameCount = _frameEnd - _frameStart + 1;

    setPixelsPerFrame(width());
}


void
GUI_TLT_View::setOutFrame(qlonglong frame)
{
    _frameEnd = frame;
    _frameCount = _frameEnd - _frameStart + 1;

    // RLP_LOG_DEBUG(_frameStart << "-" << _frameEnd << "count:" << _frameCount)

    setPixelsPerFrame(width());
}


void
GUI_TLT_View::setFrameCount(qlonglong frame)
{
    _frameStart = 1;
    _frameEnd = frame;
    _frameCount = frame;

    setPixelsPerFrame(width());
}


void
GUI_TLT_View::onPlayheadStartScrub(int xpos)
{
    RLP_LOG_DEBUG(xpos)

    CmdFilterResult res = _controller->checkCommand("Scrub");
    if (!res.first)
    {
        RLP_LOG_WARN("Command not allowed: " << res.second)
        return;
    }

    _currFrame = getFrameAtXPos(xpos);
    updateIfNecessary();

    emit startScrub(_currFrame);
}


void
GUI_TLT_View::onPlayheadScrubbing(int xpos)
{
    // RLP_LOG_DEBUG("");
    CmdFilterResult res = _controller->checkCommand("Scrub");
    if (!res.first)
    {
        RLP_LOG_WARN("Command not allowed: " << res.second)
        return;
    }

    _currFrame = getFrameAtXPos(xpos);
    updateIfNecessary();

    emit updateToFrame(_currFrame);
}


void
GUI_TLT_View::onPlayheadEndScrub(int xpos)
{
    // RLP_LOG_DEBUG("")

    CmdFilterResult res = _controller->checkCommand("Scrub");
    if (!res.first)
    {
        RLP_LOG_WARN("Command not allowed: " << res.second)
        return;
    }

    _currFrame = getFrameAtXPos(xpos);
    updateIfNecessary();

    emit endScrub(_currFrame);
}


void
GUI_TLT_View::onPlayheadRightClick(QPointF pos)
{
    QPointF mpos = _playhead->mapToScene(pos);

    // RLP_LOG_DEBUG(mpos)

    if (_contextMenu->isVisible())
    {
        _contextMenu->setVisible(false);
    } else
    {
        _contextMenu->setPos(mpos.x() + 10, mpos.y());
        _contextMenu->setVisible(true);
    }
}


void
GUI_TLT_View::onFrameCountChanged(qlonglong frameCount)
{
    RLP_LOG_DEBUG(frameCount)

    setFrameCount(frameCount);

    updateInternal();
}


void
GUI_TLT_View::onContextMenuItemSelected(QVariantMap itemInfo)
{
    // RLP_LOG_DEBUG(itemInfo)

    if (itemInfo.value("text").toString() == "Show Clip Details")
    {
        GUI_MenuItem* item = itemInfo.value("item").value<GUI_MenuItem*>();
        _showMediaToolTip = item->isChecked();
    }
}


void
GUI_TLT_View::onSessionPropertyChanged(QString key, QVariant val)
{
    // RLP_LOG_DEBUG(key << val)

    if (key == "bookend.state")
    {
        _bookendsOn = val.toBool();
        update();
    }
}


void
GUI_TLT_View::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // Nothing, toolbar should call updateWidth()
}


void
GUI_TLT_View::updateWidth(int width)
{
    // RLP_LOG_DEBUG(width)

    setPixelsPerFrame(width);
    setWidth(width);
}


void
GUI_TLT_View::updateInternal()
{
    _playhead->update();
}


void
GUI_TLT_View::updateIfNecessary()
{
    // RLP_LOG_DEBUG("")

    if (_controller->isPaused())
    {
        updateInternal();
    }
}


void
GUI_TLT_View::paint(GUI_Painter* painter)
{
    painter->setBrush(Qt::black);
    QColor col = Qt::black;
    if (_bookendsOn)
    {
        col = QColor(45, 25, 25);
    }

    painter->setPen(col);
    painter->drawRect(boundingRect());
}