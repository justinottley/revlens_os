
#include "RlpGui/WIDGET/ToggleIconButton.h"
#include "RlpGui/MENU/MenuPane.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/Style.h"


RLP_SETUP_LOGGER(gui, GUI, ToggleIconButton)

GUI_ToggleIconButton::GUI_ToggleIconButton(
    GUI_ItemBase* parent,
    QString filenameOn,
    QString filenameOff,
    int size,
    int padding):
        GUI_IconButton(filenameOn, parent, size, padding),
        _offSvgIcon(nullptr)
{

    if (filenameOn.endsWith(".svg"))
    {
        _svgIcon->setForwardMouseEvent(true);
        connect(
            _svgIcon,
            &GUI_SvgIcon::forwardMouseEvent,
            this,
            &GUI_ToggleIconButton::onIconMousePressEvent
        );

    }
    if (filenameOff.endsWith(".svg"))
    {

        _offSvgIcon = new GUI_SvgIcon(filenameOff, this, size, padding);
        _offSvgIcon->setForwardMouseEvent(true);
        connect(
            _offSvgIcon,
            &GUI_SvgIcon::forwardMouseEvent,
            this,
            &GUI_ToggleIconButton::onIconMousePressEvent
        );

        _offSvgIcon->setVisible(false);
    } else
    {
        _offIcon = QImage(filenameOff).scaledToHeight(size, Qt::SmoothTransformation);
    }
}


void
GUI_ToggleIconButton::setToggled(bool isToggled)
{
    // RLP_LOG_DEBUG(isToggled)

    _isToggled = isToggled;

    if (_offSvgIcon != nullptr)
    {
        if (_isToggled)
        {
            if (_svgIcon != nullptr)
            {
                _svgIcon->setVisible(false);
            }
            _offSvgIcon->setVisible(true);
        } else
        {
            _offSvgIcon->setVisible(false);
            if (_svgIcon != nullptr)
            {
                _svgIcon->setVisible(true);
            }
            
        }
    }
}


void
GUI_ToggleIconButton::mousePressEvent(QMouseEvent* event)
{
    setToggled(!_isToggled);

    GUI_IconButton::mousePressEvent(event);
    update();
}


void
GUI_ToggleIconButton::onIconMousePressEvent(QMouseEvent* event)
{
    mousePressEvent(event);
}


void
GUI_ToggleIconButton::paint(GUI_Painter* painter)
{

    // RLP_LOG_DEBUG(_inHover)

    if (_inHover)
    {
        painter->setBrush(GUI_Style::BgLightGrey);
        painter->setPen(GUI_Style::BgLightGrey);
        painter->drawRect(boundingRect());
    }

    if ((_isToggled) && (_offIcon.width() > 0))
    {
        painter->drawImage(_padding, _padding, &_offIcon);

    } else
    {
        painter->drawImage(_padding, _padding, &_icon);
    }

}