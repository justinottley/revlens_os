//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_CELL_FORMATTER_IMAGE_H
#define GUI_GRID_CELL_FORMATTER_IMAGE_H


#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/FormatterBase.h"

class GUI_GRID_Cell;

class GUI_GRID_API GUI_GRID_CellFormatterImage : public GUI_GRID_FormatterBase {
    Q_OBJECT


public:
    RLP_DEFINE_LOGGER
    
    GUI_GRID_CellFormatterImage(QString keyName="value");
    
    void paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter);

private:
    QImage updateImage(GUI_GRID_Cell* cell, QImage image);

protected:
    QString _keyName;

};

#endif