
#include "RlpGui/BASE/LayoutBase.h"

RLP_SETUP_LOGGER(gui, GUI, LayoutBase)

GUI_LayoutBase::GUI_LayoutBase(QQuickItem* parent):
    GUI_ItemBase(parent)
{
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    _offsetMap.clear();
}


GUI_LayoutBase::~GUI_LayoutBase()
{
    RLP_LOG_DEBUG("")

    clear();
}


QList<GUI_ItemBase*>
GUI_LayoutBase::items()
{
    QList<GUI_ItemBase*> list;
    return list;
}


void
GUI_LayoutBase::onParentSizeChanged(qreal width, qreal height)
{
    setWidth(width);
    setHeight(height);
}