
#include "RlpRevlens/GUI_TL2/TrackFactoryPluginBase.h"

#include "RlpRevlens/GUI_TL2/View.h"
#include "RlpRevlens/GUI_TL2/TrackManager.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, TrackFactoryPluginBase)

GUI_TL2_TrackFactoryPluginBase::GUI_TL2_TrackFactoryPluginBase(
    QString name,
    GUI_TL2_TrackManager* tmgr):
    _name(name),
    _trackManager(tmgr)
{
    startup();
}


GUI_TL2_TrackFactoryPluginBase::~GUI_TL2_TrackFactoryPluginBase()
{
    RLP_LOG_DEBUG("")
}


void
GUI_TL2_TrackFactoryPluginBase::startup()
{
    RLP_LOG_DEBUG("")
}


CNTRL_MainController*
GUI_TL2_TrackFactoryPluginBase::controller()
{
    return CNTRL_MainController::instance();
    // return _trackManager->view()->controller();
}

