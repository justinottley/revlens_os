

#include "RlpRevlens/GUI_TL2/FrameTimeInfo.h"

#include "RlpRevlens/GUI_TL2/View.h"

RLP_SETUP_LOGGER(revlens, GUI_TL2, FrameTimeInfo)

GUI_TL2_FrameTimeInfo::GUI_TL2_FrameTimeInfo(GUI_TL2_View* view):
    _view(view)
{
}


QRectF
GUI_TL2_FrameTimeInfo::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TL2_FrameTimeInfo::paint(GUI_Painter* painter)
{
    
}