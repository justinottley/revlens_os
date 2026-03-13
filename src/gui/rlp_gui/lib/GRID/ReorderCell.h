
#ifndef GUI_GRID_REORDER_CELL_H
#define GUI_GRID_REORDER_CELL_H

#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/HeaderCell.h"

class GUI_GRID_Row;


class GUI_GRID_API GUI_GRID_ReorderHeaderCell : public GUI_GRID_HeaderCell {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_ReorderHeaderCell(GUI_GRID_Row* header);

    void dragEnterEvent(QDragEnterEvent* event) {} // not supported for reorder col

    void paint(GUI_Painter* painter);
};


class GUI_GRID_API GUI_GRID_ReorderCell : public GUI_GRID_Cell {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    static const QString COL_NAME;

    GUI_GRID_ReorderCell(GUI_GRID_Row* row);

    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void hoverEnterEvent(QHoverEvent* event);
    void hoverLeaveEvent(QHoverEvent* event);

    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

    void dropEvent(QDropEvent *event);

    void paint(GUI_Painter* painter);

private:
    bool _inDragReorder;

};

#endif
