//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_ROW_EXPANDER_ITEM_H
#define GUI_GRID_ROW_EXPANDER_ITEM_H

#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/FormatterBase.h"
#include "RlpGui/GRID/ColType.h"

class GUI_GRID_API GUI_GRID_RowExpanderItem : public GUI_ItemBase {
    Q_OBJECT

signals:
    void triggered();

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_RowExpanderItem(GUI_ItemBase* parent);

    void mousePressEvent(QMouseEvent* event);

    QRectF boundingRect() const;

    virtual void paint(GUI_Painter* painter);

public slots:

    int size() { return _size; }
    void setExpanded(bool expanded) { _expanded = expanded; }

private:
    QPolygon _polyCollapsed;
    QPolygon _polyExpanded;

    QImage _imgCollapsed;
    QImage _imgExpanded;

    bool _expanded;
    int _size;
    
};

#endif