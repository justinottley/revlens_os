
#include "RlpExtrevlens/RLQTGUI/StartupPlugin.h"
#include "RlpExtrevlens/RLQTGUI/OnTheGoTrackFactoryPlugin.h"

#include "RlpRevlens/GUI_TL2/Controller.h"

RLP_SETUP_LOGGER(ext, RLQTGUI, StartupPlugin)

RLQTGUI_StartupPlugin::RLQTGUI_StartupPlugin():
    DS_Plugin("Builtin GUI Startup")
{
    GUI_TL2_Controller::instance()->registerTrackFactoryPlugin(
        "OnTheGo",
        RLQTGUI_OnTheGoTrackFactoryPlugin::create
    );
}