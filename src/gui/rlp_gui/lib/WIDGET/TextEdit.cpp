

#include "RlpGui/WIDGET/TextEdit.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpCore/UTIL/Text.h"

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>

RLP_SETUP_LOGGER(gui, GUI, TextArea)

GUI_TextArea::GUI_TextArea(GUI_ScrollArea* sarea, qreal width, qreal height):
    GUI_ItemBase(sarea->content()),
    _sarea(sarea),
    _minWidth(width),
    // _height(height),
    _text(""),
    _textWidth(0),
    _textHeight(20),
    _textColour(Qt::white),
    _hidden(false),
    _readOnly(false),
    _inHover(false),
    _wordWrap(false)
{
    sarea->setAcceptHoverEvents(true);
    sarea->setWidthOffset(30);
    setFlag(QQuickItem::ItemIsFocusScope);

    connect(
        sarea,
        &GUI_ScrollArea::focusChanged,
        this,
        &GUI_TextArea::onFocusChanged
    );

    setWidth(width);
    setHeight(height);
}


void
GUI_TextArea::setText(QString text)
{
    _text = text;
    if (_hidden)
    {
        QString ht;
        for (int cx = 0; cx != text.length(); cx++)
        {
            ht += "*";
        }
        _textWidth = UTIL_Text::getWidth(ht);
    } else
    {
        _textWidth = UTIL_Text::getWidth(_text);
    }
    

    if (_wordWrap)
    {
        _textList = UTIL_Text::wrapToWidth(_text, width() - 5);
        _textWidth = width() - 5;
        setHeight(_textList.size() * 20);
        _sarea->setHeight(height());
    }

    _sarea->setMaxChildWidth(_textWidth + 20);

    // RLP_LOG_DEBUG("width:" << width() << "new width:" << _textWidth)

    if (_textWidth + 20 > width())
    {
        setWidth(_textWidth + 20);

    } else if ( _textWidth < _minWidth)
    {
        setWidth(_minWidth);
    }

    emit textChanged(_text);
}


void
GUI_TextArea::keyPressEvent(QKeyEvent* event)
{
    // RLP_LOG_DEBUG(event)

    if (_inHover) {
        
        event->setAccepted(true);

        Qt::Key key = (Qt::Key)event->key();
    
        if (key == Qt::Key_Escape) {
            emit escapePressed();

        } else if (key == Qt::Key_Backspace) {
            _text.remove(_text.length() - 1, 1);

        } else if (key == Qt::Key_Return) {

            QVariantMap info;
            QVariant iw;
            iw.setValue(this);

            info.insert("item", iw);
            emit enterPressed(info);
            return;

        } else if ((key == Qt::Key_V) && (event->modifiers().testFlag(Qt::ControlModifier))) {

            const QClipboard *clipboard = QApplication::clipboard();
            const QMimeData *mimeData = clipboard->mimeData();
            if (mimeData->hasText()) {
                _text += mimeData->text();
            }

        } else { 
            _text += event->text();

            // RLP_LOG_DEBUG(_text)

        }

        setText(_text);

        _sarea->onHHandlePosChanged(1.0); // autoscroll

    } else {

        event->setAccepted(false);
    }
    
    update();
}


void
GUI_TextArea::keyReleaseEvent(QKeyEvent* event)
{
    // RLP_LOG_DEBUG("")

    if (!_inHover) {
        event->setAccepted(false);
    }
    
}


void
GUI_TextArea::onFocusChanged(Qt::FocusReason reason, bool isFocused)
{
    // RLP_LOG_DEBUG(reason << isFocused)

    if (_readOnly)
        return;


    _inHover = isFocused;

    if (_inHover) {
        setFocus(true);
    } else {
        setFocus(false);
    }

    emit focusChanged(reason, isFocused);

    update();
}


QRectF
GUI_TextArea::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_TextArea::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    // painter->setBrush(QBrush(Qt::red));
    // painter->drawRect(boundingRect());
    // painter->reset();
    painter->setPen(_textColour);

    QString textToDraw;

    if ((_text.size() == 0) && (_tempHintText.size() > 0) && (!_inHover))
    {
        textToDraw = _tempHintText;

    }
    else if (!_hidden)
    {
        textToDraw = _text;
    }
    else
    {
        // QString hiddenText;
        for (int cx = 0; cx != _text.length(); cx++)
        {
            textToDraw += "*";
        }
    }
    

    if (_wordWrap)
    {
        int posY = 15;
        for (int ti=0; ti != _textList.size(); ++ti)
        {
            painter->drawText(5, posY, _textList.at(ti));
            posY += _textHeight;
        }
    } else {
        painter->drawText(5, 15, textToDraw);
    }
    

    if (_inHover) {
        painter->drawText(5 + _textWidth, 15, "|");
    }
}


//
// --------
//

RLP_SETUP_LOGGER(gui, GUI, TextEdit)

GUI_TextEdit::GUI_TextEdit(QQuickItem* parent, qreal width, qreal height):
    GUI_ItemBase(parent)
{
    _scrollArea = new GUI_ScrollArea(this, width, height);
    // _scrollArea->setAutoHide(false);

    // _scrollArea->setMaxChildHeight(100);
    
    // RLP_LOG_DEBUG("w: " << width << " cw: " << _scrollArea->contentWidth())
    _textArea = new GUI_TextArea(_scrollArea, width, height);

    connect(
        _textArea,
        &GUI_TextArea::focusChanged,
        this,
        &GUI_TextEdit::onTextAreaFocusChanged
    );

    connect(
        _textArea,
        &GUI_TextArea::enterPressed,
        this,
        &GUI_TextEdit::onTextAreaEnterPressed
    );

    connect(
        _textArea,
        &GUI_TextArea::escapePressed,
        this,
        &GUI_TextEdit::onTextAreaEscapePressed
    );

    connect(
        _textArea,
        &GUI_TextArea::textChanged,
        this,
        &GUI_TextEdit::textChanged
    );

    setWidth(width);
    setHeight(height);
}


void
GUI_TextEdit::setFocused(bool isFocused)
{
    RLP_LOG_DEBUG(isFocused)

    _textArea->onFocusChanged(Qt::TabFocusReason, isFocused);
    _scrollArea->setFocused(isFocused);

    qobject_cast<GUI_Scene*>(scene())->requestUpdate();
}


void
GUI_TextEdit::onTextAreaFocusChanged(Qt::FocusReason reason, bool isFocused)
{
    // RLP_LOG_DEBUG(reason << isFocused)

    QVariant iw;
    iw.setValue(this);

    QVariantMap info;
    info.insert("widget", iw);
    info.insert("reason", (int)reason);
    info.insert("isFocused", isFocused);

    if (isFocused)
    {
        emit focusChanged(info);
    } else {

        _lastFocusInfo = info;
        QTimer::singleShot(300, this, &GUI_TextEdit::emitFocusChanged); // "emitFocusChanged(QVariantMap)");
    }
    
    // emit focusChanged(this, reason, isFocused);

}


void
GUI_TextEdit::onTextAreaEnterPressed(QVariantMap itemInfo)
{
    RLP_LOG_DEBUG("")

    emit enterPressed(itemInfo);
}


void
GUI_TextEdit::onTextAreaEscapePressed()
{
    RLP_LOG_DEBUG("")

    emit escapePressed();
}


void
GUI_TextEdit::clear()
{
    RLP_LOG_DEBUG("")

    _scrollArea->resetPosition();
    _textArea->setText("");

    _textArea->update();
}


QRectF
GUI_TextEdit::boundingRect() const
{
    return _scrollArea->boundingRect();
}