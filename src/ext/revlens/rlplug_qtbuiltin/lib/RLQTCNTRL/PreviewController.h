
#ifndef EXTREVLENS_RLQTCNTRL_PREVIEWCONTROLLER_H
#define EXTREVLENS_RLQTCNTRL_PREVIEWCONTROLLER_H

#include "RlpExtrevlens/RLQTCNTRL/Global.h"

#include "RlpExtrevlens/RLQTDISP/PreviewViewBase.h"

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/Session.h"
#include "RlpRevlens/DS/PlaybackMode.h"
#include "RlpRevlens/DS/Track.h"

#include "RlpRevlens/VIDEO/LookaheadCache.h"
#include "RlpRevlens/VIDEO/SyncTarget.h"
#include "RlpRevlens/VIDEO/ClockBase.h"


class RLQTCNTRL_PreviewController;

class RLQTCNTRL_IdentifyPathTask : public QRunnable {

public:
    RLP_DEFINE_LOGGER

    RLQTCNTRL_IdentifyPathTask(RLQTCNTRL_PreviewController* cntrl, QString path);

    void run();


private:
    RLQTCNTRL_PreviewController* _cntrl;
    QString _path;

};


class RLQTCNTRL_PreviewVideo : public VIDEO_SyncTarget {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTCNTRL_PreviewVideo(DS_ControllerBase* controller);

    qlonglong currentFrameNum() { return _currentFrame; }
    DS_FrameBufferPtr currentFrameBuffer() { return _fbuf; }

    VIDEO_LookaheadCache* cache() { return _cache; }
    DS_PlaybackMode* playbackMode() { return _playbackMode; }

    void setDisplay(RLQTDISP_PreviewViewBase* display);

public slots:

    void displayNextFrame();

    void resume();
    void stop();

    void gotoFrame(qlonglong frame) { _currentFrame = frame; }

private:
    void initFbuf();

protected:

    DS_ControllerBase* _controller;
    DS_PlaybackMode* _playbackMode;

    DS_FrameBufferPtr _fbuf;

    VIDEO_LookaheadCache* _cache;
    VIDEO_ClockBase* _avclock;
    

    qlonglong _currentFrame;

    RLQTDISP_PreviewViewBase* _display;
};


class RLQTCNTRL_PreviewController : public DS_ControllerBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTCNTRL_PreviewController();

public slots:
    DS_SessionPtr session() { return _session; }

    // Type inheritence issue, not using getVideoManager()...
    RLQTCNTRL_PreviewVideo* getVideoMgr() { return _videoManager; }

    void play();
    void pause();

    void loadMediaItem(QString path);

    void onNoticeEvent(QString noteName, QVariantMap noticeInfo);


protected:
    DS_SessionPtr _session;
    RLQTCNTRL_PreviewVideo* _videoManager;

};


#endif
