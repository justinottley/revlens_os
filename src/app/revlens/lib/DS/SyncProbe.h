//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_SYNCPROBE_H
#define REVLENS_DS_SYNCPROBE_H

#include "RlpRevlens/DS/Global.h"

class CNTRL_MainController;

class REVLENS_DS_API DS_SyncProbe {
    
public:
    
    DS_SyncProbe() {}
    
    void setMainController(CNTRL_MainController* controller) { _controller = controller; }
    // CNTRL_MainController* getController() { return _controller; }
    
    virtual void syncUpdate(qlonglong currFrame) {}
    virtual void syncUpdateImmediate(qlonglong currFrame) {}
    
    CNTRL_MainController* controller() { return _controller; }
    
protected:
    
    CNTRL_MainController* _controller;
    
};

#endif

