//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_BOOKEND_DRAGBUTTON_H
#define REVLENS_GUI_TL2_BOOKEND_DRAGBUTTON_H


#include "RlpRevlens/GUI_TL2/Global.h"

class GUI_TL2_Bookends;

class REVLENS_GUI_TL2_API GUI_TL2_BookendDragButton : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    enum ButtonSide {
        BTN_NONE,
        BTN_LEFT,
        BTN_RIGHT
    };

    static const int BUTTON_WIDTH;

    GUI_TL2_BookendDragButton(ButtonSide side, GUI_TL2_Bookends* bookends);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void onParentSizeChanged(qreal width, qreal height);

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);

public slots:


    qreal xpos() const;
    void setXPos(int xpos) { _xpos = xpos; }

    qreal getPosAtFrame(qlonglong frame);
    qlonglong posFrame() { return _posFrame; }
    void setPosFrame(qlonglong frame);

    QColor colStart();
    QColor colEnd();
    float colStartPos();
    float colEndPos();

signals:

    void buttonFrameChanged(int side, qlonglong frame);
    
private:

    ButtonSide _side;
    GUI_TL2_Bookends* _bookends;

    QColor _col;
    bool _hover;

    QColor _colOffStart;
    QColor _colOffEnd;

    QColor _colOnStart;
    QColor _colOnEnd;

    QColor _colStart;
    QColor _colEnd;

    bool _inDrag;
    qlonglong _posFrame;

    qreal _xpos;
};

#endif