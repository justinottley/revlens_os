
#ifndef EXTREVLENS_RLQTGUI_STARTUP_PLUGIN_H
#define EXTREVLENS_RLQTGUI_STARTUP_PLUGIN_H

#include "RlpExtrevlens/RLQTGUI/Global.h"

#include "RlpRevlens/DS/Plugin.h"

class EXTREVLENS_RLQTGUI_API RLQTGUI_StartupPlugin : public DS_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_StartupPlugin();


};

#endif
