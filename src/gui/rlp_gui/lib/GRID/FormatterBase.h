//
//  Copyright (c) 2014-2020 Justin Ottley. All rights reserved.
//

#ifndef GUI_GRID_FORMATTER_BASE_H
#define GUI_GRID_FORMATTER_BASE_H

#include "RlpGui/GRID/Global.h"
#include "RlpGui/BASE/SvgIcon.h"

class GUI_GRID_Row;
class GUI_GRID_Cell;

enum ColFormatterType {
    CF_HEADER,
    CF_DATA,
    CF_EDIT
};

class GUI_GRID_FormatterBase;
typedef QMap<ColFormatterType, GUI_GRID_FormatterBase*> ColFormatterMap;

class GUI_GRID_API GUI_GRID_FormatterBase : public QObject {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_FormatterBase();
    
    virtual void paintHeaderCell(GUI_GRID_Cell* cell, GUI_Painter* painter);

    /*
    {
        paintCell(cell, painter, option, widget);
    }
    */


    virtual void paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter);

    virtual GUI_ItemBase* makeDragItem(GUI_GRID_Cell* cell);

    virtual QString getValue(GUI_GRID_Cell* cell) { return ""; }


protected:
    GUI_SvgIcon* _editIcon;
    GUI_SvgIcon* _copyIcon;
};

#endif