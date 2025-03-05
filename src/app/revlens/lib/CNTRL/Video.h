//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_CNTRL_VIDEO_H
#define REVLENS_CNTRL_VIDEO_H

#include "RlpRevlens/CNTRL/Global.h"

#include "RlpRevlens/DS/SyncProbe.h"
#include "RlpRevlens/DS/PlaybackMode.h"
#include "RlpRevlens/DS/EventPlugin.h"
#include "RlpRevlens/DS/FrameBuffer.h"

#include "RlpRevlens/VIDEO/LookaheadCache.h"
#include "RlpRevlens/VIDEO/SyncTarget.h"
#include "RlpRevlens/VIDEO/ClockBase.h"

#include "RlpRevlens/DISP/GLView.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"


class CNTRL_MainController;

class REVLENS_CNTRL_API CNTRL_Video : public VIDEO_SyncTarget {
    Q_OBJECT

signals:
    void displayDeregistered(DISP_GLView* display);
    void displayContextMenuItemSelected(QVariantMap item);

public:
    RLP_DEFINE_LOGGER

    CNTRL_Video(CNTRL_MainController* controller);

    void registerEventPlugin(DS_EventPlugin* plugin);
    void registerPaintEngine(QString name, PaintEngineCreateFunc func);

    void setTimeline(DS_SyncProbe* timeline) { _timeline = timeline; }

    bool registerProbe(DS_SyncProbe* probe);
    bool deregisterProbe(DS_SyncProbe* probe);

    
public slots:

    void registerDisplay(DISP_GLView* display);
    bool deregisterDisplay(DISP_GLView* display);

    void registerDisplayContextMenuItem(QVariantMap cmiInfo);

    void update();

    void stop(int stopFrame=INT_MIN);
    void resume();

    float getTargetFrameRate();
    void setTargetFrameRate(float frameRate);

    QList<DISP_GLView*> getDisplays() { return _displays; }

    // for python binding
    int getDisplayCount() { return _displays.size(); }
    DISP_GLView* getDisplayByIdx(int idx) { return _displays.at(idx); }

    qlonglong currentFrameNum() { return _currentFrame; }
    DS_PlaybackMode* playbackMode() { return _playbackMode; }
    VIDEO_LookaheadCache* cache() { return _cache; }

    void displayNextFrame();
    void updateToFrame(qlonglong frameNum, bool recenter=false, bool schedule=false, bool scheduleForceRead=false);
    void updateToFrameAsync(qlonglong frameNum, bool recenter=false, bool forceRead=false);

    void probeSyncUpdateImmediate(qlonglong frameNum);

    void onScrubStart(qlonglong frameNum);
    void onScrubEnd(qlonglong frameNum);
    void onFrameRead(MEDIA_ReadEvent* fre);
    void onFrameCountChanged(qlonglong frameNum);

    void onFullscreenStateChanged(bool enabled, QString displayUuid);

    void clearAll(bool resetCurrFrame=true);
    void recenter(qlonglong frameNum) { _cache->recenter(frameNum); }

    bool isCached(qlonglong frameNum) { return _cache->isCached(frameNum); }
    bool isScrubbing() { return _scrubbing; }

signals:

    void frameDisplayed(qlonglong frameNum);

private:
    CNTRL_MainController* _controller;
    DS_FrameBufferPtr _fbuf;

    DS_SyncProbe* _timeline;
    
    QList<DISP_GLView*> _displays;
    QList<DS_SyncProbe*> _probes;

    QList<QVariantMap> _contextMenuItems;
    PaintEngineCreateMap _peCreateMap;

    DS_PlaybackMode* _playbackMode;
    VIDEO_LookaheadCache* _cache;
    VIDEO_ClockBase* _avclock;

    qlonglong _currentFrame;

    bool _scrubbing;
    qlonglong _prerollNum;
    qlonglong _prerollCount;
};


#endif
