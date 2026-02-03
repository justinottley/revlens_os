//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_SYNCPROBE_H
#define REVLENS_DS_SYNCPROBE_H

#include "RlpRevlens/DS/Global.h"

class DS_ControllerBase;

class REVLENS_DS_API DS_SyncProbe {

public:

    DS_SyncProbe() {}

    void setMainController(DS_ControllerBase* controller) { _controller = controller; }


    virtual void syncUpdate(qlonglong currFrame) {}
    virtual void syncUpdateImmediate(qlonglong currFrame) {}

    DS_ControllerBase* controller() { return _controller; }

protected:

    DS_ControllerBase* _controller;

};

#endif

