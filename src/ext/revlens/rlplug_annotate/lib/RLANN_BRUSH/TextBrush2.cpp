
#include "RlpExtrevlens/RLANN_BRUSH/TextBrush2.h"


#include "RlpRevlens/DISP/GLView.h"


RLP_SETUP_LOGGER(extrevlens, RlpAnnBrush, Text2)


RlpAnnBrush_Text2::RlpAnnBrush_Text2():
    RLANN_DS_BrushBase("Text")
{
}


RLANN_DS_BrushBase*
RlpAnnBrush_Text2::clone()
{
    return new RlpAnnBrush_Text2();
}


void
RlpAnnBrush_Text2::activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl)
{
    RLP_LOG_DEBUG("")
    RLANN_DS_BrushBase::activate(view, annCntrl);

    view->setCursor(Qt::IBeamCursor);
}
