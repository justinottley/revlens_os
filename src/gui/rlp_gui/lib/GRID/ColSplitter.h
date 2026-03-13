
//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_SPLITTER_H
#define GUI_GRID_SPLITTER_H

#include "RlpGui/GRID/Global.h"

class GUI_GRID_Cell;

class GUI_GRID_API GUI_GRID_ColSplitter : public GUI_ItemBase {
    Q_OBJECT

signals:

    void posChanged(int pos);


public:
    RLP_DEFINE_LOGGER

    GUI_GRID_ColSplitter(GUI_ItemBase* parent);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);
    

    virtual QRectF boundingRect() const;
    
    virtual void paint(GUI_Painter* painter);



private:
    GUI_GRID_Cell* _cell;

    bool _inDrag;
    int _dragPosX;
    int _dragPos;

    bool _inHover;

};

#endif