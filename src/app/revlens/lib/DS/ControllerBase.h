//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_CONTROLLERBASE_H
#define REVLENS_DS_CONTROLLERBASE_H

#include "RlpRevlens/DS/Global.h"
#include "RlpRevlens/DS/Session.h"

#include "RlpCore/CNTRL/ControllerBase.h"

class REVLENS_DS_API DS_ControllerBase : public CoreCntrl_ControllerBase {
    Q_OBJECT

signals:
    void dataReady(QVariantMap info);
    void sessionChanged(DS_SessionPtr session);

public:

    DS_ControllerBase();
    virtual ~DS_ControllerBase() {}

    virtual DS_SessionPtr session() { return nullptr; }
    virtual DS_PlaybackMode* playbackMode() { return nullptr; }
    
    virtual bool event(QEvent* event);
    virtual void licenseCheck() {}
    virtual qlonglong currentFrameNum() { return 0; }


public slots:

    virtual void sendCall(QString method, QVariantList args, QVariantMap kwargs) {}
    virtual void emitSyncAction(QString action, QVariantMap kwargs) {}


};

typedef std::shared_ptr<DS_ControllerBase> DS_ControllerPtr;


Q_DECLARE_METATYPE(DS_ControllerPtr)

#endif
