

#include "RlpGui/BASE/DividerItem.h"
#include "RlpGui/BASE/Style.h"

GUI_DividerItem::GUI_DividerItem(qreal width, qreal height, GUI_ItemBase* parent):
    GUI_ItemBase(parent)
{
    setSize(QSizeF(width, height));
}


void
GUI_DividerItem::paint(GUI_Painter* painter)
{
    qreal w = width();
    qreal h = height();

    QPen p(GUI_Style::BgMidGrey);
    p.setWidth(3);

    painter->setPen(p);
    painter->drawLine(w / 2, 3,  w / 2, h - 2);
}