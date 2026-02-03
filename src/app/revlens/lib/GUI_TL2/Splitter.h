//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//
#ifndef REVLENS_GUI_TL2_SPLITTER_H
#define REVLENS_GUI_TL2_SPLITTER_H

#include "RlpRevlens/GUI_TL2/Global.h"


class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_Splitter : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_Splitter(GUI_ItemBase* view);

    static const int BUTTON_WIDTH;

    void onParentSizeChanged(qreal nwidth, qreal nheight);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);
    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);


    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);


signals:

    void splitterMoved(int xpos);

private:

    GUI_TL2_View* _view;

    bool _inDrag;
    qreal _dragPos;
    qreal _dragPosG;
    
    bool _hover;
};

#endif
