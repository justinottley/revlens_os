

#include "RlpGui/BASE/SpacerItem.h"


GUI_SpacerItem::GUI_SpacerItem(qreal width, qreal height, GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    setSize(QSizeF(width, height));
}


QRectF
GUI_SpacerItem::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}