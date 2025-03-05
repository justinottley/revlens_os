
#include "RlpGui/WIDGET/ToggleIconButton.h"
#include "RlpGui/MENU/MenuPane.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"


RLP_SETUP_LOGGER(gui, GUI, ToggleIconButton)

GUI_ToggleIconButton::GUI_ToggleIconButton(
    QString filenameOn,
    QString filenameOff,
    GUI_ItemBase* parent,
    int size,
    int padding):
        GUI_IconButton(filenameOn, parent, size, padding),
        _toggled(false)
{
    _offIcon = QImage(filenameOff).scaledToHeight(size, Qt::SmoothTransformation);
}

void
GUI_ToggleIconButton::mousePressEvent(QMouseEvent* event)
{
    RLP_LOG_INFO("")

    GUI_IconButton::mousePressEvent(event);
    _toggled = !_toggled;
}


void
GUI_ToggleIconButton::paint(GUI_Painter* painter)
{

    // RLP_LOG_DEBUG(_inHover)

    if (_inHover) {
        painter->setBrush(GUI_Style::BgLightGrey);
        painter->setPen(GUI_Style::BgLightGrey);
        painter->drawRect(boundingRect());
    }

    if (_toggled) {
        painter->drawImage(_padding, _padding, &_offIcon);

    } else {
        painter->drawImage(_padding, _padding, &_icon);

    }

}