//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpCore/PY/Interp.h"
#include "RlpCore/UTIL/AppGlobals.h"

#include "RlpGui/BASE/ItemBase.h"

#include "RlpRevlens/DS/EventPlugin.h"

#include "RlpRevlens/AUDIO/Manager.h"

#include "RlpRevlens/CNTRL/Commands.h"
#include "RlpRevlens/CNTRL/MediaManager.h"
#include "RlpRevlens/CNTRL/Video.h"
#include "RlpRevlens/CNTRL/Audio.h"
#include "RlpRevlens/CNTRL/PluginManager.h"
#include "RlpRevlens/CNTRL/Server.h"
#include "RlpRevlens/CNTRL/MediaServiceController.h"

// XXX: temporary
#include "RlpRevlens/CNTRL/FrameRateProbe.h"

RLP_SETUP_LOGGER(revlens, CNTRL, MainController)

CNTRL_MainController* CNTRL_MainController::_instance;

CNTRL_MainController::CNTRL_MainController():
    DS_ControllerBase(),
    _playbackMode(new DS_PlaybackMode()),
    _playbackState(CNTRL_MainController::PAUSED)
{
    _plugins.clear();
    _evtPluginMap.clear();

    _auth = new CoreCntrl_Auth();

    _mediaManager = new CNTRL_MediaManager(this);

    // Init a new session, since the video manager and audio manager
    // both need a session object (I think?)
    // will need to call setSession() again
    // after this init is complete to properly propagate the sessionChanged signal
    //
    // setSession("Default");
    _session = _mediaManager->getSession("Default");

    _videoManager = new CNTRL_Video(this);
    _audioManager = new CNTRL_Audio(this);

    _pluginManager = new CNTRL_PluginManager(this);

    // TODO FIXME: MOVE This out.. to python? not sure
    _pluginManager->registerVideoProbe(new CNTRL_FrameRateProbe());

    _mediaServiceController = new CNTRL_MediaServiceController(this);

    _serverManager = new CNTRL_Server(this);

    initConnections();
    initCommands();
}


CNTRL_MainController::~CNTRL_MainController()
{
}


void
CNTRL_MainController::initConnections()
{
    RLP_LOG_DEBUG("")

    connect(
        _session.get(),
        SIGNAL(frameCountChanged(qlonglong)),
        _videoManager,
        SLOT(onFrameCountChanged(qlonglong))
    );

    connect(
        _session.get(),
        &DS_Session::trackDataChanged,
        this,
        &CNTRL_MainController::onTrackDataChanged
    );

    connect(
        _mediaManager,
        &CNTRL_MediaManager::nodeDataReady,
        _audioManager->mgr(),
        &AUDIO_Manager::onNodeDataReady
    );
}


void
CNTRL_MainController::initCommands()
{
    RLP_LOG_DEBUG("")

    registerCommand(new CNTRL_TogglePlayStateCommand(this));
    registerCommand(new CNTRL_ScrubCommand(this));
}


void
CNTRL_MainController::licenseCheck()
{
    return _serverManager->licenseCheck();
}


void
CNTRL_MainController::emitStartupReady(int delay)
{
    RLP_LOG_DEBUG(delay)

    QTimer::singleShot(delay, this, SLOT(_emitStartupReady()));
}


bool
CNTRL_MainController::setSession(QString name)
{
    RLP_LOG_DEBUG(name)

    bool result = false;

    DS_SessionPtr session = _mediaManager->getSession(name);
    if ((session != nullptr) && (_session != session)) {
        _session = session;
        result = true;

        emit sessionChanged(session);
    }

    return result;
}




void
CNTRL_MainController::show()
{
}


void
CNTRL_MainController::playPause()
{
    if (_playbackState == CNTRL_MainController::PAUSED) {
        play();

    } else {
        pause();
    }
}


void
CNTRL_MainController::play(bool emitAction)
{
    _playbackState = CNTRL_MainController::PLAYING;

    if (emitAction)
    {

        QVariantMap kwargs;
        emit syncAction("play", kwargs);
    }

    emit playStateChanged((int)_playbackState);

    _videoManager->resume();
    _audioManager->resume();

}


void
CNTRL_MainController::pause(qlonglong frameNum, bool emitAction)
{
    _playbackState = CNTRL_MainController::PAUSED;

    int currFrame = _videoManager->currentFrameNum() - 1;

    _videoManager->stop(currFrame);
    _audioManager->stop(currFrame);

    if (frameNum != -1)
    {
        gotoFrame(frameNum, true, true, false);
    }

    emit playStateChanged((int)_playbackState);


    // _audioManager->suspend();
    // _audioManager->updateToFrame(currFrame);

    if (emitAction)
    {
        QVariantMap kwargs;
        kwargs.insert("frame", (qlonglong)currentFrameNum());
        emit syncAction("pause", kwargs);
    }

}


void
CNTRL_MainController::clearAll(bool emitAction)
{
    _videoManager->clearAll();
    _session->clear();
    gotoFrame(0, false);

    if (emitAction)
    {
        QVariantMap kwargs;
        emit syncAction("clearAll", kwargs);
    }
}


void
CNTRL_MainController::gotoFrame(qlonglong frameNum, bool recenter, bool schedule, bool emitAction)
{
    if (!_session->isFrameValid(frameNum)) {
        RLP_LOG_ERROR("invalid frame: " << frameNum);
        return;
    }

    RLP_LOG_DEBUG(frameNum << " " << recenter << " " << schedule);

    _audioManager->updateToFrame(frameNum, recenter);
    _videoManager->updateToFrame(frameNum, recenter, schedule);

    if (emitAction)
    {
        QVariantMap kwargs;
        kwargs.insert("frame", (qlonglong)frameNum);
        kwargs.insert("recenter", recenter);

        emit syncAction("gotoFrame", kwargs);
    }
}


void
CNTRL_MainController::setInFrame(qlonglong frameNum, bool clearCache, bool emitAction)
{
    RLP_LOG_DEBUG(frameNum);

    _playbackMode->setInFrame(frameNum);
    session()->setInFrame(frameNum);

    // Clamp current frame if outside in/out point
    if (frameNum > currentFrameNum())
    {
        gotoFrame(frameNum, true, true);

    } else
    {

        // Audio needs to be rebuffered
        // can be optimized in future by detecting whether in/out points are cached or not
        _audioManager->clearAll();

        if (clearCache)
        {
            _videoManager->clearAll(false);
        }
    }

    if (emitAction)
    {
        QVariantMap kwargs;
        kwargs.insert("frame", frameNum);
        emit syncAction("setInFrame", kwargs);
    }
}


void
CNTRL_MainController::setOutFrame(qlonglong frameNum, bool clearCache, bool emitAction)
{
    RLP_LOG_DEBUG(frameNum);

    _playbackMode->setOutFrame(frameNum);
    session()->setOutFrame(frameNum);

    // Clamp current frame if outside in/out point
    if (frameNum < currentFrameNum())
    {
        gotoFrame(frameNum, true, true);

    }
    else
    {
        // Audio needs to be rebuffered
        // can be optimized in future by detecting whether in/out points are cached or not
        _audioManager->clearAll();

        if (clearCache) {
            _videoManager->clearAll(false);
        }
    }

    if (emitAction)
    {
        QVariantMap kwargs;
        kwargs.insert("frame", frameNum);
        emit syncAction("setOutFrame", kwargs);
    }
}


void
CNTRL_MainController::setVolume(float vol)
{
   _audioManager->setVolume(vol);

   emit volumeChanged(vol);
}


qlonglong
CNTRL_MainController::currentFrameNum()
{
    return _videoManager->currentFrameNum();
}


DS_PlaybackMode*
CNTRL_MainController::playbackMode()
{
    return _playbackMode;
}


//
// RPC
//

void
CNTRL_MainController::emitSyncAction(QString action, QVariantMap kwargs)
{
    // RLP_LOG_DEBUG(action)

    emit syncAction(action, kwargs);
}


void
CNTRL_MainController::emitToolCreated(QString toolName, GUI_ItemBase* tool)
{
    // RLP_LOG_DEBUG(toolName)

    QVariantMap toolInfo;
    QVariant toolW;
    toolW.setValue(tool);

    toolInfo.insert("name", toolName);
    toolInfo.insert("tool", toolW);

    emit toolCreated(toolInfo);
}


void
CNTRL_MainController::handleRemoteCall(QVariantMap msgObj)
{
    // RLP_LOG_VERBOSE(msgObj)

    /*
    {
        'method': 'method_string',
        'args': [],
        'kwargs': {},
        'session': {
            'kwargs': {},
        }
    }
    */

    emit remoteCall(msgObj);

}


//
//
//

bool
CNTRL_MainController::registerEventPlugin(DS_EventPlugin* plugin)
{
    // RLP_LOG_DEBUG(plugin->name());

    _evtPluginMap.insert(plugin->name(), plugin);
    _plugins.push_back(plugin);
    return true;
}


QVariantList
CNTRL_MainController::getEventPluginNames()
{
    QVariantList names;

    for (int i=0; i != _plugins.size(); ++i) {
        names.push_back(_plugins[i]->name());
    }

    return names;
}


DS_EventPlugin*
CNTRL_MainController::getEventPluginByName(QString name)
{
    return _evtPluginMap.value(name);
}


void
CNTRL_MainController::fullRefresh()
{
    RLP_LOG_DEBUG("")

    _videoManager->clearAll(false);
    gotoFrame(currentFrameNum());
    _videoManager->update();
}


void
CNTRL_MainController::onTrackDataChanged(QString eventName, QString trackUuidStr, QVariantMap trackData)
{
    RLP_LOG_DEBUG(eventName)

    if (eventName == "set_enabled")
    {
        fullRefresh();
    }
}


bool
CNTRL_MainController::keyPressEventInternal(QKeyEvent* event)
{
    // RLP_LOG_DEBUG(event)

    bool result = false;

    // RLP_LOG_DEBUG(event->key() << " " << event->modifiers());

    for (int i=0; i != _plugins.size(); ++i) {
        result = _plugins[i]->keyPressEventInternal(event);
        if (result) {
            break;
        }
    }

    return result;
}


bool
CNTRL_MainController::keyReleaseEventInternal(QKeyEvent* event)
{
    bool result = false;

    // RLP_LOG_DEBUG("");

    for (int i=0; i != _plugins.size(); ++i) {
        result = _plugins[i]->keyReleaseEventInternal(event);
        if (result) {
            break;
        }
    }


    return result;
}


void
CNTRL_MainController::forceQuit()
{
    RLP_LOG_INFO("")

    PY_Interp::call("revlens.service.iosched.force_quit");

    exit(0);
}