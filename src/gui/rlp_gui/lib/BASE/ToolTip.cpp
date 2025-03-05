
#include "RlpGui/BASE/ToolTip.h"

GUI_ToolTip::GUI_ToolTip(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _font(QApplication::font())
{
    setHeight(30);
    setZ(100);
}


void
GUI_ToolTip::setText(QString text)
{
    _text = text;
    QFontMetrics fm(_font);

    setWidth(fm.horizontalAdvance(text) + 30);

    // if (fm.height() > 20)
    // {
        // setHeight(fm.height());
    // }
    
}


void
GUI_ToolTip::paint(GUI_Painter* painter)
{
    painter->setBrush(Qt::black);
    painter->drawRect(QRectF(0, 0, width() - 2, height() - 2));

    painter->setPen(Qt::white);
    painter->drawText(10, 18, _text);
}