

#include "RlpGui/GRID/CellWidgets.h"
#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/View.h"
#include "RlpGui/GRID/ColModel.h"

#include "RlpGui/MENU/MenuItem.h"

#include "RlpGui/BASE/Scene.h"



RLP_SETUP_LOGGER(gui, GUI_GRID, CellChoiceWidget)
RLP_SETUP_LOGGER(gui, GUI_GRID, CellHoverButtonWidget)

GUI_GRID_CellWidget::GUI_GRID_CellWidget(GUI_GRID_Cell* cell):
    GUI_ItemBase(cell),
    _cell(cell)
{
}


void
GUI_GRID_CellWidget::onParentSizeChanged(qreal width, qreal height)
{
    setWidth(width);
    setHeight(height);
}


GUI_GRID_CellChoiceWidget::GUI_GRID_CellChoiceWidget(GUI_GRID_Cell* cell, QVariantMap widgetInfo):
    GUI_GRID_CellWidget(cell),
    _widgetInfo(widgetInfo),
    _menuButton(nullptr),
    _menuWidget(nullptr),
    _built(false)
{
    _menuButton = new GUI_SvgButton(":feather/chevron-down.svg", this, 20);
    _menuButton->connectToMenu();
    _menuButton->setMenuXPosOffset(40);

    if (widgetInfo.contains("visible"))
    {
        bool isVis = widgetInfo.value("visible").toBool();
        if (!isVis)
        {
            _menuButton->setVisible(false);
        }
    }

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
GUI_GRID_CellChoiceWidget::buildWidgets()
{
    // RLP_LOG_DEBUG("")

    if (_built)
    {
        return;
    }


    if (_widgetInfo.contains("menu_widget"))
    {
        QString menuWidgetType = _widgetInfo.value("menu_widget").toString();
        if (menuWidgetType == "grid")
        {
            _menuWidget = new GUI_MenuItem("", _menuButton->menu()->itemWrapper());

            GUI_GRID_View* grid = new GUI_GRID_View(_menuWidget);
            connect(
                grid,
                &GUI_GRID_View::selectionChanged,
                this,
                &GUI_GRID_CellChoiceWidget::onMenuWidgetSelectionChanged
            );

            grid->setPos(10, 0);
            grid->toolbar()->setVisible(false);
            grid->toolbar()->setHeight(0);
            grid->header()->setPos(0, 0);
            grid->scrollArea()->setPos(0, grid->header()->height() + 10);

            GUI_GRID_ColModel* cm = grid->colModel();

            QVariantList colModelInfo = _widgetInfo.value("menu_widget.colmodel").toList();

            // RLP_LOG_DEBUG(colModelInfo)

            for (int ci=0; ci != colModelInfo.size(); ++ci)
            {
                QVariantMap colInfo = colModelInfo.at(ci).toMap();
                cm->addCol(colInfo);
            }
            
            cm->updateCols();
            grid->updateHeader();

            QVariantMap data;
            QVariant v;
            v.setValue(grid);
            data.insert("grid", v);
            _menuWidget->setData(data);
            
            int gwidth = _widgetInfo.value("menu_widget.width").toInt();
            int gheight = _widgetInfo.value("menu_widget.height").toInt();

            _menuWidget->setWidth(gwidth);
            _menuWidget->setHeight(gheight);

            grid->setWidth(gwidth);
            grid->setHeight(gheight);

            _menuButton->menu()->addWidgetItem(_menuWidget);
        }
    }

    _built = true;
}


void
GUI_GRID_CellChoiceWidget::onMenuWidgetSelectionChanged(QString selMode, QVariantList selInfo)
{
    RLP_LOG_DEBUG(selInfo)

    QVariantMap md;
    md.insert("sel_mode", selMode);
    md.insert("sel_info", selInfo);
    QVariant v;
    v.setValue(_menuWidget);
    md.insert("choice_menu_widget", v);
    md.insert("col_name", _cell->colName());
    md.insert("row_idx", _cell->row()->rowIdx());
    md.insert("row_data", _cell->row()->rowData()->values());

    // This is to prevent crashes related to running cleanup of the grid in a slot
    //
    QTimer::singleShot(10, this, [this, md]() {
        this->cell()->emitCellWidgetEvent("choice.grid.selection_changed", md);

    });

    
    _menuButton->menu()->setVisible(false);
}


void
GUI_GRID_CellChoiceWidget::onMenuButtonPressed(QVariantMap md)
{
    buildWidgets();

    // Adjust position if it extends beyond width of window
    if (_menuWidget != nullptr)
    {
        int windowWidth = scene()->view()->width();
        int menuWidth = _menuWidget->width();
        QPointF menuPos = _menuButton->menu()->position();
        int fxpos = menuWidth + menuPos.x();

        if (fxpos > windowWidth)
        {
            int xpos = windowWidth - menuWidth - 20;
            _menuButton->menu()->setPos(xpos, menuPos.y());
        }
    }


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
GUI_GRID_CellChoiceWidget::onParentSizeChanged(qreal width, qreal height)
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

