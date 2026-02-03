//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_TOGGLE_ICON_BUTTON_H
#define GUI_TOGGLE_ICON_BUTTON_H

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/WIDGET/IconButton.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/SvgIcon.h"

#include "RlpGui/MENU/MenuPane.h"


class GUI_WIDGET_API GUI_ToggleIconButton : public GUI_IconButton {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_ToggleIconButton(
        GUI_ItemBase* parent,
        QString filenameOn,
        QString filenameOff,
        int size=DEFAULT_ICON_SIZE,
        int padding=4);

    void mousePressEvent(QMouseEvent* event);

    void paint(GUI_Painter* painter);
    
    GUI_SvgIcon* offSvgIcon() { return _offSvgIcon; }

public slots:

    static GUI_ToggleIconButton*
    new_GUI_ToggleIconButton(GUI_ItemBase* parent, QString fOn, QString fOff, int size)
    {
        return new GUI_ToggleIconButton(parent, fOn, fOff, size);
    }

    void setToggled(bool isToggled);

    void onIconMousePressEvent(QMouseEvent* event);

protected:

    QImage _offIcon;
    GUI_SvgIcon* _offSvgIcon;
};

#endif