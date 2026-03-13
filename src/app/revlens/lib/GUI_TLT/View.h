
#ifndef REVLENS_GUI_TLT_VIEW_H
#define REVLENS_GUI_TLT_VIEW_H

#include "RlpRevlens/GUI_TLT/Global.h"
#include "RlpRevlens/GUI_TLT/MediaManager.h"

#include "RlpGui/BASE/ItemBase.h"

#include "RlpRevlens/DS/SyncProbe.h"

#include "RlpRevlens/GUI_TL2/ViewBase.h"
#include "RlpRevlens/GUI_TL2/Playhead.h"
#include "RlpRevlens/GUI_TL2/SyncHandler.h"


class GUI_TLT_View : public GUI_TL2_ViewBase {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER

    GUI_TLT_View(CNTRL_MainController* cntrl, GUI_ItemBase* parent);
    ~GUI_TLT_View();

    GUI_ItemBase* timelineArea() { return this; }

    int timelineWidth() { return width(); }
    int timelineHeight() { return height(); }

    qlonglong getFrameAtXPos(int xpos);
    float getXPosAtFrame(qlonglong frame);

    void setPixelsPerFrame(float width);

    void onParentSizeChanged(qreal nwidth, qreal nheight);

    void updateWidth(int width); // rescale pixelsPerFrame, resize
    void updateInternal();
    void updateIfNecessary();

    void paint(GUI_Painter* painter);

public: // DS_SyncProbe

    void syncUpdate(qlonglong frameNum); // called during playback



public slots:

    static GUI_TLT_View*
    new_GUI_TLT_View(GUI_ItemBase* parent);

    GUI_ItemBase* bookends() { return this ;}
    GUI_TL2_Playhead* playhead() { return _playhead; }
    GUI_TLT_MediaManager* mediaManager() { return _mmgr; }


    qlonglong currFrame() { return _currFrame; }

    qlonglong frameStart() { return _frameStart; }
    void setInFrame(qlonglong frame);
    void setFrameStart(qlonglong frame) { setInFrame(frame); }

    qlonglong frameEnd() { return _frameEnd; }
    void setOutFrame(qlonglong frame);
    void setFrameEnd(qlonglong frame) { setOutFrame(frame); }

    qlonglong frameCount() { return _frameCount; }
    void setFrameCount(qlonglong frameCount);

    void setShowMediaToolTip(bool doShow) { _showMediaToolTip = doShow; }
    bool showMediaToolTip() { return _showMediaToolTip; }

    void onPlayheadStartScrub(int xpos);
    void onPlayheadScrubbing(int xpos);
    void onPlayheadEndScrub(int xpos);
    void onPlayheadRightClick(QPointF pos);

    void onFrameCountChanged(qlonglong frameCount);
    void onContextMenuItemSelected(QVariantMap itemInfo);
    void onSessionPropertyChanged(QString key, QVariant val);

    int timelineInputMode() { return GUI_TL2_ViewBase::TL_MODE_SCRUB; }

private:
    int _twidth;

    qlonglong _frameStart;
    qlonglong _frameEnd;
    qlonglong _frameCount;

    qlonglong _currFrame;

    float _pixelsPerFrame;

    GUI_MenuPane* _contextMenu;
    GUI_TL2_Playhead* _playhead;
    GUI_TLT_MediaManager* _mmgr;
    GUI_TL2_SyncHandler* _syncHandler;

    bool _showMediaToolTip;
    bool _bookendsOn;
};

#endif
