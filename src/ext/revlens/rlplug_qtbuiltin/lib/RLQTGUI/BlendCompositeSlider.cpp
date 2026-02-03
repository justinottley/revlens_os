

#include "RlpExtrevlens/RLQTGUI/BlendCompositeSlider.h"


RLQTGUI_BlendCompositeSlider::RLQTGUI_BlendCompositeSlider(DISP_GLView* display):
    GUI_Slider(display, GUI_Slider::O_SL_HORIZONTAL),
    _display(display)
{
    setBgColour(GUI_Style::BgLoMidGrey);
    setOpacity(0.75);
    setPos(10, 50);
}