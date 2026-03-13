
#ifndef EXTREVLENS_RLANN_BRUSH_TEXT2_H
#define EXTREVLENS_RLANN_BRUSH_TEXT2_H


#include "RlpExtrevlens/RLANN_BRUSH/Global.h"

#include "RlpExtrevlens/RLANN_DS/BrushBase.h"

class DISP_GLView;

class RlpAnnBrush_Text2 : public RLANN_DS_BrushBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RlpAnnBrush_Text2();
    RLANN_DS_BrushBase* clone();

    void activate(DISP_GLView* view, RLANN_DS_DrawControllerBase* annCntrl);
};


#endif
