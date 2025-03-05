

#include "RlpGui/GRID/Header.h"
#include "RlpGui/GRID/HeaderCell.h"
#include "RlpGui/GRID/View.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, Header)

GUI_GRID_Header::GUI_GRID_Header(GUI_GRID_View* view):
    GUI_GRID_Row::GUI_GRID_Row(view, view, -1, nullptr, 20),
    _colModel(view->colModel())
{
}


void
GUI_GRID_Header::onColAdded(QString colInfo)
{
    RLP_LOG_DEBUG("")

}


CoreDs_ModelDataItemPtr
GUI_GRID_Header::rowData()
{
    RLP_LOG_ERROR("Not implemented")

    // return _colModel->cols();
    return nullptr; // CoreDs_ModelDataItem();
}


QVariant
GUI_GRID_Header::modelValue(QString colName)
{
    // RLP_LOG_DEBUG(colName)
    
    return _colModel->colType(colName)->displayName();
}


bool
GUI_GRID_Header::hasModelValue(QString colName)
{
    return _colModel->hasColType(colName);
}


QString
GUI_GRID_Header::modelMimeType(QString key)
{
    return QString("text/grid.colname");
}


bool
GUI_GRID_Header::hasModelMimeType(QString key)
{
    return true;
}


GUI_GRID_Cell*
GUI_GRID_Header::makeCell(QString colName)
{
    // RLP_LOG_DEBUG(colName)
    return new GUI_GRID_HeaderCell(this, colName);
}


QRectF
GUI_GRID_Header::boundingRect() const
{
    return QRectF(0, 0, _view->colModel()->colWidth(), height());
}


