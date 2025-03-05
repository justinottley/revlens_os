

#include "RlpGui/WIDGET/ToggleSvgButton.h"

RLP_SETUP_LOGGER(gui, GUI, ToggleSvgButton)

GUI_ToggleSvgButton::GUI_ToggleSvgButton(
    QQuickItem* parent,
    QString svgOnPath,
    QString svgOffPath,
    int size,
    int padding
):
    GUI_ButtonBase(parent)
{
    _menu = new GUI_MenuPane(this);

    _btnOn = new GUI_SvgButton(svgOnPath, this, size, padding);
    _btnOn->setVisible(false);
    _btnOff = new GUI_SvgButton(svgOffPath, this, size, padding);
    // _btnOff->setVisible(false);

    connect(
        _btnOn,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_ToggleSvgButton::onToggleButtonPressed
    );

    connect(
        _btnOff,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_ToggleSvgButton::onToggleButtonPressed
    );


    setWidth(std::max(_btnOn->width(), _btnOff->width()));
    setHeight(std::max(_btnOff->height(), _btnOff->height()));

}


void
GUI_ToggleSvgButton::onToggleButtonPressed(QVariantMap md)
{
    RLP_LOG_DEBUG(md)

    int button = md.value("button").toInt();
    if (button == 1)
    {
        toggle();

    }

    md.insert("toggled", _isToggled);
    emit onButtonToggled(md);
}


void
GUI_ToggleSvgButton::toggle()
{
    RLP_LOG_DEBUG("")

    _isToggled = !_isToggled;

    if (_isToggled)
    {
        _btnOff->setVisible(false);
        _btnOn->setVisible(true);
    } else
    {
        _btnOn->setVisible(false);
        _btnOff->setVisible(true);
    }
}