
#include "RlpRevlens/GUI_TL2/View.h"

#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"
#include "RlpRevlens/CNTRL/MediaManager.h"

#include "RlpRevlens/GUI_TL2/Controller.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, View)

GUI_TL2_ViewTimelineArea::GUI_TL2_ViewTimelineArea(GUI_TL2_View* view):
    GUI_ItemBase(view),
    _view(view)
{
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    // need this for the timeline area to paint over the button bar area
    setZValue(1);
}


void
GUI_TL2_ViewTimelineArea::refresh()
{
    // RLP_LOG_DEBUG("")

    setWidth(_view->timelineWidth());
    setHeight(_view->height()); // _view->timelineHeight() - 1);
    setPos(_view->timelineXPosStart() - 1, 0);

}


QRectF
GUI_TL2_ViewTimelineArea::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_ViewTimelineArea::paint(GUI_Painter* painter)
{
    // painter->setBrush(QBrush(Qt::red));
    // painter->drawRect(boundingRect());
}


GUI_TL2_View::GUI_TL2_View(CNTRL_MainController* cntrl, GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    DS_SyncProbe(),
    _controller(cntrl),
    _timelineXPosStart(180),
    _timelineXPosRightPad(28),
    _timelineMaxTrackHeight(0),
    _frameCount(1),
    _frameStart(1),
    _currFrame(1),
    _isFocusedOnFrame(false),
    _zoom(1.0),
    _pan(0.0),
    _panStart(0.0),
    _yscrollPos(0.0),
    _inputMode(TL_MODE_SCRUB),
    _uuidStr(QUuid::createUuid().toString())
{
    // important: ensure all elements (tracks) clip to the widget area and
    // dont draw outside (like via scrolling)
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    /*
    connect(
        parent,
        SIGNAL(sizeChanged(qreal, qreal)),
        this,
        SLOT(onSizeChanged(qreal, qreal))
    );
    */
    _timelineArea = new GUI_TL2_ViewTimelineArea(this);

    _ticks = new GUI_TL2_Ticks(this);

    _zoomBar = new GUI_TL2_ZoomBar(this); // _timelineArea);
    

    _toolBar = new GUI_TL2_ToolBar(this);
    _toolBar->setPos(0, height() - 20);

    _bookends = new GUI_TL2_Bookends(this);
    
    _splitter = new GUI_TL2_Splitter(this);
    _splitter->setPos(
        _timelineXPosStart - _bookends->buttonWidth() - _splitter->width() - 1,
        0);

    _playhead = new GUI_TL2_Playhead(this);
    
    _cache = new GUI_TL2_Cache(this);
    _cache->setPos(0, 5);
    
    _vbar = new GUI_TL2_VerticalScrollBar(this);
    _vbar->updatePos();

    _syncHandler = new GUI_TL2_SyncHandler(this);
    
    setFrameCount(1);

    _trackManager = new GUI_TL2_TrackManager(this);

    initConnections();

    // setWidth(qobject_cast<GUI_FrameBase*>(parent)->width()); // 900;
    // setHeight(qobject_cast<GUI_FrameBase*>(parent)->height()); // 200;

    // _zoomBar->setPos(0, height() - 20);

    cntrl->getVideoManager()->registerProbe(this);
    GUI_TL2_Controller::instance()->registerTimelineView(_uuidStr, this);

    onParentSizeChanged(parent->width(), parent->height()); // TODO FIXME WHY DO I NEED THIS HEIGHT OFFSET??
}


GUI_TL2_View::~GUI_TL2_View()
{
    RLP_LOG_DEBUG("")
    CNTRL_MainController::instance()->getVideoManager()->deregisterProbe(this);
}


void
GUI_TL2_View::initConnections()
{
    RLP_LOG_DEBUG("Internal")

    //
    // Internal
    //

    connect(_splitter, SIGNAL(splitterMoved(int)), this, SLOT(onSplitterMoved(int)));
    connect(_zoomBar, SIGNAL(zoomChanged(float, int, int, int)), this, SLOT(onZoomChanged(float, int, int, int)));
    connect(_zoomBar, SIGNAL(panChanged(int, int)), this, SLOT(onPanChanged(int, int)));

    connect(_bookends, SIGNAL(inFrameChanged(qlonglong)), this, SLOT(onInFrameChanged(qlonglong)));
    connect(_bookends, SIGNAL(outFrameChanged(qlonglong)), this, SLOT(onOutFrameChanged(qlonglong)));

    connect(_vbar, SIGNAL(scrollPosChanged(float)), this, SLOT(onScrollPosChanged(float)));

    //
    // Main Controller
    //

    RLP_LOG_DEBUG("Main Controller")

    connect(
        _controller,
        SIGNAL(sessionChanged(DS_SessionPtr)),
        this,
        SLOT(onSessionChanged(DS_SessionPtr))
    );
    
    connect(
        _controller->getMediaManager(),
        &CNTRL_MediaManager::nodeDataReady,
        this,
        &GUI_TL2_View::onNodeDataReady
    );

    /*
    connect(
        _controller,
        SIGNAL(dataReady(QVariantMap)),
        this,
        SLOT(onDataReady(QVariantMap))
    );
    */


    connect(
        _trackManager,
        SIGNAL(itemSelected(QVariantMap)),
        _controller->getMediaManager(),
        SLOT(onItemSelectionChanged(QVariantMap))
    );
    

    /*
     * do not reenable, disabled in original view
     * 
    connect(
        this,
        SIGNAL(inFrameChanged(qlonglong)),
        _controller,
        SLOT(setInFrame(qlonglong))
    );

    connect(
        this,
        SIGNAL(outFrameChanged(qlonglong)),
        _controller,
        SLOT(setOutFrame(qlonglong))
    );
    */


    //
    // Cache
    //

    RLP_LOG_DEBUG("Cache")

    connect(
        _controller->getVideoManager()->cache(),
        SIGNAL(lookaheadAdd(qlonglong)),
        _cache,
        SLOT(onLookaheadAdd(qlonglong))
    );

    connect(
        _controller->getVideoManager()->cache(),
        SIGNAL(lookaheadRelease(qlonglong)),
        _cache,
        SLOT(onLookaheadRelease(qlonglong))
    );

    connect(
        _controller->getVideoManager()->cache(),
        SIGNAL(lookaheadClear()),
        _cache,
        SLOT(onLookaheadClear())
    );


    RLP_LOG_DEBUG("Audio")

    connect(
        _controller->getAudioManager(),
        SIGNAL(bufferFillEvent(QList<qlonglong>&)),
        _cache,
        SLOT(onAudioAdd(QList<qlonglong>&))
    );

    //
    // Video - frameDisplayed (async frame display)
    //

    /*
    connect(
        _controller->getVideoManager(),
        SIGNAL(frameDisplayed(qlonglong)),
        this,
        SLOT(onFrameDisplayed(qlonglong))
    );
    */

    //
    // Timeline
    //

    RLP_LOG_DEBUG("Timeline")

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

    //
    // Sync
    //

    RLP_LOG_DEBUG("Sync")

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

    


}


QQuickItem*
GUI_TL2_View::create(GUI_ItemBase* parent, QVariantMap toolInfo)
{
    CNTRL_MainController* cntrl = CNTRL_MainController::instance();

    GUI_TL2_View* tlView = new GUI_TL2_View(cntrl, parent);

    tlView->onSessionChanged(cntrl->session());
    tlView->trackManager()->initCurrSession();
    tlView->trackManager()->refresh();

    return tlView;
}


GUI_TL2_View*
GUI_TL2_View::new_GUI_TL2_View(GUI_ItemBase* parent)
{
    QVariantMap toolInfo;
    QQuickItem* tlView = GUI_TL2_View::create(parent, toolInfo);
    return qobject_cast<GUI_TL2_View*>(tlView);
}



// ------


int
GUI_TL2_View::timelineWidth()
{
    return width() - _timelineXPosStart - _timelineXPosRightPad - GUI_TL2_BookendDragButton::BUTTON_WIDTH;
}


int
GUI_TL2_View::timelineHeight()
{
    // return height() - _zoomBar->height() - 1;
    return height() - _zoomBar->height() - 10;
}


int
GUI_TL2_View::timelineTrackHeaderWidth()
{
    return _timelineXPosStart - _bookends->buttonWidth() - _splitter->width();
}


QRectF
GUI_TL2_View::timelineClipRect()
{
    return QRectF(
        _timelineXPosStart - _bookends->buttonWidth() - 1,
        0,
        timelineWidth() + (_bookends->buttonWidth() * 2) + 1,
        timelineHeight());
}


QRegion
GUI_TL2_View::timelineClipFullHeightRegion()
{
    return QRegion(
        _timelineXPosStart - _bookends->buttonWidth() - 1,
        0,
        timelineWidth() + (_bookends->buttonWidth() * 2),
        height());
}


QRegion
GUI_TL2_View::fullRegion()
{
    return QRegion(0, 0, width(), height());
}


QRectF
GUI_TL2_View::timelineRect()
{
    return QRectF(
        _timelineXPosStart,
        0,
        timelineWidth(),
        timelineHeight()
    );
}


void
GUI_TL2_View::setTimelineXPosStart(int xpos)
{
    // RLP_LOG_DEBUG(xpos)

    _timelineXPosStart = xpos;
    setPixelsPerFrame(width());

    refresh();
}


void
GUI_TL2_View::setTimelineMaxTrackHeight(int maxHeight)
{
    // RLP_LOG_DEBUG(maxHeight)

    _timelineMaxTrackHeight = maxHeight;

    emit timelineMaxTrackHeightChanged(maxHeight);
}

// ------


void
GUI_TL2_View::setPixelsPerFrame(float width)
{
    _pixelsPerFrame = (((float)(width - _timelineXPosStart - _timelineXPosRightPad - _bookends->buttonWidth())) / (float)_frameCount);
    // _pixelsPerFrame = (((float)(width - _timelineXPosStart - _timelineXPosRightPad)) / (float)_frameCount);

    // RLP_LOG_DEBUG(_pixelsPerFrame << " frameCount: " << _frameCount)
}



void
GUI_TL2_View::setInFrame(qlonglong frame, bool emitSignal)
{
    RLP_LOG_DEBUG(frame);

    _bookends->setInFrame(frame);
    _bookends->update();
}


void
GUI_TL2_View::setOutFrame(qlonglong frame, bool emitSignal)
{
    RLP_LOG_DEBUG(frame);
    
    _bookends->setOutFrame(frame);
    _bookends->update();
}


void
GUI_TL2_View::setFrameCount(qlonglong frameCount, bool updateBookends)
{
    RLP_LOG_DEBUG(frameCount);

    qlonglong currFrameCount = _frameCount;
    _frameCount = frameCount;
    setPixelsPerFrame(width());
    
    if (updateBookends) {
        if (_bookends->btnRight()->posFrame() > frameCount) {
        
            RLP_LOG_WARN("Out point greater than new frame count, resetting to " << frameCount);

            _bookends->setOutFrame(frameCount);

        } else if (_bookends->btnRight()->posFrame() == currFrameCount) {

            // Automatically grow the bookend end point
            _bookends->setOutFrame(frameCount);

        }
    }
    
}


void
GUI_TL2_View::setFrameStart(qlonglong frame)
{
    RLP_LOG_DEBUG(frame);

    _frameStart = frame;
}


void
GUI_TL2_View::setFrameEnd(qlonglong frame)
{
    RLP_LOG_DEBUG(frame);

    if (frame > _frameStart) {
        setFrameCount(frame - _frameStart, false);
    }
}


void
GUI_TL2_View::setTimelineInputMode(int modeInt)
{
    RLP_LOG_DEBUG(modeInt)

    GUI_TL2_View::TimelineInputMode mode = static_cast<GUI_TL2_View::TimelineInputMode>(modeInt);
    QString modeStr = "Timeline : ";

    if (mode == TL_MODE_EDIT)
    {
        _playhead->setVisible(false);
        _toolBar->setEditMode();

        modeStr += "Edit";

    }
    else
    {
        _playhead->setVisible(true);
        _toolBar->setScrubMode();

        modeStr += "Scrub";
    }

    _inputMode = mode;

    updateInternal();

    // Crashes on Windows
    // TODO FIXME
    //
    // QVariantMap callInfo;
    // QVariantList argList;
    // argList.append(modeStr);

    // callInfo.insert("method", "rlplug_qtbuiltin.cmds.display_message");
    // callInfo.insert("args", argList);

    // _controller->pyCall(callInfo);

}


// ------

void
GUI_TL2_View::onPlayheadStartScrub(int xpos)
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

    emit startScrub(_currFrame);
}


void
GUI_TL2_View::onPlayheadScrubbing(int xpos)
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
GUI_TL2_View::onPlayheadEndScrub(int xpos)
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


// -----
// Slots
// -----




void
GUI_TL2_View::onScrollPosChanged(float ypos)
{
    // RLP_LOG_DEBUG(ypos);

    _yscrollPos = ypos;
    _trackManager->refresh();
}



void
GUI_TL2_View::onInFrameChanged(qlonglong frame)
{
    RLP_LOG_DEBUG(frame);

    // Just re-emit signal coming in from private
    //
    // emit inFrameChanged(frame);
    _controller->setInFrame(frame);
}


void
GUI_TL2_View::onOutFrameChanged(qlonglong frame)
{
    RLP_LOG_DEBUG(frame);

    // Just re-emit signal coming in from private
    //
    // emit outFrameChanged(frame);
    _controller->setOutFrame(frame);
}


void
GUI_TL2_View::onZoomChanged(float zoom, int side, int leftXPos, int rightXPos)
{
    // RLP_LOG_DEBUG(zoom)

    _zoom = zoom;

    // RLP_LOG_DEBUG("zoom: " << _zoom << " side: " << side << " leftXPos: " << leftXPos << " rightXPos: " << rightXPos);
    // RLP_LOG_DEBUG("PPF:" << _pixelsPerFrame << " timelineWidth: " << timelineWidth());

    float npan = leftXPos * _zoom;

    _panStart = npan;
    _pan = npan;

    // if (side == GUI_TL2_ZoomBarDragButton::BTN_LEFT)

    _bookends->onZoomChanged();
    refresh();
}


void
GUI_TL2_View::onPanChanged(int side, int xDiff)
{

    _pan = _panStart + ((float)xDiff * _zoom);

    // RLP_LOG_DEBUG("_pan: " << _pan << " _panStart: " << _panStart << " _zoom: " << _zoom << " xDiff: " << xDiff);

    _bookends->onPanChanged();
    refresh();
}


void
GUI_TL2_View::onSplitterMoved(int xpos)
{
    // RLP_LOG_DEBUG(xpos);

    setTimelineXPosStart(xpos + _bookends->buttonWidth() + _splitter->width() + 1);
}


// -----
// Slots - Session
//

void
GUI_TL2_View::onSessionChanged(DS_SessionPtr session)
{
    // RLP_LOG_DEBUG("");

    
    connect(
        session.get(),
        SIGNAL(frameCountChanged(qlonglong)),
        this,
        SLOT(onFrameCountChanged(qlonglong))
    );

    connect(
        session.get(),
        SIGNAL(inFrameChanged(qlonglong)),
        this,
        SLOT(setInFrame(qlonglong))
    );

    connect(
        session.get(),
        SIGNAL(outFrameChanged(qlonglong)),
        this,
        SLOT(setOutFrame(qlonglong))
    );

    connect(
        session.get(),
        SIGNAL(sessionCleared()),
        this,
        SLOT(onSessionCleared())
    );

    
    _trackManager->onSessionChanged(session);
    
}


void
GUI_TL2_View::onSessionCleared()
{
    RLP_LOG_DEBUG("");

    _trackManager->clearTracks();
    setFrameCount(1);
}


void
GUI_TL2_View::onFrameCountChanged(qlonglong frameCount)
{
    RLP_LOG_DEBUG(frameCount)

    // force reset
    _toolBar->reset();

    setFrameCount(frameCount);
    update();
}


void
GUI_TL2_View::onNodeDataReady(QVariantMap nodeInfo)
{
    if (nodeInfo.value("data_type") == "media_list")
    {
        refresh();
    }
}

// ---------
// Frame Geo
// ---------


float
GUI_TL2_View::getXPosAtFrame(qlonglong frame)
{
    // float result = ((frame - _frameStart) * _pixelsPerFrame * _zoom) - _pan + _timelineXPosStart;
    float result = ((frame - _frameStart) * _pixelsPerFrame * _zoom) - _pan;

    // RLP_LOG_DEBUG(frame << ":" << result << "framestart:" << _frameStart << "ppf:" << _pixelsPerFrame << "zoom:" << _zoom << "pan:" << _pan)

    return result;
}


qlonglong
GUI_TL2_View::getFrameAtXPos(int xpos)
{
    // RLP_LOG_DEBUG(xpos << " timelineXPosStart: " << _timelineXPosStart)

    // TODO FIXME: floor() returns an int
    
    // return floor(((float)xpos + _pan - _timelineXPosStart) / (_pixelsPerFrame * _zoom)) + _frameStart;
    return floor(((float)xpos + _pan) / (_pixelsPerFrame * _zoom)) + _frameStart;
}


// --------
// Painter
// --------


void
GUI_TL2_View::updateInternal()
{
    // update();

    _playhead->update();
    _cache->update();
    _ticks->update();
    _zoomBar->update();
}


void
GUI_TL2_View::refresh()
{
    _timelineArea->refresh();

    _zoomBar->refresh();
    _bookends->refresh();
    _toolBar->refresh();
    _playhead->refresh();
    _ticks->refresh();

    _trackManager->refresh();
}


void
GUI_TL2_View::syncUpdate(qlonglong frameNum)
 {
    // RLP_LOG_DEBUG(frameNum);

    _currFrame = frameNum - 1;
    updateInternal();
}


void
GUI_TL2_View::syncUpdateImmediate(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum)

    // Avoiding one functional call indirection..
    _currFrame = frameNum - 1;
    updateInternal();
}


void
GUI_TL2_View::updateIfNecessary()
{
    // RLP_LOG_DEBUG("")

    if (_controller->isPaused()) {
        updateInternal();
    }
}


void
GUI_TL2_View::onParentSizeChanged(qreal nwidth, qreal nheight)
{
    // RLP_LOG_DEBUG(nwidth << " " << nheight)

    GUI_ItemBase* parent = qobject_cast<GUI_ItemBase*>(parentItem());
    qreal parentHeightOffset = parent->heightOffset();

    if (parentHeightOffset > 0)
    {
        parentHeightOffset += 5;
    }

    setPixelsPerFrame(nwidth);

    setWidth(nwidth);
    setHeight(nheight - parentHeightOffset);

    

    //_zoomBar->onSizeChanged(nwidth, nheight);
    //_toolBar->onSizeChanged(nwidth, nheight);
    
    //_bookends->onParentSizeChanged(nwidth, nheight);
    
    _timelineArea->refresh();
    _trackManager->refresh(nheight); // onParentSizeChanged(nwidth, nheight);
    _playhead->refresh();
    // _vbar->onSizeChanged(nwidth, nheight);

}


QRectF
GUI_TL2_View::boundingRect() const
{
    //RLP_LOG_DEBUG("BR: " << parentItem()->boundingRect() << " " << width() << " " << height())

    // return parentItem()->boundingRect();
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_View::paint(GUI_Painter* painter)
{
    painter->save();

    QRectF br = boundingRect();

    QRegion clipRegion(0, 0, br.width(), br.height());
    painter->setClipRegion(clipRegion); // QRect(0, 0, 100, 100));

    // painter->setPen(Qt::red);
    painter->setBrush(Qt::black);
    painter->setPen(Qt::black);
    painter->drawRect(boundingRect());

    painter->restore();
}
