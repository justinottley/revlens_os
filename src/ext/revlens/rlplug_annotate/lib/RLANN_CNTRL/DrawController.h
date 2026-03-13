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

signals:

    void syncDrawStart(qlonglong drawFrame, QString trackUuidStr);
    void syncDrawEnd(qlonglong drawFrame, QString trackUuidStr);
    void syncHoldFrame(QString trackUuidStr, QString annUuidStr, qlonglong srcFrame, qlonglong destFrame);
    void syncReleaseFrame(QString trackUuidStr, QString annUuidStr, qlonglong frame);
    void activeTrackChanged(QString uuid);


public:
    RLP_DEFINE_LOGGER

    RLANN_CNTRL_DrawController(bool setSingleton=true);

    static RLANN_CNTRL_DrawController* instance() { return _instance; }
    static void setInstance(RLANN_CNTRL_DrawController* instance) { _instance = instance; }

    virtual DS_EventPlugin* duplicate();

    bool setMainController(DS_ControllerBase* controller);

    void updateDisplay() { _controller->getVideoManager()->update(); }
    bool event(QEvent* event);

    bool keyPressEventInternal(QKeyEvent* event);

    RLANN_DS_QImageFrameBufferPtr getPaperTraceFBuf() { return _paperTraceFbuf; }

    // Called when a frame has exited the lookahead
    // attempt to offload any annotations to disk
    //
    void scheduleReleaseTask(qlonglong frameNum);
    void scheduleReloadAnnotationTask(qlonglong frameNum, bool runImmediate = false);

    StringMousePosMap* userPositionMap() { return _remoteUserPositionMap; }


public:
    void emitSyncDrawStart(qlonglong currFrame, QString trackUuidStr)
    {
        emit syncDrawStart(currFrame, trackUuidStr);
    }

    void emitSyncDrawEnd(qlonglong drawFrame, QString trackUuidStr)
    {
        emit syncDrawEnd(drawFrame, trackUuidStr);
    }

    void stopUpdateTimer()
    {
        _updateTimer.stop();
    }


public slots:

    qlonglong currFrame() { return _currFrame; }
    void setCurrFrame(qlonglong frame) { _currFrame = frame; }

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

    float paperTraceOpacity() { return _paperTraceOpacity; }
    void setPaperTraceOpacity(float val) { _paperTraceOpacity = val; }


    // Track
    //
    void setActiveTrack(QString uuidStr);

    DS_TrackPtr activeTrack();
    RLANN_DS_Track* getAnnotationTrackByUuid(QString uuidStr);

    // Annotation / Frame
    //
    void holdFrame(QString viewUuidStr, QString trackUuidStr, qlonglong srcFrame, qlonglong destFrame, bool emitSyncAction=true);
    void releaseFrame(QString viewUuidStr, QString trackUuidStr, qlonglong srcFrame, bool emitSyncAction=true);

    // Brush
    //
    // void setBrushColor(QColor col);
    // void setBrushSize(int size);
    void setBrushSettings(QVariantMap settings, QString trackUuidStr="", bool emitSyncAction=true);
    bool setBrush(QString brushName, QString trackUuidStr="");
    void setAntialiasing(bool doAntialiasing) {_doAntialiasing = doAntialiasing; }

    void setSyncAnnotations(bool doSync);
    bool syncAnnotations() { return _doSyncAnnotations; }
    void registerAnnotationForSync(QString trackUuidStr, QString annUuidStr);

    RLANN_MP_Handler* mypaintHandler();

    // bool getAntialiasing() { return _doAntialiasing; }
    QString brushEngine() { return _brushEngine; }
    void setBrushEngine(QString brushEngine) { _brushEngine = brushEngine; }

    QImage extractAnnotation(QImage bg, QImage rawAnnotation);
    QImage flattenAnnotation(QImage bg, QImage rawAnnotation);

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
    float _paperTraceOpacity;
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

    StringMousePosMap* _remoteUserPositionMap;
};


Q_DECLARE_METATYPE(RLANN_CNTRL_DrawController *)


#endif