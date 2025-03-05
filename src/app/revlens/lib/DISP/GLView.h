//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//


#ifndef REVLENS_DISP_GLVIEW_H
#define REVLENS_DISP_GLVIEW_H

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/MENU/MenuPane.h"
#include "RlpGui/BASE/GLItem.h"

#include "RlpRevlens/DISP/Global.h"
#include "RlpRevlens/DISP/FrameRateTimer.h"
#include "RlpRevlens/DISP/GLPaintEngine2D.h"

#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/EventPlugin.h"

class REVLENS_DISP_API DISP_GLView : public GUI_GLItem {
    Q_OBJECT

signals:

    void displayDestroyed(DISP_GLView* view);
    void displayZoomChanged(float zoom);
    void fullscreenEnabled(bool isEnabled, QString uuidStr);

    void panPosChanged(QPointF pan); // to emit to display engine (potentially across threads)
    void zoomPosChanged(int zoomDiffX);
    void onPanZoomEnd();

    void hoverEnter();
    void hoverLeave();
    void hoverMove();

    void grabImageReady(QImage img);
    void paintEngineCreated(DISP_GLPaintEngine2D* pe);


public:
    RLP_DEFINE_LOGGER

    DISP_GLView(GUI_ItemBase* parent=nullptr, int width=853, int height=480);
    ~DISP_GLView();

    void initPaintEngines(PaintEngineCreateMap peMap);

    DS_FrameBufferPtr presentNextFrame(long frameNum=0);
    void setFrameBuffer(DS_FrameBufferPtr fbuf);
    DS_FrameBufferPtr frameBuffer() { return _currFrame; }

    bool event(QEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);
    void hoverMoveEvent(QHoverEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    // void update();

    void initGL();
    void paintGL();
    void paint(GUI_Painter* painter);

public slots: // Public API

    // qreal width();
    // qreal height();
    QString uuid() { return _uuidStr; }

    float getFpsAvg() { return _frameRateTimer.fpsAverage(); }
    QPointF getPanPercent();
    void setPanPos(QPointF pan) { emit panPosChanged(pan); }

    float getZoom();

    GUI_MenuPane* contextMenu() { return _contextMenu; }

    // connector from paint engine
    void onPaintEngineDisplayZoomChanged(float zoom) { emit displayZoomChanged(zoom); }

    void onParentSizeChanged(qreal width, qreal height);

    void grabImage();

    void enableFullscreen();
    void disableFullscreen();

    void setEnabled(bool isEnabled) { _enabled = isEnabled; }
    bool isEnabled() { return _enabled; }
    bool isFullscreen() { return _fullscreen; }

public slots: // Plugins

    bool registerEventPlugin(DS_EventPlugin* plugin);
    DS_EventPlugin* getEventPluginByName(QString name);

    void setSoloPluginByIdx(int idx);
    void setSoloPluginByName(QString name);
    void resetSoloPlugin(QString name, bool enabled=false);

public slots: // Paint engine

    void setCurrentPaintEngine(QString name);

    DISP_GLPaintEngine2D* getOrInitPaintEngine(QString name);
    DISP_GLPaintEngine2D* getCurrentPaintEngine();


protected:

    QString _uuidStr;
    bool _enabled;
    bool _fullscreen;
    
    DS_FrameBufferPtr _currFrame;

    QList<DS_EventPlugin*> _plugins;
    int _soloPluginIdx; // index of soloed plugin in plugin lists

    DISP_FrameRateTimer _frameRateTimer;

    GUI_MenuPane* _contextMenu;

    bool _readPixels;

    DISP_GLPaintEngine2D* _engine;
    QString _currPaintEngine;

    // constructed paint engines
    QMap<QString, DISP_GLPaintEngine2D*> _paintEngineMap;

    // registered paint engines, not constructed yet,
    // construction must happen on rendering thread
    PaintEngineCreateMap _peInitMap; 
};

Q_DECLARE_METATYPE(DISP_GLView*)

#endif
