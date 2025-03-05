//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_CNTRL_CONTROLLER_H
#define REVLENS_CNTRL_CONTROLLER_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpCore/CNTRL/Auth.h"

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/PlaybackMode.h"
#include "RlpRevlens/DS/Session.h"

class GUI_ItemBase;
class DS_EventPlugin;
class CNTRL_Video;
class CNTRL_Audio;
class CNTRL_MediaManager;
class CNTRL_PluginManager;
class CNTRL_Server;
class CNTRL_MediaServiceController;


// class CNTRL_MainController;
// static CNTRL_MainController* INSTANCE = nullptr;

class REVLENS_CNTRL_API CNTRL_MainController : public DS_ControllerBase {
    Q_OBJECT

signals:
    void startupReady();
    void noticeEvent(QString noticeName, QVariantMap noticeInfo);
    void sessionChanged(DS_SessionPtr session);

public:
    RLP_DEFINE_LOGGER

    enum PlaybackState
    {
        PLAYING,
        PAUSED
    };

    CNTRL_MainController();
    ~CNTRL_MainController();

    void show();
    void licenseCheck();

    bool registerEventPlugin(DS_EventPlugin* plugin);

    // bool insertEventPlugin(QString& name, int pos=-1);

    static CNTRL_MainController* instance() { return _instance; }
    static void setInstance(CNTRL_MainController* instance) { _instance = instance; }

    void gotoFrame(float frameNum) { return gotoFrame((qlonglong)frameNum); }
    void fullRefresh();

public slots:

    bool keyPressEventInternal(QKeyEvent* event);
    bool keyReleaseEventInternal(QKeyEvent* event);

    void onTrackDataChanged(QString trackUuid, QString eventName, QVariantMap data);

public slots:

    // Public API
    //

    void play(bool emitAction=true);
    void pause(qlonglong frame=-1, bool emitAction=true);
    void playPause();
    int getPlaybackState() { return (int)_playbackState; }

    bool isPlaying() {
        return _playbackState == PLAYING;
    }

    bool isPaused() {
        return _playbackState == PAUSED;
    }


    void gotoFrame(qlonglong frameNum, bool recenter=true, bool schedule=false, bool emitAction=true);

    void clearAll(bool emitAction=true);

    qlonglong inFrame() { return _playbackMode->getInFrame(); }
    qlonglong outFrame() { return _playbackMode->getOutFrame(); }

    void setInFrame(qlonglong frameNum, bool clearCache=false, bool emitAction=true);
    void setOutFrame(qlonglong frameNum, bool clearCache=false, bool emitAction=true);

    void setVolume(float vol);

    // Accessors
    //
    CoreCntrl_Auth* loginCntrl() { return _auth; }
    CNTRL_MediaManager* getMediaManager() { return _mediaManager; }
    CNTRL_Video* getVideoManager() { return _videoManager; }
    CNTRL_Audio* getAudioManager() { return _audioManager; }
    CNTRL_PluginManager* getPluginManager() { return _pluginManager; }
    CNTRL_Server* getServerManager() { return _serverManager;}
    CNTRL_MediaServiceController* mediaServiceCntrl() { return _mediaServiceController; }

    // Plugins
    //
    QVariantList getEventPluginNames();
    DS_EventPlugin* getEventPluginByName(QString name);

    DS_SessionPtr session() { return _session; }
    bool setSession(QString name);

    DS_PlaybackMode* playbackMode();

    qlonglong currentFrameNum();

    void forceQuit();

    // RPC
    //
    void handleRemoteCall(QVariantMap msgObj);


public slots:

    void _emitStartupReady() { emit startupReady(); }
    void emitStartupReady(int delay=100);

    void emitSyncAction(QString action, QVariantMap kwargs);
    void emitToolCreated(QString toolName, GUI_ItemBase* tool);
    void emitNoticeEvent(QString noticeName, QVariantMap evtInfo)
    {
        emit noticeEvent(noticeName, evtInfo);
    }
    // void sendCall(QString method, QVariantList args, QVariantMap kwargs, QVariantMap skwargs);



signals:

    void mediaLoaded(QVariantMap mediaIn);

    void playStateChanged(int playbackstate);
    void volumeChanged(float vol);
    void receivedStartupReady();

    void remoteCall(QVariantMap msgObj);
    void syncAction(QString action, QVariantMap kwargs);


    // A mechanism to allow plugins to know when a tool has been created
    // to attach additional GUIs, etc (use case, viewer toolbar)
    //
    void toolCreated(QVariantMap toolInfo);

private:
    void initConnections();
    void initCommands();

    static CNTRL_MainController* _instance;

    DS_PlaybackMode* _playbackMode;

    DS_SessionPtr _session;

    CoreCntrl_Auth* _auth;

    CNTRL_Audio* _audioManager;
    CNTRL_Video* _videoManager;

    CNTRL_MediaManager* _mediaManager;
    CNTRL_PluginManager* _pluginManager;
    CNTRL_Server* _serverManager;
    CNTRL_MediaServiceController* _mediaServiceController;

    PlaybackState _playbackState;

    QMap<QString, DS_EventPlugin*> _evtPluginMap;
    QList<DS_EventPlugin*>  _plugins;

};



#endif
