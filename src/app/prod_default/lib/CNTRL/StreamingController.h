
#ifndef EXTREVLENS_CNTRL_StreamingController_H
#define EXTREVLENS_CNTRL_StreamingController_H

#include "RlpProd/CNTRL/Global.h"

#include "RlpRevlens/CNTRL/MainController.h"
#include "RlpRevlens/CNTRL/Video.h"
// #include "RlpRevlens/VIDEO/ThreadedClock.h"

#include "RlpRevlens/DS/ControllerBase.h"
#include "RlpRevlens/DS/Session.h"

#include "RlpExtrevlens/RLANN_DS/FrameBuffer.h"
#include "RlpExtrevlens/RLANN_CNTRL/DrawController.h"

class CNTRL_StreamingController;

class CNTRL_StreamingController : public DS_ControllerBase {
    Q_OBJECT

signals:
    void streamReady();

public:
    RLP_DEFINE_LOGGER

    CNTRL_StreamingController();

    static CNTRL_StreamingController* instance();

    DS_FrameBufferPtr drawFbuf() { return _drawFbuf; }

public slots:

    DS_SessionPtr session() { return _session; }

    CNTRL_Video* getVideoManager() { return _videoManager; }
    RLANN_CNTRL_DrawController* drawController() { return _drawController; }
    DS_EventPlugin* initAnnotationDisplayPlugin();

    void emitStreamReady() { emit streamReady(); }

    void loadStream(QString streamUrl, QVariantMap streamInfo);
    void startStream();
    void stopStream();

    QString streamUrl() { return _streamUrl; }
    QVariantMap streamInfo() { return _streamInfo; }

    void clearAnnotations();

    void onMediaNoticeEvent(QString evtName, QVariantMap evtInfo);

private:
    CNTRL_Video* _videoManager;
    DS_SessionPtr _session;

    RLANN_CNTRL_DrawController* _drawController;
    DS_FrameBufferPtr _drawFbuf;

    QString _streamUrl;
    QVariantMap _streamInfo;

    QVariantMap _fbufFrameInfo;

    static CNTRL_StreamingController* _instance;

};

#endif
