//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLANN_DS_DrawControllerBase_H
#define EXTREVLENS_RLANN_DS_DrawControllerBase_H

#include "RlpExtrevlens/RLANN_DS/Global.h"

#include "RlpRevlens/DS/EventPlugin.h"
#include "RlpRevlens/CNTRL/MainController.h"

#include "RlpExtrevlens/RLANN_DS/QImageFrameBuffer.h"
#include "RlpExtrevlens/RLANN_DS/Annotation.h"

class RLANN_DS_Track;

class EXTREVLENS_RLANN_DS_API RLANN_DS_DrawControllerBase : public DS_EventPlugin {
    Q_OBJECT

signals:
    void brushInfoChanged(QVariantMap info);

public:
    RLANN_DS_DrawControllerBase();
    
    virtual void scheduleReleaseTask(qlonglong frameNum) {}
    virtual void scheduleReloadAnnotationTask(qlonglong frameNum, bool runImmediate = false) {}

    // TODO FIXME: UGLY
    virtual RLANN_DS_QImageFrameBufferPtr getPaperTraceFBuf() { return nullptr; }
    virtual bool isPaperTraceActive() { return false; }
    virtual float paperTraceOpacity() { return 0.7; }

public slots:
    void emitBrushInfoChanged(QVariantMap info)
    {
        emit brushInfoChanged(info);
    }
};

#endif