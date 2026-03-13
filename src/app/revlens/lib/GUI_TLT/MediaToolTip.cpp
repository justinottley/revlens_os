
#include "RlpRevlens/GUI_TLT/MediaToolTip.h"

#include "RlpGui/BASE/Style.h"
#include "RlpCore/UTIL/Text.h"

RLP_SETUP_LOGGER(revlens, GUI_TLT, MediaToolTip)

GUI_TLT_MediaToolTip::GUI_TLT_MediaToolTip(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _textTop(""),
    _textBottom("")
{
    setWidth(200);
    setHeight(60);
}


void
GUI_TLT_MediaToolTip::setTextTop(QString text)
{
    _textTop = text;

    updateWidth();
}


void
GUI_TLT_MediaToolTip::setTextBottom(QString text)
{
    _textBottom = text;

    updateWidth();
}


void
GUI_TLT_MediaToolTip::updateWidth()
{
    int thumbWidth = _thumbnail.width();

    int topTextWidth = UTIL_Text::getWidth(_textTop) + thumbWidth + 20;
    int botTextWidth = UTIL_Text::getWidth(_textBottom) + thumbWidth + 20;

    int newWidth = std::max(topTextWidth, botTextWidth);

    setWidth(newWidth);
}


void
GUI_TLT_MediaToolTip::paint(GUI_Painter* painter)
{
    QPen p(GUI_Style::Highlight);
    p.setWidth(3);

    painter->setPen(p);
    painter->setBrush(QBrush(QColor(10, 10, 10)));
    painter->drawRoundedRect(boundingRect(), 8, 8);

    if (_thumbnail.width() > 0)
    {
        painter->drawImage(10, 10, &_thumbnail);

    }

    QFont f = painter->font();
    f.setPixelSize(12);
    painter->setFont(f);

    painter->setPen(Qt::white);

    if (_textBottom != "")
    {
        painter->drawText(80, 45, _textBottom);
    }

    if (_textTop != "")
    {
        f.setBold(true);
        painter->setFont(f);
        painter->drawText(80, 25, _textTop);
    }

}