

#include "RlpGui/TEST/ItemBase.h"

GUI_QuickItemBase::GUI_QuickItemBase(QQuickItem* parent):
    QQuickPaintedItem(parent)
{
    qInfo() << "INIT!";
}


void
GUI_QuickItemBase::paint(GUI_Painter* painter)
{
    qInfo() << "PAINT";

    painter->drawText(10, 40, "Hello world again");
}