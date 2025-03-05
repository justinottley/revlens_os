#ifndef REVLENS_GUI_TL2_ZOOMBAR_DRAGBUTTON_H
#define REVLENS_GUI_TL2_ZOOMBAR_DRAGBUTTON_H

#include "RlpRevlens/GUI_TL2/Global.h"

class GUI_TL2_ZoomBar;

class REVLENS_GUI_TL2_API GUI_TL2_ZoomBarDragButton : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

public:

    enum ButtonSide {
        BTN_NONE,
        BTN_LEFT,
        BTN_RIGHT
    };

    GUI_TL2_ZoomBarDragButton(ButtonSide side, GUI_TL2_ZoomBar* zoomBar);
    
    static const int BUTTON_SIZE;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void refresh();

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

    
    // QPointF pos() const;

public slots:

    ButtonSide side() { return _side; }

    void setXPos(int xpos); // access from python

    qreal xposOffset();
    qreal xposMax();
    void setXPosDelta(qreal delta) { _xposDelta = delta; }

signals:

    void buttonMoved(int side, float xpos);

private:

    ButtonSide _side;
    GUI_TL2_ZoomBar* _zoomBar;

    bool _inDrag;
    qreal _dragPos;
    qreal _dragPosG;

    qreal _xposDelta;
    // float _leftOffset;
    
    // int _xpos;
    // int _xposOffset; // fixed offset for placing button - left or right

};

#endif
