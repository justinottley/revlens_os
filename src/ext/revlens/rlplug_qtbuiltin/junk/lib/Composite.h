//
//  Copyright (c) 2014-2022 Justin Ottley. All rights reserved.
//

#ifndef rlplug_QTBUILTIN_DISP_COMPOSITE_H
#define rlplug_QTBUILTIN_DISP_COMPOSITE_H

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/WIDGET/IconButton.h"
#include "RlpGui/WIDGET/Slider.h"

#include "RlpRevlens/DS/EventPlugin.h"
#include "RlpRevlens/DISP/GLView.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"
#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/DISP/CompositePlugin.h"

#include "RlpExtrevlens/RLQTDISP/CompositeImageWindow.h"

// #include "RlpExtrevlens/RLQTDISP/WipeCompositeSplitter.h"

#include "RlpExtrevlens/RLQTDISP/Global.h"



class EXTREVLENS_RLQTDISP_API RLQTDISP_Composite : public DS_EventPlugin {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER
    
    
    RLQTDISP_Composite();
    
    DS_EventPlugin* duplicate();
    bool setMainController(CNTRL_MainController* controller);
    bool setDisplay(DISP_GLView* display);

    void recalculateImageRects(DS_BufferPtr fbuf, qreal wx, qreal wy, qreal zoom);

    bool mousePressEventInternal(QMouseEvent* event);
    bool mouseMoveEventInternal(QMouseEvent* event);
    bool mouseReleaseEventInternal(QMouseEvent* event);
    
    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& metadata);
    void resizeGLInternal(int width, int height);

public slots:

    // void onImageWindowNeeded(QVariantMap* fbufmdata);

    void onCompositeModeChanged(QString);
    void onWipePosChanged(float xpos);
    void onBlendPosChanged(float val);

    void onSessionChanged(DS_SessionPtr session);
    void onSessionCleared();

    void clear();
    void hideTools();
    void setControlsEnabled(bool visible);

private:

    CompositePluginMap* _plugins;
    QString _currPlugin;

    qreal _windowWidth;
    qreal _windowHeight;

    QString _mode;
    bool _visEnabled;

    // RLQTDISP_CompositeTools* _tools;
    // RLQTGUI_WipeCompositeSplitter* _wsplitter;
    GUI_Slider* _blendSlider;
};


Q_DECLARE_METATYPE(RLQTDISP_Composite *)

#endif