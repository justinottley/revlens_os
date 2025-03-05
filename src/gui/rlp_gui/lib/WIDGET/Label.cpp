
#include "RlpGui/WIDGET/Label.h"
#include "RlpGui/BASE/Style.h"


RLP_SETUP_LOGGER(gui, GUI, Label)

GUI_Label::GUI_Label(QQuickItem* parent, QString text):
    GUI_ItemBase(parent),
    _text(text),
    _leftPad(0),
    _yOffset(0),
    _bgCol(Qt::transparent),
    _pen(Qt::white),
    _font(QApplication::font()),
    _leftSvgIcon(nullptr),
    _doDropShadow(false),
    _dsXOffset(2),
    _dsYOffset(2)
{
    setHeight(20);
    setText(text);
}


void
GUI_Label::setFont(QFont f)
{
    _font = f;
    setText(_text);
}


void
GUI_Label::setText(QString text)
{
    _text = text;
    QFontMetrics fm(_font);

    int nwidth = fm.horizontalAdvance(text) + 10;
    if (_leftSvgIcon != nullptr)
    {
        nwidth += _leftSvgIcon->width() + 5;
    }
    setWidth(nwidth);
    if (fm.height() > 20)
    {
        setHeight(fm.height());
    }
    // emit sizeChanged(width(), height());

    // RLP_LOG_DEBUG(text << width() << height())
}


void
GUI_Label::setBgColor(QColor col)
{
    _bgCol = col;
}


void
GUI_Label::setLeftImagePath(QString imagePath, int width, int height)
{
    // RLP_LOG_DEBUG(imagePath)

    _leftImage = QImage(imagePath);
    if ((width != 0) && (height == 0))
    {
        // RLP_LOG_DEBUG("scaling to width: " << width)

        _leftImage = _leftImage.scaledToWidth(width, Qt::SmoothTransformation);
    }

    setWidth(_leftImage.width() + 100);
    setHeight(_leftImage.height() + 20);

}


void
GUI_Label::setLeftSvgIconPath(QString path, int size, int padding)
{
    // RLP_LOG_DEBUG(path)

    _leftSvgIcon = new GUI_SvgIcon(path, this, size, padding);
}


QRectF
GUI_Label::boundingRect() const
{
    /*
    int bwidth = width();
    if (_leftSvgIcon != nullptr)
    {
        bwidth += _leftSvgIcon->width();
    }

    return QRectF(0, 0, bwidth + _leftPad, height());
    */
   return QRectF(0, 0, width(), height());
}


void
GUI_Label::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    // painter->setPen(Qt::transparent);
    // painter->setBrush(Qt::transparent);
    // painter->setBrush(QBrush(Qt::red));
    if (_bgCol != Qt::transparent)
    {
        painter->save();
        painter->setPen(_bgCol);
        painter->setBrush(QBrush(_bgCol));
        painter->drawRect(boundingRect());
        painter->restore();
    }

    int leftImageWidth = _leftImage.width();
    if (leftImageWidth > 0)
    {
        painter->drawImage(5 + _leftPad, 15, &_leftImage);
    }

    if (_leftSvgIcon != nullptr)
    {
        leftImageWidth = _leftSvgIcon->width();
    }

    // painter->setBrush(QBrush(Qt::red));
    // painter->drawRect(boundingRect());

    //painter->save();
    painter->setFont(_font);

    int xpos = 5 + leftImageWidth + _leftPad;
    int ypos = 15 + _yOffset;

    if (_doDropShadow)
    {
        QPen dsPen(QColor(25, 25, 25));
        painter->setPen(dsPen);
        painter->drawText(xpos + _dsXOffset, ypos + _dsYOffset, _text);
    }

    painter->setPen(_pen);
    painter->drawText(xpos, ypos, _text);

    //painter->restore();
}