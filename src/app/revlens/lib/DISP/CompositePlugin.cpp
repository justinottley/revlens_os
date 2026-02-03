
#include "RlpRevlens/DISP/CompositePlugin.h"
#include "RlpRevlens/DISP/GLView.h"

RLP_SETUP_LOGGER(revlens, DISP, CompositePlugin)

DISP_CompositePlugin::DISP_CompositePlugin(QString name):
    _name(name),
    _enabled(false)
{
}


void
DISP_CompositePlugin::setDisplay(DISP_GLView* display)
{
    RLP_LOG_DEBUG(display)

    _display = display;
}


void
DISP_CompositePlugin::setEnabled(bool isEnabled)
{
    RLP_LOG_DEBUG(isEnabled)

    _enabled = isEnabled;
}