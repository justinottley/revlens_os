//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TIMELINE_VIEW_H
#define CORE_GUI_TIMELINE_VIEW_H

#include "RlpRevlens/GUI_TL2/Global.h"

// #include "RlpGui/BASE/ItemBase.h"

#include "RlpGui/BASE/FrameBase.h"

#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpRevlens/DS/SyncProbe.h"

#include "RlpRevlens/GUI_TL2/VerticalScrollBar.h"
#include "RlpRevlens/GUI_TL2/TrackManager.h"
#include "RlpRevlens/GUI_TL2/Track.h"
#include "RlpRevlens/GUI_TL2/Ticks.h"
#include "RlpRevlens/GUI_TL2/ZoomBar.h"
#include "RlpRevlens/GUI_TL2/ToolBar.h"
#include "RlpRevlens/GUI_TL2/Bookends.h"
#include "RlpRevlens/GUI_TL2/Splitter.h"
#include "RlpRevlens/GUI_TL2/Playhead.h"
#include "RlpRevlens/GUI_TL2/Cache.h"
#include "RlpRevlens/GUI_TL2/SyncHandler.h"



class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_ViewTimelineArea : public GUI_ItemBase {

public:
    GUI_TL2_ViewTimelineArea(GUI_TL2_View* track);
    void refresh();
    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

private:
    GUI_TL2_View* _view;

};


class REVLENS_GUI_TL2_API GUI_TL2_View : public GUI_ItemBase, public DS_SyncProbe {
    Q_OBJECT

signals:

    void startScrub(qlonglong frame);
    void updateToFrame(qlonglong frame);
    void endScrub(qlonglong frame);

    void timelineMaxTrackHeightChanged(int maxHeight);


public:

    enum TimelineInputMode {
        TL_MODE_SCRUB,
        TL_MODE_EDIT
    };

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_View(CNTRL_MainController* cntrl, GUI_ItemBase* parent);
    ~GUI_TL2_View();

    void initConnections();
    CNTRL_MainController* controller() { return _controller; }

    void syncUpdate(qlonglong currFrame);
    void syncUpdateImmediate(qlonglong currFrame);

    QRectF boundingRect() const;

    static QQuickItem* create(GUI_ItemBase* parent, QVariantMap toolInfo);

    void paint(GUI_Painter* painter);

public slots:
    
    static GUI_TL2_View* new_GUI_TL2_View(GUI_ItemBase* parent);

    QString uuid() { return _uuidStr; }

    void updateIfNecessary();
    void updateInternal();
    void refresh();

    // Parent dimensions changed
    void onParentSizeChanged(qreal width, qreal height);

    GUI_TL2_ViewTimelineArea* timelineArea() { return _timelineArea; }
    GUI_TL2_TrackManager* trackManager() { return _trackManager; }
    // GUI_Scene* scene() { return _scene; }
    GUI_TL2_ZoomBar* zoomBar() { return _zoomBar; }
    GUI_TL2_Playhead* playhead() { return _playhead; }
    GUI_TL2_Ticks* ticks() { return _ticks; }
    GUI_TL2_ToolBar* buttonBar() { return _toolBar; }

    int timelineWidth();
    int timelineHeight();

    int timelineTrackHeaderWidth();


    int timelineXPosStart() { return _timelineXPosStart; }
    int timelineXPosRightPad() { return _timelineXPosRightPad; }

    int timelineMaxTrackHeight() { return _timelineMaxTrackHeight; }
    void setTimelineMaxTrackHeight(int mheight);
    
    void setTimelineXPosStart(int xpos);

    QRectF timelineClipRect();
    QRegion timelineClipFullHeightRegion();
    QRegion fullRegion();

    QRectF timelineRect();

    qlonglong frameStart() { return _frameStart; }
    void setFrameStart(qlonglong frame);

    qlonglong frameEnd() { return frameStart() + _frameCount; }
    void setFrameEnd(qlonglong frame);

    bool isFocusedOnFrame() { return _isFocusedOnFrame; }
    void setFocusedOnFrame(bool isFocused) { _isFocusedOnFrame = isFocused; }

    float pixelsPerFrame() { return _pixelsPerFrame; }
    void setPixelsPerFrame(float width);

    float zoom() { return _zoom; }
    void setZoom(float zoom) { _zoom = zoom; }
    
    float pan() { return _pan; }
    void setPan(float pan) { _pan = pan; }

    float panStart() { return _panStart; }
    void setPanStart(float panStart) { _panStart = panStart; }

    qlonglong currFrame() { return _currFrame; }
    qlonglong currMouseFrame() { return _playhead->currMouseFrame(); }
    // void setCurrFrame(qlonglong currFrame) { _currFrame = currFrame; }
    
    qlonglong frameCount() { return _frameCount; }
    void setFrameCount(qlonglong frameCount, bool updateBookends=true);

    void setInFrame(qlonglong frame, bool emitSignal=true);
    void setOutFrame(qlonglong frame, bool emitSignal=true);

    float getXPosAtFrame(qlonglong frame);
    qlonglong getFrameAtXPos(int xpos);

    float yscrollPos() { return _yscrollPos; }

    bool isCacheVisible() { return _cache->isVisible(); }
    void setCacheVisible(bool cacheVisible)
    {
        _cache->setVisible(cacheVisible);
        updateIfNecessary();
    }

    bool isTickLinesVisible() { return _ticks->isLinesVisible(); }
    void setTickLinesVisible(bool tickLinesVisible)
    {
        _ticks->setLinesVisible(tickLinesVisible);
        updateIfNecessary();
    }

    bool isTickNumbersVisible() { return _ticks->isNumbersVisible(); }
    void setTickFrameNumbersVisible(bool tickNumsVisible)
    {
        _ticks->setNumbersVisible(tickNumsVisible);
        updateIfNecessary();
    }

    int timelineInputMode() { return _inputMode; }
    void setTimelineInputMode(int mode);


public slots:

    // Internal slots
    //

    void onSplitterMoved(int xpos);

    void onInFrameChanged(qlonglong inFrame);
    void onOutFrameChanged(qlonglong outFrame);

    void onPlayheadStartScrub(int xpos);
    void onPlayheadEndScrub(int xpos);
    void onPlayheadScrubbing(int xpos);

    void onZoomChanged(float zoom, int side, int leftXPos, int rightXPos);
    void onPanChanged(int side, int xDiff);
    
    void onScrollPosChanged(float pos);


public slots:

    void onSessionChanged(DS_SessionPtr session);
    void onSessionCleared();
    
    void onFrameCountChanged(qlonglong frameCount);
    void onNodeDataReady(QVariantMap info);

private:

    // State
    //
    int _timelineXPosStart;
    int _timelineXPosRightPad;
    int _timelineMaxTrackHeight;


    qlonglong _frameCount;
    qlonglong _frameStart; // view specific frame start
    qlonglong _currFrame;

    bool _isFocusedOnFrame;

    float _zoom;

    float _pan;
    float _panStart;
    float _yscrollPos;

    float _pixelsPerFrame;

    TimelineInputMode _inputMode;


private:

    CNTRL_MainController* _controller;

    GUI_TL2_ViewTimelineArea* _timelineArea;
    GUI_TL2_TrackManager* _trackManager;

    // Widgets
    //

    GUI_TL2_VerticalScrollBar* _vbar;
    GUI_TL2_Ticks* _ticks;
    GUI_TL2_ZoomBar* _zoomBar;
    GUI_TL2_ToolBar* _toolBar;
    GUI_TL2_Bookends* _bookends;
    GUI_TL2_Splitter* _splitter;
    GUI_TL2_Playhead* _playhead;
    GUI_TL2_Cache* _cache;
    
    GUI_TL2_SyncHandler* _syncHandler;

    QString _uuidStr;
};

#endif