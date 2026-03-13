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

#include "RlpRevlens/MEDIA/Factory.h"


RLP_SETUP_LOGGER(revlens, CNTRL, Video)

int CNTRL_Video::_INSTANCE_COUNT = 0;
QVariantList CNTRL_Video::_VIDEO_SOURCES;

CNTRL_Video::CNTRL_Video(DS_ControllerBase* controller, QString name) :
    VIDEO_SyncTarget(),
    _name(name),
    _controller(controller),
    _playbackMode(controller->playbackMode()),
    _direction(DS_PlaybackMode::FORWARD),
    _cache(new VIDEO_LookaheadCache()),
    _currentFrame(0),
    _scrubbing(false),
    _prerollNum(0),
    _prerollCount(0)
{
    _INSTANCE_COUNT++;
    _idx = _INSTANCE_COUNT;

    _VIDEO_SOURCES.append(_name);

    _contextMenuItems.clear();

    initFbuf();

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

    _compositePluginMap = new CompositePluginMap();
    _compositePluginMap->clear();

    registerPaintEngine("Default2D", &DISP_GLPaintEngine2D::create);
    registerPaintEngine("MultiTex", &DISP_GLPaintEngineMultiTex::create);

    QVariantMap evtInfo;
    evtInfo.insert("name", _name);
    evtInfo.insert("idx", _idx);

    _controller->emitNoticeEvent("video.source_created", evtInfo);
    RLP_LOG_DEBUG("OK")
}


void
CNTRL_Video::initFbuf()
{
    RLP_LOG_DEBUG("")

    // TODO FIXME: FIX FOR CRASH AT STARTUP ON WINDOWS
    // initializeGL() on windows at startup time requires
    // glTexImage2D to be initialized with a nonzero size buffer

    int defaultChannelCount = 4;
    int defaultPixelTypeDepth = 1; // sizeof(half);
    int defaultPixelSize = defaultChannelCount * defaultPixelTypeDepth;

    QVariantMap frameInfo;
    frameInfo.insert("width", 853);
    frameInfo.insert("height", 480);
    frameInfo.insert("channelCount", defaultChannelCount);
    frameInfo.insert("pixelSize", defaultPixelTypeDepth);
    
    _fbuf = DS_FrameBufferPtr(new DS_FrameBuffer(this, frameInfo));
    _fbuf->reallocate();
    _fbuf->setFrameNum(0);

    _blackFbuf = DS_FrameBufferPtr(new DS_FrameBuffer(this, frameInfo));
    _blackFbuf->reallocate();
}


void
CNTRL_Video::registerDisplay(DISP_GLView* display, bool autoSetPaintEngine)
{
    RLP_LOG_DEBUG("")

    connect(
        display,
        &DISP_GLView::fullscreenEnabled,
        this,
        &CNTRL_Video::onFullscreenStateChanged
    );


    if (autoSetPaintEngine)
    {
        display->initPaintEngines(_peCreateMap);

        if (_peCreateMap.contains("OCIO"))
        {
            display->setCurrentPaintEngine("OCIO");
        } else
        {
            display->setCurrentPaintEngine("Default2D");
        }
    }

    // WARNING: setFrameBuffer() must be called before setVideoSourceIdx()
    //
    display->setFrameBuffer(_idx, _fbuf);
    if (_idx == 1)
    {
        display->setVideoSourceIdx(_idx);
    }

    QVariantList plugNames = _controller->getEventPluginNames();
    for (int pi=0; pi != plugNames.size(); ++pi)
    {
        QString plugName = plugNames.at(pi).toString();
        DS_EventPlugin* plug = _controller->getEventPluginByName(plugName);
        display->registerEventPlugin(plug->duplicate());
    }

    _displays.append(display);

    display->setIdx(_displays.size());


    MEDIA_Factory* factory = MEDIA_Factory::instance();
    display->registerCompositePlugins(_compositePluginMap);

    // Setup display context menu items
    //

    GUI_MenuPane* mpane = display->contextMenu();
    for (int i=0; i != _contextMenuItems.size(); ++i)
    {
        QVariantMap cmiInfo = _contextMenuItems.at(i);
        mpane->addItem(cmiInfo.value("name").toString(), cmiInfo, false);
    }

    emit displayRegistered(display);
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

    bool result = false;

    if (didx >= 0)
    {
        RLP_LOG_DEBUG("Deregistering Display" << display)
        _displays.removeAt(didx);

        result = true;
    }

    return false;
}


bool
CNTRL_Video::registerProbe(DS_SyncProbe* probe)
{   
    RLP_LOG_DEBUG( "");

    probe->setMainController(_controller);
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


bool
CNTRL_Video::registerCompositePlugin(DISP_CompositePlugin* compositePlugin)
{
    RLP_LOG_DEBUG("Registering Composite Plugin" << compositePlugin->name())

    _compositePluginMap->insert(compositePlugin->name(), compositePlugin);

    return true;
}


void
CNTRL_Video::update()
{

    for (int di=0; di != _displays.size(); ++di)
    {
        DISP_GLView* disp = _displays.at(di);
        if (disp->videoSourceIdx() == _idx)
        {
            disp->update();
        }
        else
        {
            RLP_LOG_WARN("Skipping update for display at" << di)
        }
    }
}


void
CNTRL_Video::displayNextFrame()
{
    if (_controller->getPlaybackState() != DS_ControllerBase::PLAYSTATE_PLAYING)
    {
        return;
    }

    bool result = false;

    if (_prerollNum == _prerollCount) // no (more) preroll
    {
        if (_currentFrame < 0)
        {
            RLP_LOG_WARN("INVALID CURRENT FRAME");

            _controller->runCommand("TogglePlayState"); // pause..
            return;

        }

        _fbuf->setFrameNum(_currentFrame);

        result = _cache->present(_currentFrame, _fbuf);

    } else
    {
        _prerollNum++;

        if (_prerollNum == _prerollCount)
        {
            // Preroll done
            //

            for (int i=0; i != _probes.size(); i++)
            {
                _probes[i]->prerollEnd();
            }

            for (int di=0; di != _displays.size(); di++)
            {
                DISP_GLView* disp = _displays.at(di);
                disp->setFrameBuffer(_idx, _fbuf);
                disp->setVideoSourceIdx(_idx);
            }
        }
    }


    for (int di=0; di != _displays.size(); di++)
    {
        DISP_GLView* disp = _displays.at(di);
        if (disp->isEnabled() && disp->videoSourceIdx() == _idx)
        {
            disp->update();
        }
        else
        {
            // RLP_LOG_WARN("Display at" << di << "not updated")
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
        std::pair<qlonglong, DS_PlaybackMode::DirectionMode> nextFrameResult = _playbackMode->getNextFrameNum(
            _currentFrame,
            _direction,
            DS_PlaybackMode::INC
        );

        // HACK?
        if ((nextFrameResult.second == DS_PlaybackMode::FORWARD) && // no nothing if we're going backward
            (nextFrameResult.first < _currentFrame))
        {
            // RLP_LOG_DEBUG("Loop reset detected")

            _prerollNum = 0;

            if (_prerollCount > 0)
            {
                for (int i=0; i != _probes.size(); i++)
                {
                    _probes[i]->prerollStart(nextFrameResult.first);
                }

                for (int di=0; di != _displays.size(); di++)
                {
                    DISP_GLView* disp = _displays.at(di);
                    disp->setFrameBuffer(_idx, _blackFbuf);
                    disp->setVideoSourceIdx(_idx);
                }

                // Prevent showing last frame before preroll accidentally when
                // preroll is done
                _fbuf->setFrameNum(nextFrameResult.first);
                _cache->present(nextFrameResult.first, _fbuf);

            }
        }

        _currentFrame = nextFrameResult.first;

        if (nextFrameResult.second != _direction)
        {
            // RLP_LOG_DEBUG("Switching direction")

            _direction = nextFrameResult.second;
            _playbackMode->setDirection(_direction);
        }
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


DISP_GLView*
CNTRL_Video::getDisplayByUuid(QString uuidStr)
{
    // O(n) on number of displays, not ideal but assuming number of displays
    // is low
    //
    QList<DISP_GLView*>::iterator it;
    for (it = _displays.begin(); it != _displays.end(); ++it)
    {
        DISP_GLView* display = *it;
        if (display->uuid() == uuidStr)
        {
            return display;
        }
    }

    return nullptr;
}


void
CNTRL_Video::updateToFrame(qlonglong frameNum, bool recenter, bool schedule)
{
    // RLP_LOG_DEBUG( frameNum << " recenter:" << recenter << " schedule:" << schedule << " forceRead:" << scheduleForceRead);

    _currentFrame = frameNum;
    _prerollNum = _prerollCount;

    _fbuf->setFrameNum(frameNum);


    bool result = _cache->presentImmediate(
        frameNum,
        _fbuf,
        schedule,
        recenter);

    if (result)
    {

        update();

        for (int i=0; i != _probes.size(); i++)
        {
            DS_SyncProbe* probe = _probes.at(i);
            if (probe != nullptr)
            {
                _probes[i]->syncUpdateImmediate(frameNum);
            }
        }

        if (!_scrubbing)
        {
            // For UI updates
            emit frameDisplayed(frameNum);
        }
    }

}


void
CNTRL_Video::updateToFrameAsync(qlonglong frameNum, bool recenter)
{
    // RLP_LOG_DEBUG(frameNum)

    updateToFrame(frameNum, recenter, true);
}


void
CNTRL_Video::probeSyncUpdateImmediate(qlonglong frameNum)
{
    // RLP_LOG_DEBUG( frameNum);
    
    for (int i=0; i != _probes.size(); i++)
    {
        _probes[i]->syncUpdateImmediate(frameNum);
    }
}


void
CNTRL_Video::stop(int stopFrame)
{
    _avclock->stop();

    // The playback engine will have the current frame set to the next frame..
    // ensure the stopped frame is set correctly
    if (stopFrame != INT_MIN)
    {
        RLP_LOG_DEBUG( stopFrame);

        _currentFrame = stopFrame;
        probeSyncUpdateImmediate(stopFrame);
    }

    _prerollNum = _prerollCount;
}


void
CNTRL_Video::resume()
{
    RLP_LOG_DEBUG( _currentFrame);

    _avclock->resume();
} 


void
CNTRL_Video::clearAll(bool resetCurrFrame)
{
    if (resetCurrFrame)
    {
        _currentFrame = 0;
    }

    _cache->expireAll();

}


void
CNTRL_Video::setLoopMode(DS_PlaybackMode::LoopMode loopMode)
{
    RLP_LOG_DEBUG(loopMode)

    _playbackMode->setLoopMode(loopMode);
    _direction = DS_PlaybackMode::FORWARD;
    _playbackMode->setDirection(_direction);
}


void
CNTRL_Video::setDirectionMode(DS_PlaybackMode::DirectionMode dirMode)
{
    RLP_LOG_DEBUG(dirMode)
    _direction = dirMode;
    _playbackMode->setDirection(dirMode);

    updateToFrame(_currentFrame, /*recenter=*/ true, /*schedule=*/ true);
}


void
CNTRL_Video::setPrerollFrameCount(int frameCount)
{
    RLP_LOG_DEBUG(frameCount)

    _prerollCount = frameCount;
    _prerollNum = frameCount;
}


void
CNTRL_Video::onFrameCountChanged(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    _playbackMode->setFrameRange(1, frameNum);
}


void
CNTRL_Video::onInFrameChanged(qlonglong inFrame)
{
    // RLP_LOG_DEBUG(inFrame)

    _playbackMode->setInFrame(inFrame);
}


void
CNTRL_Video::onScrubStart(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    _scrubbing = true;
    
    updateToFrame(
        frameNum,
        /* recenter = */ false,
        /* schedule = */ true
    );
}


void
CNTRL_Video::onScrubEnd(qlonglong frameNum)
{
    // RLP_LOG_DEBUG(frameNum);

    _scrubbing = false;
    updateToFrame(
        frameNum,
        /* recenter = */ true,
        /* schedule = */ true
    );

    QVariantMap info;
    info.insert("frame_num", frameNum);
    _controller->emitNoticeEvent("scrub_end", info);
}


void
CNTRL_Video::onFrameRead(MEDIA_ReadEvent* fre)
{
    // RLP_LOG_DEBUG( fre->getTimeInfo()->videoFrame());

    qlonglong frameNum = fre->getTimeInfo()->videoFrame();

    _fbuf->setFrameNum(frameNum);
    fre->getFrameBuffer()->copyTo(_fbuf);

    // RLP_LOG_DEBUG(frameNum);

    update();


    for (int i=0; i != _probes.size(); i++)
    {
        _probes[i]->syncUpdateImmediate(frameNum);
    }
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
