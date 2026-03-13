
#ifndef RLPEXTREVLENS_RLANN_BRUSH_TEXT_PREVIEW_H
#define RLPEXTREVLENS_RLANN_BRUSH_TEXT_PREVIEW_H

#include "RlpExtrevlens/RLANN_BRUSH/Global.h"

#include "RlpGui/BASE/ItemBase.h"

class RlpAnnBrush_TextPreview : public GUI_ItemBase {
    Q_OBJECT

signals:
    void enterPressed(QVariantMap md);
    void focusChanged(RlpAnnBrush_TextPreview* tp, bool focus);

public:
    RLP_DEFINE_LOGGER

    RlpAnnBrush_TextPreview(GUI_ItemBase* parent);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void paint(GUI_Painter* painter);

public slots:

    void addText(QString text) { _text += text; }
    void setText(QString text) { _text = text; }
    QString text() { return _text; }
    float textSize();

    void setPenSettings(QVariantMap settings);
    void setPenColor(QColor col) { _penCol = col; }
    void setPenWidth(int width) { _penWidth = width; }
    void setPosition(QPointF pos) { _posInImage = pos; }
    void setImageRatio(float imgRatio) { _imgRatio = imgRatio; }

    void setHidden(bool isHidden) { _hidden = isHidden; }

private:
    QString _text;
    QPointF _posInImage; // in image coordinates
    float _imgRatio;
    QColor _penCol;
    int _penWidth;

    bool _hidden;
    bool _autoHide;
};

#endif
