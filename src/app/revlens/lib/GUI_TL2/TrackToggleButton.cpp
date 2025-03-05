
#include "RlpRevlens/GUI_TL2/TrackToggleButton.h"

#include "RlpRevlens/GUI_TL2/Track.h"
#include "RlpRevlens/GUI_TL2/View.h"

#include "RlpRevlens/GUI_TL2/TrackButton.h"

GUI_TL2_TrackToggleButton::GUI_TL2_TrackToggleButton(GUI_ItemBase* parent):
    GUI_TL2_TrackButton::GUI_TL2_TrackButton(parent)
{
}


QColor
GUI_TL2_TrackToggleButton::colPen()
{
    QColor result;
    
    if (_inHover) {
        result = _colHoverOn;

    } else {
        if (_isToggled) {
            result = _colBgOn;

        } else {
            result = _colHoverOff;
        }
    }
    return result;
}


void
GUI_TL2_TrackToggleButton::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("");
    // qobject_cast<GUI_Scene*>(scene())->requestUpdate();
    update();
}


void
GUI_TL2_TrackToggleButton::mouseReleaseEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("TrackToggleButton");

    _isToggled = !_isToggled;
    emit toggleChanged(_isToggled);

    //qobject_cast<GUI_Scene*>(scene())->requestUpdate();
    update();
}


void
GUI_TL2_TrackToggleButton::paint(GUI_Painter* painter)
{
    
    // NOTE: setting clip region doesnt seem to be working for these buttons. There are painting
    // artifacts and the clipping is offset

    QPen p(colPen());
    p.setWidth(2);

    painter->setPen(p);
    painter->setBrush(QBrush(QColor(20, 20, 20)));
    painter->drawRect(boundingRect());

    //painter->setRenderHint(QPainter::Antialiasing, true);

    // GUI_TL2_TrackButton::paint(painter, option, widget);
    
    if (_isToggled) {
        paintOn(painter);
    } else {
        paintOff(painter);
    }
}