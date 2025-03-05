//
//  Copyright (c) 2014-2024 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_VIEWER_TIMELINE_H
#define REVLENS_GUI_VIEWER_TIMELINE_H

#include "RlpRevlens/GUI/Global.h"

#include "RlpRevlens/DISP/GLView.h"


class GUI_ViewerItem;

class REVLENS_GUI_API GUI_ViewerTimeline : public DS_EventPlugin {
    Q_OBJECT

signals:
    void startScrub(qlonglong frame);
    void updateToFrame(qlonglong frame);
    void endScrub(qlonglong frame);

public:
    RLP_DEFINE_LOGGER

    GUI_ViewerTimeline();
    DS_EventPlugin* duplicate();

    bool mousePressEventInternal(QMouseEvent* event);
    bool mouseMoveEventInternal(QMouseEvent* event);
    bool mouseReleaseEventInternal(QMouseEvent* event);
    bool hoverMoveEventInternal(QHoverEvent* event);

    // QRectF boundingRect() const;
    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata);

    qlonglong getFrameNum(int xpos);

protected:
    int _circpos;
    bool _chover; // circle hover
    bool _lhover; // line hover
    int _lxpos;

    qreal _hmid;
    qreal _wmax;

    qlonglong _currframe;
    qlonglong _framecount;
    qlonglong _hframe;

    QColor _fgcol;
    QTimer _timer;
};

#endif