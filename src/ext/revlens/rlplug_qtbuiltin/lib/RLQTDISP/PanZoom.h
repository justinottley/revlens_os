//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef rlplug_QTBUILTIN_DISP_PANZOOM_H
#define rlplug_QTBUILTIN_DISP_PANZOOM_H

#include "RlpCore/LOG/Logging.h"

#include "RlpRevlens/DS/EventPlugin.h"
#include "RlpRevlens/DISP/GLView.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"
#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpExtrevlens/RLQTDISP/Global.h"


class EXTREVLENS_RLQTDISP_API RLQTDISP_PanZoom : public DS_EventPlugin {
    Q_OBJECT

signals:

    void panChanged(QPointF panDiffPos);
    void zoomChanged(int zoomDiffX);

    void panZoomEnd();

public:
    RLP_DEFINE_LOGGER


    RLQTDISP_PanZoom();

    DS_EventPlugin* duplicate();

    bool setDisplay(DISP_GLView* display);

    bool mousePressEventInternal(QMouseEvent* event);
    bool mouseMoveEventInternal(QMouseEvent* event);
    bool mouseReleaseEventInternal(QMouseEvent* event);

    // void paintGLPostDraw(QPainter& painter, const QVariantMap& metadata);
    // void resizeGLInternal(int width, int height);


private:

    bool inPan(QMouseEvent* event);
    bool inZoom(QMouseEvent* event);


private:

    QPointF _panStartPos;
    bool _inPan;

    int _zoomStartX;
    bool _inZoom;

};


Q_DECLARE_METATYPE(RLQTDISP_PanZoom *)

#endif