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

#include "RlpExtrevlens/RLQTDISP/CompositeImageWindow.h"
#include "RlpExtrevlens/RLQTDISP/CompositeWipeSplitter.h"

#include "RlpExtrevlens/RLQTDISP/Global.h"


class EXTREVLENS_RLQTDISP_API RLQTDISP_CompositeTools : public GUI_ItemBase {
    Q_OBJECT

signals:
    void compositeModeChanged(QString mode);

public:
    RLP_DEFINE_LOGGER

    RLQTDISP_CompositeTools(DISP_GLView* parent);

    QRectF boundingRect() const;
    void paint(GUI_Painter* painter);

public slots:

    void setEnabled(bool enabled);
    void onMenuItemSelected(QVariantMap itemData);

protected:
    GUI_IconButton* _modeButton;
    DISP_GLView* _display;
};


class EXTREVLENS_RLQTDISP_API RLQTDISP_Composite : public DS_EventPlugin {
    Q_OBJECT

signals:
    void imageWindowNeeded(QVariantMap* fbufmdata);

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

    void onImageWindowNeeded(QVariantMap* fbufmdata);

    void onCompositeModeChanged(QString);
    void onWipePosChanged(float xpos);
    void onBlendPosChanged(float val);

    void onSessionChanged(DS_SessionPtr session);
    void onSessionCleared();

    void clear();
    void hideTools();
    void setControlsEnabled(bool visible);

private:

    QMap<int, RLQTDISP_CompositeImageWindow*> _imgMap;
    qreal _windowWidth;
    qreal _windowHeight;

    QString _mode;
    bool _visEnabled;

    RLQTDISP_CompositeTools* _tools;
    RLQTDISP_CompositeWipeSplitter* _wsplitter;
    GUI_Slider* _blendSlider;
};


Q_DECLARE_METATYPE(RLQTDISP_Composite *)

#endif