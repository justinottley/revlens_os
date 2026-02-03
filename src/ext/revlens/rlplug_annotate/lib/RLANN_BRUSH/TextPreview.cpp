

#include "RlpExtrevlens/RLANN_BRUSH/TextPreview.h"

#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(ext, RlpAnnBrush, TextPreview)

RlpAnnBrush_TextPreview::RlpAnnBrush_TextPreview(GUI_ItemBase* parent):
    GUI_ItemBase(parent),
    _penWidth(10),
    _hidden(false),
    _autoHide(false)
{
    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemIsFocusScope);

    RLP_LOG_DEBUG("")
}


void
RlpAnnBrush_TextPreview::setPenSettings(QVariantMap settings)
{
    RLP_LOG_DEBUG(settings)

    _penCol = QColor(
        settings.value("col").toMap().value("b").toInt(),
        settings.value("col").toMap().value("g").toInt(),
        settings.value("col").toMap().value("r").toInt()
    );

    _penWidth = settings.value("width").toInt();
}


void
RlpAnnBrush_TextPreview::hoverEnterEvent(QHoverEvent* event)
{
    RLP_LOG_DEBUG("hover enter")

    _inHover = true;
    setFocus(true);

    emit focusChanged(this, true);
}


void
RlpAnnBrush_TextPreview::hoverLeaveEvent(QHoverEvent* event)
{
    RLP_LOG_DEBUG("hover leave")

    _inHover = false;
    setFocus(false);

    emit focusChanged(this, false);
}


void
RlpAnnBrush_TextPreview::keyPressEvent(QKeyEvent* event)
{
    if (!_inHover)
        return;

    event->setAccepted(true);
    Qt::Key key = (Qt::Key)event->key();
    
    if (key == Qt::Key_Backspace)
    {
        if (_text.length() > 0)
        {
            _text = _text.left(_text.length() - 1);
        }
    } else if (key == Qt::Key_Return)
    {
        QVariantMap md;
        md.insert("text.value", _text);
        md.insert("text.size", textSize());
        md.insert("image.pos", _posInImage);
        md.insert("preview.pos", pos());

        // _text = "";
        setVisible(false);
        emit enterPressed(md);

    } else if (key == Qt::Key_Escape)
    {
        _text = "";
        setVisible(false);
        emit escapePressed();

    } else
    {
        _text += event->text();
    }

    update();

}


void
RlpAnnBrush_TextPreview::keyReleaseEvent(QKeyEvent* event)
{
}


float
RlpAnnBrush_TextPreview::textSize()
{
    return int(rint(_penWidth * _imgRatio * 2));
}


void
RlpAnnBrush_TextPreview::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    painter->setOpacity(0.75);

    QPen p(Qt::white);
    p.setWidth(3);
    painter->setPen(p);
    painter->setBrush(QBrush(Qt::black));

    QRectF r(5, 5, width() - 10, height() - 10);
    painter->drawRoundedRect(r, 5, 5);
    // painter->drawRoundedRect(boundingRect(), 5, 5);

    QPen pe(_penCol);
    QFont f = painter->font();

    float pixelSize = textSize();

    f.setPixelSize(pixelSize);
    painter->setFont(f);
    painter->setPen(pe);
    painter->drawText(10, f.pixelSize() + 1, _text);
}

