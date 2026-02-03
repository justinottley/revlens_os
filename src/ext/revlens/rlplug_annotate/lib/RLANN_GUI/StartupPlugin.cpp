
#include "RlpExtrevlens/RLANN_GUI/StartupPlugin.h"
#include "RlpExtrevlens/RLANN_GUI/AnnotationTrackFactoryPlugin.h"

#include "RlpRevlens/GUI_TL2/Controller.h"

RLP_SETUP_LOGGER(ext, RLANN_GUI, StartupPlugin)

RLANN_GUI_StartupPlugin::RLANN_GUI_StartupPlugin():
    DS_Plugin("Annotation GUI Startup")
{
    GUI_TL2_Controller::instance()->registerTrackFactoryPlugin(
        "Annotation",
        RLANN_GUI_AnnotationTrackFactoryPlugin::create
    );
}