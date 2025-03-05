//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_DS_SYNCTARGET_H
#define REVLENS_DS_SYNCTARGET_H

#include "RlpRevlens/DS/Global.h"

class CNTRL_MainController;

class REVLENS_DS_API DS_SyncTarget {
    
public:
    
    DS_SyncTarget() {}
    
    void setMainController(CNTRL_MainController* controller) { _controller = controller; }
    // CNTRL_MainController* getController() { return _controller; }
    
    virtual void presentNextFrame(qlonglong frameNum) {}
    
    CNTRL_MainController* controller() { return _controller; }
    
protected:
    
    CNTRL_MainController* _controller;
    
};

#endif

