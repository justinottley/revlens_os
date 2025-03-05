//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_TEXTEDIT_H
#define CORE_GUI_TEXTEDIT_H

#include "RlpCore/LOG/Logging.h"

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/BASE/ItemBase.h"
#include "RlpGui/BASE/ScrollArea.h"


class GUI_WIDGET_API GUI_TextArea : public GUI_ItemBase {
    Q_OBJECT

signals:
    void focusChanged(Qt::FocusReason reason, bool isFocused);
    void textChanged(QString text);

public:
    RLP_DEFINE_LOGGER

    GUI_TextArea(GUI_ScrollArea* sarea, qreal width=200, qreal height=200);

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    // void mousePressEvent(QMouseEvent* event);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    QString text() { return _text; }
    void setText(QString text);

    void setTempHintText(QString text) { _tempHintText = text; }
    QString tempHintText() { return _tempHintText; }

    void setTextColour(QColor col) { _textColour = col; }
    void setTextHidden(bool isHidden) { _hidden = isHidden; } // password-style display

    void setWordWrap(bool doWordWrap) { _wordWrap = doWordWrap; }
    bool wordWrap() { return _wordWrap; }

    void setReadOnly(bool readOnly) { _readOnly = readOnly; }
    bool isReadOnly() { return _readOnly; }

    void onFocusChanged(Qt::FocusReason reason, bool isFocused);

private:
    GUI_ScrollArea* _sarea;

    QString _text;
    QString _tempHintText;
    QStringList _textList; // wrapped text

    int _textWidth;
    int _textHeight;

    QColor _textColour;

    bool _hidden; // whether to display text or show * for password style
    bool _readOnly;

    qreal _minWidth;

    bool _inHover;

    int _cursorPos;
    int _selStart;
    int _selEnd;

    bool _wordWrap;
    
};


class GUI_WIDGET_API GUI_TextEdit : public GUI_ItemBase {
    Q_OBJECT

signals:
    void textChanged(QString text);

public:
    RLP_DEFINE_LOGGER

    GUI_TextEdit(QQuickItem* parent, qreal width=200, qreal height=200);

    QRectF boundingRect() const;

public slots:

    static GUI_TextEdit*
    new_GUI_TextEdit(QQuickItem* parent, qreal width=200, qreal height=200)
    {
        return new GUI_TextEdit(parent, width, height);
    }

    QString text() { return _textArea->text(); }
    void setText(QString text) { _textArea->setText(text); }

    void setTextColour(QColor col) { _textArea->setTextColour(col); }

    void setTempHintText(QString text) { _textArea->setTempHintText(text); }
    QString tempHintText() { return _textArea->tempHintText(); }

    qreal itemHeight() { return _scrollArea->height(); }

    void setTextHidden(bool isHidden) { _textArea->setTextHidden(isHidden); }

    void clear();

    void setFocused(bool isFocused);

    void setWordWrap(bool doWordWrap) { _textArea->setWordWrap(doWordWrap); }

    void setOutlined(bool outlined) { _scrollArea->setOutlined(outlined); }
    void setHScrollBarVisible(bool isVisible) { _scrollArea->setHScrollBarVisible(isVisible); }

    void onTextAreaFocusChanged(Qt::FocusReason reason, bool isFocused);
    void onTextAreaEnterPressed(QVariantMap item);
    void onTextAreaEscapePressed();

    void emitFocusChanged() { emit focusChanged(_lastFocusInfo); }

    GUI_ScrollArea* scrollArea() { return _scrollArea; }
    GUI_TextArea* textArea() { return _textArea; }

    void resetPosition() { _scrollArea->resetPosition(); }


protected:

    GUI_ScrollArea* _scrollArea;
    GUI_TextArea* _textArea;

    QVariantMap _lastFocusInfo;

};

Q_DECLARE_METATYPE(GUI_TextEdit*)

/*
class GUI_WIDGET_API GUI_TextEditDecorator : public QObject {
    Q_OBJECT

public slots:

    GUI_TextEdit*
    new_GUI_TextEdit(GUI_ItemBase* parent, qreal width=200, qreal height=200)
    {
        return new GUI_TextEdit(parent, width, height);
    }
};
*/


#endif