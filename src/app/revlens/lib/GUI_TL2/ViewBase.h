
#ifndef REVLENS_GUI_TL2_VIEWBASE_H
#define REVLENS_GUI_TL2_VIEWBASE_H


#include "RlpRevlens/GUI_TL2/Global.h"

#include "RlpGui/BASE/ItemBase.h"

#include "RlpRevlens/DS/SyncProbe.h"
#include "RlpRevlens/CNTRL/MainController.h"

class GUI_TL2_ViewBase : public GUI_ItemBase, public DS_SyncProbe {
    Q_OBJECT

signals:

    void startScrub(qlonglong frame);
    void updateToFrame(qlonglong frame);
    void endScrub(qlonglong frame);

public:

    enum TimelineInputMode {
        TL_MODE_SCRUB,
        TL_MODE_EDIT
    };


public:

    GUI_TL2_ViewBase(CNTRL_MainController* cntrl, GUI_ItemBase* parent);

    CNTRL_MainController* controller() { return _controller; }

    virtual GUI_ItemBase* timelineArea() { return nullptr; }

    virtual int timelineWidth() { return 0; }
    virtual int timelineHeight() { return 0; }

    virtual qlonglong getFrameAtXPos(int xpos) { return 1; }
    virtual float getXPosAtFrame(qlonglong frame) { return 0; }

    virtual qlonglong frameStart() { return 0; }
    virtual qlonglong frameEnd() { return 0; }

    virtual qlonglong currFrame() { return 0; }


public slots:

    virtual void onPlayheadStartScrub(int xpos) {}
    virtual void onPlayheadEndScrub(int xpos) {}
    virtual void onPlayheadScrubbing(int xpos) {}

protected:
    CNTRL_MainController* _controller;


};

#endif
