

#ifndef rlplug_QTBUILTIN_GUI_BLEND_COMPOSITE_SLIDER_H
#define rlplug_QTBUILTIN_GUI_BLEND_COMPOSITE_SLIDER_H

#include "RlpExtrevlens/RLQTGUI/Global.h"

#include "RlpGui/WIDGET/Slider.h"

#include "RlpRevlens/DISP/GLView.h"


class RLQTGUI_BlendCompositeSlider : public GUI_Slider {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    RLQTGUI_BlendCompositeSlider(DISP_GLView* display);



protected:

    DISP_GLView* _display;


};

#endif

