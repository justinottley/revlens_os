//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_CONTROLLERBASE_H
#define REVLENS_DS_CONTROLLERBASE_H

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/PlaybackModeSession.h"

#include "RlpCore/CNTRL/ControllerBase.h"

class DS_EventPlugin;

class CNTRL_Video;
class CNTRL_Audio;
class CNTRL_Audio;

class REVLENS_DS_API DS_ControllerBase : public CoreCntrl_ControllerBase {
    Q_OBJECT

signals:
    void dataReady(QVariantMap info);
    void statusEvent(QString event);
    void sessionChanged(DS_SessionPtr session);
    void playStateChanged(int playbackstate);

public:
    enum PlaybackState
    {
        PLAYSTATE_UNKNOWN,
        PLAYSTATE_PLAYING,
        PLAYSTATE_PAUSED
    };

    DS_ControllerBase();
    virtual ~DS_ControllerBase() {}

    virtual DS_SessionPtr session() { return nullptr; }
    virtual bool event(QEvent* event);
    virtual void licenseCheck() {}
    virtual qlonglong currentFrameNum() { return 0; }

    virtual DS_PlaybackMode* playbackMode() { return _playbackMode; }

    virtual QVariantList getEventPluginNames() { return QVariantList(); }
    virtual DS_EventPlugin* getEventPluginByName(QString plugName) { return nullptr; }

    // per-display metadata (like Composite mode)
    virtual QVariantMap getDisplayMetadata() { return QVariantMap(); }

    virtual CNTRL_Video* getVideoManager() { return nullptr; }
    virtual CNTRL_Audio* getAudioManager() { return nullptr; }


public slots:

    virtual void sendCall(QString method, QVariantList args, QVariantMap kwargs) {}
    virtual void emitSyncAction(QString action, QVariantMap kwargs) {}
    void emitStatusEvent(QString message)
    {
        emit statusEvent(message);
    }

    virtual void play(bool emitAction=true) {}
    virtual void pause(qlonglong frame=-1, bool emitAction=true) {}
    virtual void gotoFrame(qlonglong frameNum, bool recenter=true, bool schedule=false, bool emitAction=true) {}

    virtual int getPlaybackState() { return _playbackState; }

    virtual void fullRefresh() {}

protected:

    PlaybackState _playbackState;
    DS_PlaybackMode* _playbackMode;


};

typedef std::shared_ptr<DS_ControllerBase> DS_ControllerPtr;


Q_DECLARE_METATYPE(DS_ControllerPtr)

#endif
