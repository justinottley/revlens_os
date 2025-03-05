
#ifndef GUI_TOGGLE_SVGBUTTON_H
#define GUI_TOGGLE_SVGBUTTON_H

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/BASE/ButtonBase.h"
#include "RlpGui/PANE/SvgButton.h"
#include "RlpGui/MENU/MenuPane.h"

class GUI_WIDGET_API GUI_ToggleSvgButton : public GUI_ButtonBase {
    Q_OBJECT

signals:
    void onButtonToggled(QVariantMap md);

public:
    RLP_DEFINE_LOGGER

    GUI_ToggleSvgButton(
        QQuickItem* parent,
        QString svgOnPath,
        QString svgOffPath,
        int size=DEFAULT_ICON_SIZE,
        int padding=0
    );

public slots:

    static GUI_ToggleSvgButton*
    new_GUI_ToggleSvgButton(QQuickItem* parent, QString svgOnPath, QString svgOffPath, int size)
    {
        return new GUI_ToggleSvgButton(parent, svgOnPath, svgOffPath, size);
    }

    GUI_SvgButton* onButton() { return _btnOn; }
    GUI_SvgButton* offButton()  { return _btnOff; }

    GUI_MenuPane* menu() { return _menu; }

    void onToggleButtonPressed(QVariantMap md);

protected:

    void toggle();

    GUI_SvgButton* _btnOn;
    GUI_SvgButton* _btnOff;

    GUI_MenuPane* _menu;

};
#endif
