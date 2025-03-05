//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_CNTRL_CONTROLLER_H
#define EXTREVLENS_RLANN_CNTRL_CONTROLLER_H

#include "RlpExtrevlens/RLANN_CNTRL/Global.h"

#include "RlpRevlens/DS/EventPlugin.h"
#include "RlpRevlens/DISP/GLView.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"

#include "RlpRevlens/GUI_TL2/Controller.h"

#include "RlpExtrevlens/RLANN_DS/DrawControllerBase.h"
#include "RlpExtrevlens/RLANN_DS/Annotation.h"
#include "RlpExtrevlens/RLANN_DS/Track.h"
#include "RlpExtrevlens/RLANN_CNTRL/TrackFactoryPlugin.h"


typedef QMap<QString, QPoint> StringPointMap;

class RLANN_CNTRL_FrameProbe;
class RLANN_MP_Handler;

class EXTREVLENS_RLANN_CNTRL_API RLANN_CNTRL_DrawController : public RLANN_DS_DrawControllerBase {
    Q_OBJECT

public:
    static int UPDATE_TIMER_INTERVAL;

    enum PaperTraceMode {
        PAPER_TRACE_ALWAYS_ON,
        PAPER_TRACE_ANNOTATION_ONLY
    };

public:
    RLP_DEFINE_LOGGER
    
    RLANN_CNTRL_DrawController();
    
    static RLANN_CNTRL_DrawController* instance() { return _instance; }
    static void setInstance(RLANN_CNTRL_DrawController* instance) { _instance = instance; }

    virtual DS_EventPlugin* duplicate();

    bool setMainController(CNTRL_MainController* controller);

    void updateDisplay() { _controller->getVideoManager()->update(); }
    bool event(QEvent* event);

    bool keyPressEventInternal(QKeyEvent* event);

    RLANN_DS_QImageFrameBufferPtr getPaperTraceFBuf() { return _paperTraceFbuf; }

    // Called when a frame has exited the lookahead
    // attempt to offload any annotations to disk
    //
    void scheduleReleaseTask(qlonglong frameNum);
    void scheduleReloadAnnotationTask(qlonglong frameNum, bool runImmediate = false);

    StringPointMap* userPositionMap() { return _remoteUserMap; }

signals:
    
    void syncDrawStart(qlonglong drawFrame, QString trackUuidStr);
    void syncDrawEnd(qlonglong drawFrame, QString trackUuidStr);
    void syncHoldFrame(QString trackUuidStr, QString annUuidStr, qlonglong srcFrame, qlonglong destFrame);
    void syncReleaseFrame(QString trackUuidStr, QString annUuidStr, qlonglong frame);
    void activeTrackChanged(QString uuid);
    void brushInfo(QVariantMap info);

public:
    void emitSyncDrawStart(qlonglong currFrame, QString trackUuidStr)
    {
        emit syncDrawStart(currFrame, trackUuidStr);
    }

    void emitSyncDrawEnd(qlonglong drawFrame, QString trackUuidStr)
    {
        emit syncDrawEnd(drawFrame, trackUuidStr);
    }

    void emitBrushInfo(QVariantMap info)
    {
        emit brushInfo(info);
    }

    void stopUpdateTimer()
    {
        _updateTimer.stop();
    }


public slots:

    qlonglong currFrame() { return _currFrame; }

    void setupFrame(qlonglong frameNum, QString trackUuidStr = "", QString annUuidStr = "");
    void updateAnnDisplayWindow(RLANN_DS_AnnotationPtr ann);
    RLANN_DS_Annotation* initAnnotation(qlonglong frameNum, QString trackUuidStr, int width, int height);

    void setPaperTraceMode(int mode)
    {
        _paperTraceMode = (PaperTraceMode)mode;
    }

    void setPaperTraceActive(bool isActive)
    {
        _doPaperTrace = isActive;
    }

    bool isPaperTraceActive() { return _doPaperTrace; }

    // Track
    //
    void setActiveTrack(QString uuidStr);
    
    DS_TrackPtr activeTrack();

    // Annotation / Frame
    //
    void holdFrame(QString viewUuidStr, QString trackUuidStr, qlonglong srcFrame, qlonglong destFrame, bool emitSyncAction=true);
    void releaseFrame(QString viewUuidStr, QString trackUuidStr, qlonglong srcFrame, bool emitSyncAction=true);

    // Brush
    //
    // void setBrushColor(QColor col);
    // void setBrushSize(int size);
    void setBrushSettings(QVariantMap settings);
    bool setBrush(QString brushName);
    void setAntialiasing(bool doAntialiasing) {_doAntialiasing = doAntialiasing; }

    void setSyncAnnotations(bool doSync);
    bool syncAnnotations() { return _doSyncAnnotations; }
    void registerAnnotationForSync(QString trackUuidStr, QString annUuidStr);

    RLANN_MP_Handler* mypaintHandler();

    // bool getAntialiasing() { return _doAntialiasing; }
    QString brushEngine() { return _brushEngine; }
    void setBrushEngine(QString brushEngine) { _brushEngine = brushEngine; }

public slots:

    void onSessionChanged(DS_SessionPtr session);
    void onTrackDataChanged(QString eventName, QString trackUuidStr, QVariantMap trackData);
    void onTrackActivated(QString trackUuidStr);
    void onTimerMediaUpdate();
    void onPlayStateChanged(int playState);
    void onLookaheadRelease(qlonglong frame);
    void onDisplayDeregistered(DISP_GLView*);
    void onAnnotationImageChanged(RLANN_DS_Annotation* ann);

    // Sync
    //
    void handleSyncAction(QString action, QVariantMap kwargs);


private:
    void initPreferences();

    QThreadPool* _tpool;

    RLANN_CNTRL_FrameProbe* _frameProbe;
    RLANN_CNTRL_TrackFactoryPlugin* _trackFactory;

    QString _activeTrackUuidStr;

    bool _drawing;

    qlonglong _currFrame;

    bool _doAntialiasing;

    PaperTraceMode _paperTraceMode;
    bool _doPaperTrace;
    RLANN_DS_QImageFrameBufferPtr _paperTraceFbuf;

    // keep track of a list of track UUIDs to post sync updates via timer
    // (to keep a constant update frequency)
    QTimer _updateTimer;
    QSet<QPair<QString, QString>> _updateSet; 

    bool _doSyncAnnotations;

    static RLANN_CNTRL_DrawController* _instance;

    // just a value for now to help control what brush gets
    // set when picking pen or eraaser in UI
    QString _brushEngine;

    StringPointMap* _remoteUserMap;
};


Q_DECLARE_METATYPE(RLANN_CNTRL_DrawController *)


class EXTREVLENS_RLANN_CNTRL_API RLANN_CNTRL_DisplayPlugin : public DS_EventPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_CNTRL_DisplayPlugin(RLANN_CNTRL_DrawController* annCntrl);
    ~RLANN_CNTRL_DisplayPlugin();

    bool setDisplay(DISP_GLView* display);

    void updateAnnDisplayWindow(RLANN_DS_AnnotationPtr ann);

    bool hoverMoveEventInternal(QHoverEvent* event);
    bool mousePressEventInternal(QMouseEvent* event);
    bool mouseMoveEventInternal(QMouseEvent* event);
    bool mouseReleaseEventInternal(QMouseEvent* event);

    void strokeTo(const QPointF &endPoint, qlonglong frame, RLANN_DS_Track* track);
    
    void resizeGLInternal(int width, int height);
    void paintGLPostDraw(GUI_Painter* painter, const QVariantMap& mdata);

public slots:

    // Display
    void onDisplayZoomChanged(float displayZoom);
    void onDisplayHoverEnter();

protected:
    QString _username;

    RLANN_CNTRL_DrawController* _annCntrl;
    bool _drawing;

    int _windowWidth;
    int _windowHeight;
    float _displayZoom;
};


#endif