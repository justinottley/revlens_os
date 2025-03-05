//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef EXTREVLENS_RLQTCNTRL_PLUGIN_H
#define EXTREVLENS_RLQTCNTRL_PLUGIN_H

#include "RlpExtrevlens/RLQTCNTRL/Global.h"

#include "RlpCore/LOG/Logging.h"
#include "RlpGui/MENU/MenuPane.h"
#include "RlpRevlens/DS/EventPlugin.h"



class EXTREVLENS_RLQTCNTRL_API RLQTCNTRL_Plugin : public DS_EventPlugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTCNTRL_Plugin();

    DS_EventPlugin* duplicate();
    
    bool keyPressEventInternal(QKeyEvent* event);
    bool keyReleaseEventInternal(QKeyEvent* event);
    bool mousePressEventInternal(QMouseEvent* event);


};



Q_DECLARE_METATYPE(RLQTCNTRL_Plugin *)

#endif

