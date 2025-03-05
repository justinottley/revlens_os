//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TOGGLE_ICON_BUTTON_H
#define CORE_GUI_TOGGLE_ICON_BUTTON_H

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/MENU/MenuPane.h"
#include "RlpGui/WIDGET/IconButton.h"

class GUI_WIDGET_API GUI_ToggleIconButton : public GUI_IconButton {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ToggleIconButton(
        QString filenameOn,
        QString filenameOff,
        GUI_ItemBase* parent,
        int size=DEFAULT_ICON_SIZE,
        int padding=4);

    void mousePressEvent(QMouseEvent* event);

    void paint(GUI_Painter* painter);


protected:

    QImage _offIcon;
    bool _toggled;    

};

#endif