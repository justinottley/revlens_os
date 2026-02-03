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

#include "RlpRevlens/DISP/CompositePlugin.h"

#include "RlpRevlens/DS/FrameBuffer.h"
#include "RlpRevlens/DS/EventPlugin.h"

typedef QList<QString>::iterator PluginIterator;

class REVLENS_DISP_API DISP_GLView : public GUI_GLItem {
    Q_OBJECT

signals:

    void displayDestroyed(DISP_GLView* view);
    void displayZoomEnd(float zoom);
    void displayPanEnd(QPointF pan);

    void fullscreenEnabled(bool isEnabled, QString uuidStr);

    void panPosChanged(QPointF pan); // to emit to display engine (potentially across threads)
    void zoomPosChanged(int zoomDiffX);
    void onPanZoomEnd();

    void hoverEnter();
    void hoverLeave();
    void hoverMove();

    void grabImageReady(QImage img);
    void paintEngineCreated(DISP_GLPaintEngine2D* pe);

    void compositeModeChanged(QString newMode);

public:
    RLP_DEFINE_LOGGER

    DISP_GLView(GUI_ItemBase* parent=nullptr, int width=853, int height=480);
    ~DISP_GLView();

    void initPaintEngines(PaintEngineCreateMap peMap);

    void setIdx(int idx);

    DS_FrameBufferPtr presentNextFrame(long frameNum=0);
    void setFrameBuffer(int videoSourceIdx, DS_FrameBufferPtr fbuf);
    DS_FrameBufferPtr frameBuffer() { return _currFrame; }

    bool event(QEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);
    void hoverMoveEvent(QHoverEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void touchEvent(QTouchEvent* event);
    void onTabletEvent(QTabletEvent* event); // not a native Qt event handler, received via sig-slot connection

    void initGL();
    void paintGL();
    void paint(GUI_Painter* painter);


public slots: // Public API

    void setDevicePixelRatio(qreal dpr);

    void setVideoSourceIdx(int videoSourceIdx);
    int videoSourceIdx() { return _videoSourceIdx; }

    QString uuid() { return _uuidStr; }
    int idx() { return _idx; }

    float getFpsAvg() { return _frameRateTimer.fpsAverage(); }
    QPointF getPanPercent();
    void setPanPos(QPointF pan) { emit panPosChanged(pan); }

    float getZoom();
    float getDisplayZoom();
    float getRelZoom();

    void resetZoom();
    void resetPan();

    QVariantList getCurrentFramebufferList(); // python binding friendly return type

    GUI_MenuPane* contextMenu() { return _contextMenu; }

    void onParentSizeChanged(qreal width, qreal height);

    void grabImage();

    void enableFullscreen();
    void disableFullscreen();

    void setEnabled(bool isEnabled);
    void enable() { setEnabled(true); } // for easier use via QTimer::singleShot
    void disable() { setEnabled(false); } // for easier use via QTimer::singleShot
    bool isEnabled() { return _enabled; }
    bool isFullscreen() { return _fullscreen; }

    void setCompositeMode(QString compositeModeName);


public slots: // Plugins

    bool registerEventPlugin(DS_EventPlugin* plugin);
    DS_EventPlugin* getEventPluginByName(QString name);
    bool setPluginIndex(QString name, int posIdx);

    bool setSoloPluginByName(QString name);
    bool resetSoloPlugin(QString name, bool enabled=false);

    QVariantMap* getDisplayMetadata() { return _displayMetadata; }
    void setDisplayMetadata(QString key, QVariant val);
    void clearDisplayMetadata(QString prefix="");

    void registerCompositePlugins(CompositePluginMap* cpm);


public slots: // Paint engine

    void setCurrentPaintEngine(QString name);

    DISP_GLPaintEngine2D* getOrInitPaintEngine(QString name);
    DISP_GLPaintEngine2D* getCurrentPaintEngine();

    void onSplitterDragStarted();
    void onSplitterDragEnded();

protected:

    QString _uuidStr;
    int _idx;
    int _videoSourceIdx;

    bool _enabled;
    bool _drawEnabled; // optimization for linux, dont invoke paint engine
    bool _fullscreen;

    DS_FrameBufferPtr _currFrame;

    QHash<int, DS_FrameBufferPtr> _videoSourceFbufMap;

    QList<QString> _pluginNames; // ordered list of plugins
    QMap<QString, DS_EventPlugin*> _plugins;

    QString _soloPluginName;

    CompositePluginMap* _compositePlugins;

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

    // Per-display metadata that might be needed / useful during IO read,
    // for example composite settings (Tile / Wipe etc)
    //
    QVariantMap* _displayMetadata;
};


Q_DECLARE_METATYPE(DISP_GLView*)

#endif
