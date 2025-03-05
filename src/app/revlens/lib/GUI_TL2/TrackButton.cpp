
#include "RlpRevlens/GUI_TL2/TrackButton.h"

#include "RlpRevlens/GUI_TL2/Track.h"


int GUI_TL2_TrackButton::SIZE = 18;

RLP_SETUP_LOGGER(revlens, GUI_TL2, TrackButton)

GUI_TL2_TrackButton::GUI_TL2_TrackButton(GUI_ItemBase* parent):
    GUI_ButtonBase(parent),
    _colHoverOff(QColor(20, 20, 20)),
    _colHoverOn(QColor(100, 100, 100)),
    _colBgOn(20, 20, 20)
{
    setWidth(size());
    setHeight(size());
    setAcceptHoverEvents(true);
}


QColor
GUI_TL2_TrackButton::colPen()
{
    if (_inHover) {
        return _colHoverOn;
    }

    return _colHoverOff;
}



QRectF
GUI_TL2_TrackButton::boundingRect() const
{
    return QRectF(0, 0, size(), size());
}
