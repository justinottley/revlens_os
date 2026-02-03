
#include "RlpExtrevlens/RLANN_GUI/ColourButton.h"

#include "RlpGui/BASE/Style.h"


RLP_SETUP_EXT_LOGGER(RLANN_GUI, ColourButton)

RLANN_GUI_ColourButton::RLANN_GUI_ColourButton(QColor col, GUI_ItemBase* parent):
    GUI_IconButton("", parent),
    _col(col),
    _selected(false)
{
    connect(
        this, &RLANN_GUI_ColourButton::buttonPressed,
        this, &RLANN_GUI_ColourButton::onButtonPressed
    );
}


void
RLANN_GUI_ColourButton::onButtonPressed(QVariantMap /* metadata */)
{
    // RLP_LOG_DEBUG("")

    emit colourSelected(_col);
    _inHover = false;
}


void
RLANN_GUI_ColourButton::paint(GUI_Painter* painter)
{
    QPen p;

    painter->setPen(_col);
    // painter->save();

    if ((_inHover) && (!_selected))
    {
        p.setWidth(4);
        p.setColor(GUI_Style::BgDarkGrey);

    } else if (_selected)
    {
        p.setWidth(4);
        p.setColor(Qt::black);

    } else
    {
        p.setWidth(1);

    }

    painter->setPen(p);
    painter->setBrush(_col);

    painter->drawRect(boundingRect());

    // painter->restore();
}