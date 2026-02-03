
#include "RlpRevlens/GUI_TL2/ViewBase.h"


GUI_TL2_ViewBase::GUI_TL2_ViewBase(CNTRL_MainController* cntrl, GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    DS_SyncProbe(),
    _controller(cntrl)
{
}
