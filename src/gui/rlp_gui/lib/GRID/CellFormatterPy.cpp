
#include "RlpGui/GRID/CellFormatterPy.h"
#include "RlpGui/GRID/Cell.h"

#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/View.h"

#include "RlpGui/BASE/Painter.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, CellFormatterPy)

GUI_GRID_CellFormatterPy::GUI_GRID_CellFormatterPy(py::object callback):
    GUI_GRID_FormatterBase(),
    _callback(callback)
{
}

void
GUI_GRID_CellFormatterPy::paintCell(GUI_GRID_Cell* cell, GUI_Painter* painter)
{
    if (painter->getPainterType() == GUI_Painter::PAINTER_TYPE_QNANOPAINTER)
    {
        py::object pycell = py::cast(cell);
        py::object pypainter = py::cast(static_cast<GUI_NanoPainter*>(painter));

        _callback(pycell, pypainter);
    }
}