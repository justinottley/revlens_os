//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_FORMATTER_DATE_H
#define GUI_GRID_FORMATTER_DATE_H


#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/CellFormatterStr.h"

class GUI_GRID_Cell;

class GUI_GRID_API GUI_GRID_CellFormatterDate : public GUI_GRID_CellFormatterStr {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_CellFormatterDate(QString keyName="value", int xoffset=0, int yoffset=5);

    void paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter);


};

#endif