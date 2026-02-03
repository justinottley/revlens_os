

#include "RlpGui/GRID/CellWidgets.h"
#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/Row.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, CellChoiceWidget)
RLP_SETUP_LOGGER(gui, GUI_GRID, CellHoverButtonWidget)

GUI_GRID_CellWidget::GUI_GRID_CellWidget(GUI_GRID_Cell* cell):
    GUI_ItemBase(cell),
    _cell(cell)
{
}


void
GUI_GRID_CellWidget::onParentSizeChanged(int width, int height)
{
    setWidth(width);
    setHeight(height);
}


GUI_GRID_CellChoiceWidget::GUI_GRID_CellChoiceWidget(GUI_GRID_Cell* cell):
    GUI_GRID_CellWidget(cell)
{
    _menuButton = new GUI_SvgButton(":feather/chevron-down.svg", this, 20);
    _menuButton->connectToMenu();
    _menuButton->setMenuXPosOffset(40);

    connect(
        _menuButton,
        &GUI_SvgButton::buttonPressed,
        this,
        &GUI_GRID_CellChoiceWidget::onMenuButtonPressed
    );

    connect(
        _menuButton->menu(),
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_GRID_CellChoiceWidget::onMenuItemSelected
    );

    onParentSizeChanged(cell->width(), cell->height());
}


void
GUI_GRID_CellChoiceWidget::onMenuButtonPressed(QVariantMap md)
{
    md.insert("col_name", _cell->colName());
    md.insert("row_idx", _cell->row()->rowIdx());
    md.insert("row_data", _cell->row()->rowData()->values());

    _cell->emitCellWidgetEvent("choice.menu_button_pressed", md);
}


void
GUI_GRID_CellChoiceWidget::onMenuItemSelected(QVariantMap md)
{
    md.insert("col_name", _cell->colName());
    md.insert("row_idx", _cell->row()->rowIdx());
    md.insert("row_data", _cell->row()->rowData()->values());

    _cell->emitCellWidgetEvent("choice.menu_item_selected", md);
}


void
GUI_GRID_CellChoiceWidget::onParentSizeChanged(int width, int height)
{
    GUI_GRID_CellWidget::onParentSizeChanged(width, height);

    _menuButton->setPos(width - 30, 6);
}


GUI_GRID_CellHoverButtonWidget::GUI_GRID_CellHoverButtonWidget(GUI_GRID_Cell* cell, QVariantMap initInfo):
    GUI_GRID_CellWidget(cell),
    _initInfo(initInfo)
{
    QString path = initInfo.value("path").toString();
    int size = initInfo.value("size").toInt();

    _button = new GUI_SvgButton(path, this, size);
    _button->setVisible(false);

    if (initInfo.contains("text"))
    {
        QString buttonText = initInfo.value("text").toString();
        _button->setOverlayText(buttonText);
        _button->setOverlayTextPos(25, 8);
    }

    onParentSizeChanged(cell->width(), cell->height());
}


void
GUI_GRID_CellHoverButtonWidget::cellHoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    if (!_cell->row()->hasChildren())
    {
        _button->setVisible(true);
    }
}


void
GUI_GRID_CellHoverButtonWidget::cellHoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    if (!_button->inHover())
    {
        _button->setVisible(false);
    }
}


void
GUI_GRID_CellHoverButtonWidget::onParentSizeChanged(int width, int height)
{
    GUI_GRID_CellWidget::onParentSizeChanged(width, height);

    _button->setPos(width - 80, 6);
}

