
#include "RlpGui/GRID/Toolbar.h"
#include "RlpGui/BASE/Style.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, Toolbar)
RLP_SETUP_LOGGER(gui, GUI_GRID, ToolbarInit)

GUI_GRID_ToolbarInit::GUI_GRID_ToolbarInit():
    _pycallback(py::none())
{
}


void
GUI_GRID_ToolbarInit::init(GUI_GRID_Toolbar* toolbar)
{
    RLP_LOG_DEBUG("")

    if (!_pycallback.is(py::none()))
    {
        py::object pytb = py::cast(toolbar);
        _pycallback(pytb);
    } else
    {
        GUI_HLayout* layout = toolbar->layout();
        layout->addItem(toolbar->colSelector());
    }
}


GUI_GRID_Toolbar::GUI_GRID_Toolbar(GUI_ItemBase* parent, GUI_GRID_ToolbarInit* tbi):
    GUI_ItemBase(parent)
{
    setHeight(30);

    _layout = new GUI_HLayout(this);
    _layout->setSpacing(10);

    _colSelector = new GUI_IconButton("", this, 20);
    _colSelector->setPos(0, 5);
    _colSelector->setText("Fields");
    _colSelector->setOutlined(true);
    _colSelector->setSvgIconPath(":/feather/chevron-down.svg", 20);

    if (tbi != nullptr)
    {
        tbi->init(this);
    } else
    {
        _layout->addItem(_colSelector);
    }
}


QRectF
GUI_GRID_Toolbar::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_GRID_Toolbar::paint(GUI_Painter* painter)
{
}

