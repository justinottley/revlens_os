

#include "RlpGui/BASE/MenuPaneBase.h"

GUI_MenuPaneBase::GUI_MenuPaneBase(GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{

}


void
GUI_MenuPaneBase::onParentSizeChanged(qreal width, qreal height)
{
    // RLP_LOG_DEBUG(metaObject()->className() << "parent:" << parentItem()->metaObject()->className() << width << height)

    if (width != 0)
    {
        setWidth(width);
    }

    if (height != 0)
    {
        setHeight(height);
    }
}