//
//  Copyright (c) 2014-2022 Justin Ottley. All rights reserved.
//

#ifndef CORE_GUI_FRAMEOVERLAY_H
#define CORE_GUI_FRAMEOVERLAY_H

#include "RlpGui/BASE/Global.h"
#include "RlpGui/BASE/FrameBase.h"


class GUI_BASE_API GUI_FrameOverlay : public GUI_FrameBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_FrameOverlay(int width, int height, GUI_ItemBase* parent);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void onSizeRectChanged(QRectF rect);

    QRectF boundingRect() const;

    void paint(GUI_Painter* painter);

public slots:

    void setColour(QColor col) { _col = col; }
    void setOpacity(qreal opacity) { _opacity = opacity; }
    void setText(QString text) { _text = text; }
    void setTextPos(int x, int y) { _textX = x; _textY = y; }
    void setOutlined(bool outlined) { _outlined = outlined; }

protected:

    bool _outlined;
    qreal _opacity;
    QColor _col;
    QString _text;
    int _textX;
    int _textY;

};


#endif