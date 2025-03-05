
#ifndef EXTREVLENS_RLANN_GUI_STARTUP_PLUGIN_H
#define EXTREVLENS_RLANN_GUI_STARTUP_PLUGIN_H

#include "RlpExtrevlens/RLANN_GUI/Global.h"

#include "RlpRevlens/DS/Plugin.h"

class EXTREVLENS_RLANN_GUI_API RLANN_GUI_StartupPlugin : public DS_Plugin {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLANN_GUI_StartupPlugin();

public slots:
    
};

#endif
