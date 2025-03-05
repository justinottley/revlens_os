//
//  Copyright (c) 2014-2024 Justin Ottley. All rights reserved.
//

#ifndef GUI_WIDGET_SLIDER_H
#define GUI_WIDGET_SLIDER_H

#include "RlpGui/WIDGET/Global.h"
#include "RlpGui/BASE/ItemBase.h"

class GUI_WIDGET_API GUI_Slider : public GUI_ItemBase {
    Q_OBJECT

public:
    enum Orientation {
        O_SL_HORIZONTAL,
        O_SL_VERTICAL
    };

signals:
    void startMove(float pos);
    void moving(float pos);
    void endMove(float pos);

public:
    RLP_DEFINE_LOGGER

    GUI_Slider(GUI_ItemBase* parent, Orientation orientation, int length=200, int offset=15);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void hoverMoveEvent(QHoverEvent* event);

    void paint(GUI_Painter* painter);

    float getValue(int pos);
    void setValue(int pos);

public slots:

    static GUI_Slider*
    new_GUI_Slider(GUI_ItemBase* parent, int orientation, int length)
    {
        return new GUI_Slider(parent, (Orientation)orientation, length);
    }

    void setSliderValue(float pos);

    void setBgColour(QColor col) { _bgCol = col; }

protected:

    int getXYPos();

    Orientation _orientation;
    int _length;
    int _offset;
    float _pos;
    float _circpos;

    float _mpos; // position at current mouse position

    bool _chover; // circle hover
    bool _lhover; // line hover
    int _lxpos;

    QColor _fgCol;
    QColor _bgCol;
};

#endif