
#include "RlpRevlens/GUI_TL2/TrackToggleButtonSvg.h"

#include "RlpCore/UTIL/AppGlobals.h"
#include "RlpRevlens/GUI_TL2/Track.h"



GUI_TL2_TrackToggleButtonSvg::GUI_TL2_TrackToggleButtonSvg(GUI_ItemBase* parent, QString imagePath):
    GUI_TL2_TrackToggleButton(parent),
    _imagePath(imagePath)
{
    _colBgOn = QColor(120, 120, 120); // track->colBg();

    _svgIcon = new GUI_SvgIcon(imagePath, this, 20);
}


void
GUI_TL2_TrackToggleButtonSvg::paintOn(GUI_Painter* painter)
{
    _svgIcon->setBgColour(_colBgOn);
}


void
GUI_TL2_TrackToggleButtonSvg::paintOff(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("TrackToggleButtonImage");
    _svgIcon->setBgColour(QColor(20, 20, 20));
}