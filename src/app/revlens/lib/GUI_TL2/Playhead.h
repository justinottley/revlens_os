//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

#ifndef REVLENS_GUI_TL2_PLAYHEAD_H
#define REVLENS_GUI_TL2_PLAYHEAD_H

#include "RlpRevlens/GUI_TL2/Global.h"

class GUI_TL2_View;

class REVLENS_GUI_TL2_API GUI_TL2_Playhead : public GUI_ItemBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_TL2_Playhead(GUI_TL2_View* view);

    void mouseMoveEventInternal(int xpos);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverMoveEvent(QHoverEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);


    void paintMousePos(GUI_Painter* painter);
    void paintViewPos(GUI_Painter* painter);

    virtual QRectF boundingRect() const;
    virtual void paint(GUI_Painter* painter);


public slots:

    void refresh();
    void onParentSizeChanged(qreal nwidth, qreal nheight);

    qlonglong currMouseFrame() { return _mouseFrame; }
    
private:

    GUI_TL2_View* _view;
    qlonglong _mouseFrame;
    // qlonglong _viewFrame;

    bool _inDrag;
};

#endif

