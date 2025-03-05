//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_FORMATTER_FOLDER_H
#define GUI_GRID_FORMATTER_FOLDER_H


#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/FormatterBase.h"
#include "RlpGui/GRID/CellFormatterStr.h"

class GUI_GRID_Cell;

class GUI_GRID_API GUI_GRID_CellFormatterFolder : public GUI_GRID_FormatterBase {
    Q_OBJECT


public:

    GUI_GRID_CellFormatterFolder();

    GUI_ItemBase* makeDragItem(GUI_GRID_Cell* cell);
    virtual void paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter);


protected:
    QImage _image;
    GUI_GRID_CellFormatterStr* _fmtText;

};

#endif