
#ifndef GUI_GRID_PY_FORMATTER_H
#define GUI_GRID_PY_FORMATTER_H


#include "RlpGui/GRID/Global.h"
#include "RlpGui/GRID/FormatterBase.h"

#include "RlpCore/PY/Interp.h"

class GUI_GRID_Cell;

class GUI_GRID_API GUI_GRID_CellFormatterPy : public GUI_GRID_FormatterBase {
    Q_OBJECT

public:
    RLP_DEFINE_LOGGER

    GUI_GRID_CellFormatterPy(py::object callback);

    void paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter);

public slots:

    static GUI_GRID_CellFormatterPy* new_GUI_GRID_CellFormatterPy(py::object callback)
    {
        return new GUI_GRID_CellFormatterPy(callback);
    }


private:
    py::object _callback;


};

#endif
