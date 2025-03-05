//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#include "RlpRevlens/DS/Plugin.h"

RLP_SETUP_LOGGER(revlens, DS, Plugin)


bool
DS_Plugin::setMainController(CNTRL_MainController* controller)
{
    // RLP_LOG_DEBUG("")

    _controller = controller;
    return true;
}


bool
DS_Plugin::setDisplay(DISP_GLView* display)
{
    // RLP_LOG_DEBUG("")

    _display = display;
    return true;
}


void
DS_Plugin::setEnabled(bool enabled)
{
    RLP_LOG_DEBUG(enabled)

    _enabled = enabled;
}