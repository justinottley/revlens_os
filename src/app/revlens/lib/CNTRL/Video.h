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
#include "RlpRevlens/DS/ControllerBase.h"

#include "RlpRevlens/VIDEO/LookaheadCache.h"
#include "RlpRevlens/VIDEO/SyncTarget.h"
#include "RlpRevlens/VIDEO/ClockBase.h"

#include "RlpRevlens/DISP/GLView.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"
#include "RlpRevlens/DISP/GLPaintEngineMultiTex.h"


class REVLENS_CNTRL_API CNTRL_Video : public VIDEO_SyncTarget {
    Q_OBJECT

signals:
    void displayDeregistered(DISP_GLView* display);
    void displayRegistered(DISP_GLView* display);
    void displayContextMenuItemSelected(QVariantMap item);
    void frameDisplayed(qlonglong frameNum);

public:
    RLP_DEFINE_LOGGER

    static QVariantList _VIDEO_SOURCES;

    CNTRL_Video(DS_ControllerBase* controller, QString name="Media");

    void registerEventPlugin(DS_EventPlugin* plugin);
    void registerPaintEngine(QString name, PaintEngineCreateFunc func);

    void setTimeline(DS_SyncProbe* timeline) { _timeline = timeline; }

    bool registerProbe(DS_SyncProbe* probe);
    bool deregisterProbe(DS_SyncProbe* probe);

    
public slots:

    void registerDisplay(DISP_GLView* display, bool autoSetPaintEngine=true);
    bool deregisterDisplay(DISP_GLView* display);

    void registerDisplayContextMenuItem(QVariantMap cmiInfo);

    bool registerCompositePlugin(DISP_CompositePlugin* compositePlugin);
    CompositePluginMap* getCompositePlugins() { return _compositePluginMap; }

    void update();

    void stop(int stopFrame=INT_MIN);
    void resume();

    QString name() { return _name; }
    int idx() { return _idx; }

    float getTargetFrameRate();
    void setTargetFrameRate(float frameRate);

    QList<DISP_GLView*> getDisplays() { return _displays; }

    // for python binding
    int getDisplayCount() { return _displays.size(); }
    DISP_GLView* getDisplayByIdx(int idx) { return _displays.at(idx); }
    DISP_GLView* getDisplayByUuid(QString uuidStr);

    qlonglong currentFrameNum() { return _currentFrame; }
    DS_FrameBufferPtr currentFrameBuffer() { return _fbuf; }

    // Playback mode
    //
    DS_PlaybackMode* playbackMode() { return _playbackMode; }
    void setLoopMode(DS_PlaybackMode::LoopMode loopMode);
    void setDirectionMode(DS_PlaybackMode::DirectionMode dirMode);

    DS_PlaybackMode::DirectionMode currentDirection() { return _direction; }

    VIDEO_LookaheadCache* cache() { return _cache; }

    void displayNextFrame();
    void updateToFrame(qlonglong frameNum, bool recenter=false, bool schedule=false);
    void updateToFrameAsync(qlonglong frameNum, bool recenter=false);

    void probeSyncUpdateImmediate(qlonglong frameNum);

    void onScrubStart(qlonglong frameNum);
    void onScrubEnd(qlonglong frameNum);
    void onFrameRead(MEDIA_ReadEvent* fre);
    void onFrameCountChanged(qlonglong frameNum);
    void onInFrameChanged(qlonglong inFrame);

    void onFullscreenStateChanged(bool enabled, QString displayUuid);

    void clearAll(bool resetCurrFrame=true);
    void recenter(qlonglong frameNum) { _cache->recenter(frameNum); }

    bool isCached(qlonglong frameNum) { return _cache->isCached(frameNum); }
    bool isScrubbing() { return _scrubbing; }

    void startPreroll() { _prerollNum = 0; }
    void setPrerollFrameCount(int frameCount);
    int prerollFrameNum() { return _prerollNum; }
    int prerollFrameCount() { return _prerollCount; }
    bool prerollEnabled() { return _prerollCount > 0; }

private:
    void initFbuf();

private:
    static int _INSTANCE_COUNT;

    int _idx; // to distinguish between multiple Video controllers (e.g., standalone streaming)
    QString _name;

    DS_ControllerBase* _controller;
    DS_FrameBufferPtr _fbuf;
    DS_FrameBufferPtr _blackFbuf;

    DS_SyncProbe* _timeline;
    
    QList<DISP_GLView*> _displays;
    QList<DS_SyncProbe*> _probes;

    QList<QVariantMap> _contextMenuItems;
    PaintEngineCreateMap _peCreateMap;

    CompositePluginMap* _compositePluginMap;

    DS_PlaybackMode* _playbackMode;
    DS_PlaybackMode::DirectionMode _direction;

    VIDEO_LookaheadCache* _cache;
    VIDEO_ClockBase* _avclock;

    qlonglong _currentFrame;

    bool _scrubbing;
    qlonglong _prerollNum;
    qlonglong _prerollCount;
};


#endif
