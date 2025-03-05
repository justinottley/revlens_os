//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_HEADER_CELL_H
#define GUI_GRID_HEADER_CELL_H

#include "RlpGui/GRID/Global.h"

#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/ColSplitter.h"

class GUI_GRID_Row;

class GUI_GRID_API GUI_GRID_HeaderCell : public GUI_GRID_Cell {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER
    
    GUI_GRID_HeaderCell(GUI_GRID_Row* header, QString colName);

    
    virtual void mouseMoveEvent(QMouseEvent* event);

    // void dragEnterEvent(QDragEnterEvent* event) override;
    // void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent *event);

    virtual void paint(GUI_Painter* painter);
    
    

public slots:

    void onSplitterPosChanged(int pos);


private:
    GUI_GRID_ColSplitter* _splitter;
};

#endif