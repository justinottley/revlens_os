//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/Video.h"

#include "RlpRevlens/CNTRL/MainController.h"


// Different clock implementations.
// The normal one is SyncClock, TimerClock is for single threaded, and
// ThreadedClock is the same as SyncClock, ported to the ClockBase
// interface
//

#include "RlpRevlens/VIDEO/ThreadedClock.h"
#include "RlpRevlens/VIDEO/TimerClock.h"


RLP_SETUP_LOGGER(revlens, CNTRL, Video)

CNTRL_Video::CNTRL_Video(CNTRL_MainController* controller) :
    VIDEO_SyncTarget(),
    _controller(controller),
    _playbackMode(controller->playbackMode()),
    _cache(new VIDEO_LookaheadCache()),
    _currentFrame(0),
    _scrubbing(false),
    _prerollNum(0),
    _prerollCount(0)
{
    _contextMenuItems.clear();

    QVariantMap frameInfo;
    
    int defaultChannelCount = 4;
    int defaultPixelTypeDepth = 1; // sizeof(half);
    int defaultPixelSize = defaultChannelCount * defaultPixelTypeDepth;
    
    // TODO FIXME: FIX FOR CRASH AT STARTUP ON WINDOWS
    // initializeGL() on windows at startup time requires
    // glTexImage2D to be initialized with a nonzero size buffer
    //
    frameInfo.insert("width", 853);
    frameInfo.insert("height", 480);
    frameInfo.insert("channelCount", defaultChannelCount);
    frameInfo.insert("pixelSize", defaultPixelTypeDepth);
    
    _fbuf = std::shared_ptr<DS_FrameBuffer>(new DS_FrameBuffer(this, frameInfo));
    _fbuf->reallocate();
    _fbuf->setFrameNum(0);


    #ifndef SCAFFOLD_WASM
    _avclock = new VIDEO_ThreadedClock(this, 24);
    #else
    _avclock = new VIDEO_TimerClock(this, 24);
    #endif


    connect(
        _cache,
        SIGNAL(frameReadEvent(MEDIA_ReadEvent*)),
        this,
        SLOT(onFrameRead(MEDIA_ReadEvent*))
    );

    _cache->setController(controller);
    _cache->setPlaybackMode(_playbackMode);

    registerPaintEngine("Default2D", &DISP_GLPaintEngine2D::create);

    RLP_LOG_DEBUG("OK")
}


void
CNTRL_Video::registerDisplay(DISP_GLView* display)
{
    RLP_LOG_DEBUG("")

    connect(
        display,
        &DISP_GLView::fullscreenEnabled,
        this,
        &CNTRL_Video::onFullscreenStateChanged
    );

    display->initPaintEngines(_peCreateMap);


    if (_peCreateMap.contains("OCIO"))
    {
        display->setCurrentPaintEngine("OCIO");
    } else
    {
        display->setCurrentPaintEngine("Default2D");
    }

    // display->setCurrentPaintEngine("Default2D");
    display->setFrameBuffer(_fbuf);

    QVariantList plugNames = _controller->getEventPluginNames();
    for (int pi=0; pi != plugNames.size(); ++pi)
    {
        QString plugName = plugNames.at(pi).toString();
        DS_EventPlugin* plug = _controller->getEventPluginByName(plugName);
    
        display->registerEventPlugin(plug->duplicate());
    }

    _displays.append(display);

    GUI_MenuPane* mpane = display->contextMenu();


    // Setup display context menu items
    for (int i=0; i != _contextMenuItems.size(); ++i)
    {
        QVariantMap cmiInfo = _contextMenuItems.at(i);
        mpane->addItem(cmiInfo.value("name").toString(), cmiInfo, false);
    }
}


void
CNTRL_Video::registerDisplayContextMenuItem(QVariantMap cmiInfo)
{
    RLP_LOG_DEBUG(cmiInfo);

    _contextMenuItems.append(cmiInfo);
}


bool
CNTRL_Video::deregisterDisplay(DISP_GLView* display)
{
    RLP_LOG_DEBUG("")

    disconnect(
        display,
        &DISP_GLView::fullscreenEnabled,
        this,
        &CNTRL_Video::onFullscreenStateChanged
    );

    emit displayDeregistered(display);

    int didx = -1;
    for (int dx=0; dx != _displays.size(); ++dx)
    {
        if (_displays.at(dx) == display)
        {
            RLP_LOG_DEBUG("FOUND at index" << dx)
            didx = dx;
            break;
        }
    }

    if (didx >= 0)
    {
        RLP_LOG_DEBUG("Deregistering Display" << display)
        _displays.removeAt(didx);

        return true;
    }

    return false;
}


bool
CNTRL_Video::registerProbe(DS_SyncProbe* probe)
{   
    RLP_LOG_DEBUG( "");
    _probes.push_back(probe);
    
    return true;
}


bool
CNTRL_Video::deregisterProbe(DS_SyncProbe* probe)
{
    int pidx = -1;
    for (int px=0; px != _probes.size(); ++px)
    {
        if (_probes.at(px) == probe)
        {
            RLP_LOG_DEBUG("FOUND PROBE at index" << px)
            pidx = px;
            break;
        }
    }

    if (pidx >= 0)
    {
        RLP_LOG_DEBUG("Removing probe at" << pidx)
        _probes.removeAt(pidx);

        return true;
    }

    return false;
}


void
CNTRL_Video::registerEventPlugin(DS_EventPlugin* plugin)
{
    // RLP_LOG_DEBUG(plugin->name())

    for (int di=0; di != _displays.size(); ++di)
    {
        _displays.at(di)->registerEventPlugin(plugin->duplicate());
    }
}


void
CNTRL_Video::registerPaintEngine(QString name, PaintEngineCreateFunc func)
{
    RLP_LOG_DEBUG(name)
    _peCreateMap.insert(name, func);
}


void
CNTRL_Video::update()
{
    // RLP_LOG_DEBUG("")

    for (int di=0; di != _displays.size(); ++di)
    {
        _displays.at(di)->update();
    }
}


void
CNTRL_Video::displayNextFrame()
{
    // RLP_LOG_DEBUG("")

    if (_controller->getPlaybackState() != CNTRL_MainController::PLAYING) {
        return;
    }

    bool result = false;

    if (_prerollNum == _prerollCount)
    {
        // RLP_LOG_DEBUG( "CNTRL_Video::displayNextFrame() : " << _currentFrame);

        if (_currentFrame < 0) {

            RLP_LOG_WARN("INVALID CURRENT FRAME");

            _currentFrame = 0;
            return;
            
        }

        _fbuf->setFrameNum(_currentFrame);

        result = _cache->present(_currentFrame, _fbuf);

    } else {

        // RLP_LOG_DEBUG("Incrementing preroll:" << _prerollNum)

        _prerollNum++;
    }


    for (int di=0; di != _displays.size(); di++)
    {
        if (_displays.at(di)->isEnabled()) {
            _displays.at(di)->update();
        }
    }

    // NOTE: the QtQuick based GUI_View actually only needs an update
    // per window - it looks like the whole window is redrawn.
    // Maybe this can be optimized if a SyncProbe (e.g., Timeline)
    // is in the same window as a display. Or we can just.. iterate
    // on the known windows?

    for (int i=0; i != _probes.size(); i++)
    {
        _probes[i]->syncUpdate(_currentFrame);
    }


    if (result) // Advance to the next frame unless inside preroll
    {
        _currentFrame = _playbackMode->getNextFrameNum(_currentFrame);
    }
}


void
CNTRL_Video::setTargetFrameRate(float frameRate)
{
    _avclock->setTargetFrameRate(frameRate);
}


float
CNTRL_Video::getTargetFrameRate()
{
    return _avclock->getTargetFrameRate();
}



void
CNTRL_Video::updateToFrame(qlonglong frameNum, bool recenter, bool schedule, bool scheduleForceRead)
{
    // RLP_LOG_DEBUG( frameNum << " recenter:" << recenter << " schedule:" << schedule << " forceRead:" << scheduleForceRead);

    _currentFrame = frameNum;
    _prerollNum = 0;

    _fbuf->setFrameNum(frameNum);

    // bool schedule = !(recenter);
    bool result = _cache->presentImmediate(
        frameNum,
        _fbuf, // _display->presentNextFrame(frameNum),
        schedule,
        scheduleForceRead,
        recenter);

    if (result)
    {

        update();

        for (int i=0; i != _probes.size(); i++) {
            _probes[i]->syncUpdateImmediate(frameNum);
        }

        if (!_scrubbing) {
            // For UI updates
            emit frameDisplayed(frameNum);
        }

        if (recenter) {
            _cache->recenter(frameNum);
        }

    }

}


void
CNTRL_Video::updateToFrameAsync(qlonglong frameNum, bool recenter, bool forceRead)
{
    // RLP_LOG_DEBUG(frameNum)

    updateToFrame(frameNum, recenter, true, forceRead);
}


void
CNTRL_Video::probeSyncUpdateImmediate(qlonglong frameNum)
{
    // RLP_LOG_DEBUG( frameNum);
    
    for (int i=0; i != _probes.size(); i++) {
        _probes[i]->syncUpdateImmediate(frameNum);
    }
}


void
CNTRL_Video::stop(int stopFrame)
{
    _avclock->stop();

    // The playback engine will have the current frame set to the next frame..
    // ensure the stopped frame is set correctly
    if (stopFrame != INT_MIN) {
        RLP_LOG_DEBUG( stopFrame);
        
        _currentFrame = stopFrame;
        probeSyncUpdateImmediate(stopFrame);
    }
}


void
CNTRL_Video::resume()
{
    RLP_LOG_DEBUG( _currentFrame);

    // updateToFrame(_currentFrame, true);
    _avclock->resume();
} 


void
CNTRL_Video::clearAll(bool resetCurrFrame)
{
    if (resetCurrFrame) {
        _currentFrame = 0;
    }
    _cache->expireAll();
    
}
// ---------


void
CNTRL_Video::onFrameCountChanged(qlonglong frameNum)
{
    // RLP_LOG_DEBUG( frameNum);
    _playbackMode->setFrameRange(1, frameNum);
}


void
CNTRL_Video::onScrubStart(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    _scrubbing = true;
    
    updateToFrame(frameNum, false, true);
}


void
CNTRL_Video::onScrubEnd(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    _scrubbing = false;
    
    updateToFrame(frameNum, true, true);
    // updateToFrame(frameNum, false);
    
}


void
CNTRL_Video::onFrameRead(MEDIA_ReadEvent* fre)
{
    // RLP_LOG_DEBUG( fre->getTimeInfo()->videoFrame());

    qlonglong frameNum = fre->getTimeInfo()->videoFrame();

    _fbuf->setFrameNum(frameNum);
    fre->getFrameBuffer()->copyTo(_fbuf);

    if (_scrubbing)
    {
        DS_FrameBuffer::discardForReuse(
            std::static_pointer_cast<DS_FrameBuffer>(fre->getFrameBuffer()));
    }

    // RLP_LOG_DEBUG(frameNum);

    update();


    for (int i=0; i != _probes.size(); i++) {
        _probes[i]->syncUpdateImmediate(frameNum);
    }

    // if (!_scrubbing) {
    //     _cache->recenter(frameNum);
    //     // For UI updates
    //     emit frameDisplayed(frameNum);
    // }
}


void
CNTRL_Video::onFullscreenStateChanged(bool isEnabled, QString uuidStr)
{
    RLP_LOG_DEBUG(isEnabled << uuidStr)

    // Enable this run update on only the presentation window
    // May be necessary for less capable video cards (built-in Intel)

    // for (int di=0; di != _displays.size(); di++)
    // {
    //     if (_displays.at(di)->uuid() == uuidStr)
    //     {
    //         RLP_LOG_DEBUG(uuidStr << isEnabled)
    //         _displays.at(di)->setEnabled(isEnabled);

    //     } else
    //     {
    //         RLP_LOG_DEBUG(di << !isEnabled)
    //         _displays.at(di)->setEnabled(!isEnabled);
    //     }
    // }
}
