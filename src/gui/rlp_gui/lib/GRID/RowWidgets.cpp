
#include "RlpGui/GRID/RowWidgets.h"
#include "RlpGui/GRID/Row.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, RowWidget)
RLP_SETUP_LOGGER(gui, GUI_GRID, RowWidgetPlugin)

GUI_GRID_RowWidget::GUI_GRID_RowWidget(GUI_GRID_Row* row):
    GUI_ItemBase(row),
    _row(row)
{
    
}


void
GUI_GRID_RowWidget::onParentSizeChanged(qreal width, qreal height)
{
    setWidth(width);
    setHeight(height);
}


// ----


GUI_GRID_RowWidgetPlugin::GUI_GRID_RowWidgetPlugin(QString name):
    _name(name)
{
}

