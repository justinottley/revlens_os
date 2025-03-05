

#include "RlpGui/WIDGET/RadioButton.h"


RLP_SETUP_LOGGER(gui, GUI, RadioButton)

GUI_RadioButton::GUI_RadioButton(
    GUI_ItemBase* parent,
    QString labelText,
    qreal size,
    qreal padding):
        GUI_ItemBase(parent),
        _size(size),
        _textWidth(1),
        _toggled(false)
{
    _offIcon = new GUI_SvgIcon(":feather/lightgrey/circle.svg", this, size, padding);
    connect(
        _offIcon,
        &GUI_SvgIcon::buttonPressed,
        this,
        &GUI_RadioButton::buttonPressed
    );

    _onIcon = new GUI_SvgIcon(":feather/lightgrey/check-circle.svg", this, size, padding);
    connect(
        _onIcon,
        &GUI_SvgIcon::buttonPressed,
        this,
        &GUI_RadioButton::buttonPressed
    );

    _onIcon->setVisible(false);

    setHeight(size);
    setLabelText(labelText);
}


void
GUI_RadioButton::setToggled(bool toggled)
{
    _toggled = toggled;

    // RLP_LOG_DEBUG(_toggled)

    if (_toggled)
    {
        _offIcon->setVisible(false);
        _offIcon->setHover(false);

        _onIcon->setVisible(true);
    }
    else
    {
        _offIcon->setVisible(true);

        _onIcon->setVisible(false);
        _onIcon->setHover(false);
    }

}


void
GUI_RadioButton::setLabelText(QString text)
{
    _labelText = text;

    QFont currFont = QApplication::font();
    QFontMetrics fm(currFont);

    _textWidth = fm.horizontalAdvance(text) + 10;

    setWidth(_size + _textWidth + 10);

}


void
GUI_RadioButton::mousePressEvent(QMouseEvent* event)
{
    // RLP_LOG_INFO("")

    event->setAccepted(true);

    emit buttonPressed(_metadata);
}


QRectF
GUI_RadioButton::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_RadioButton::paint(GUI_Painter* painter)
{
    painter->setPen(Qt::white);
    painter->drawText(
        _size + 5,
        15,
        _labelText
    );
}
