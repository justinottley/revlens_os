

#include "RlpGui/GRID/View.h"
#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, View)

GUI_GRID_View::GUI_GRID_View(GUI_ItemBase* parent, GUI_GRID_ToolbarInit* tbi):
    GUI_ItemBase(parent),
    _data(nullptr),
    _autoResize(true),
    _editable(false),
    _selected(nullptr),
    _selectedRow(-1),
    _endSelected(nullptr),
    _endSelectedRow(-1),
    _currFilterStr("")
{
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);

    _fullSelection.clear();
    _selectedRows.clear();

    _scrollArea = new GUI_ScrollArea(this);
    _scrollArea->setOutlined(false);
    _scrollArea->hhandle()->setAutoScroll(false);
    
    _colModel = new GUI_GRID_ColModel();

    _toolbar = new GUI_GRID_Toolbar(this, tbi);

    _header = new GUI_GRID_Header(this);
    _header->setPos(0, _toolbar->height() + 10);

    _scrollArea->setPos(0, _toolbar->height() + _header->height() + 20);
    _scrollArea->setHeightOffset(_toolbar->height() + _header->height() + 20);
    _scrollArea->setVHandlePosOffset(0);


    connect(
        _scrollArea,
        &GUI_ScrollArea::contentPosChanged,
        this,
        &GUI_GRID_View::onContentPosChanged
    );

    connect(
        _colModel,
        &GUI_GRID_ColModel::colAdded,
        this,
        &GUI_GRID_View::onColAdded
    );

    connect(
        _colModel,
        &GUI_GRID_ColModel::gridWidthChanged,
        this,
        &GUI_GRID_View::onGridWidthChanged
    );

    connect(
        qobject_cast<GUI_MenuPane*>(_toolbar->colSelector()->menu()),
        &GUI_MenuPane::menuItemSelected,
        this,
        &GUI_GRID_View::onColToggled
    );
}


void
GUI_GRID_View::setModelData(CoreDs_ModelDataItemPtr data)
{
    // RLP_LOG_DEBUG(data->size())
    
    clearData();

    _data = data;

    _scrollArea->resetPosition();


    for (int i=0; i != data->size(); ++i)
    {

        GUI_GRID_RowPtr r(new GUI_GRID_Row(_scrollArea->content(), this, i));

        connect(
            r.get(),
            &GUI_GRID_Row::rowExpanded,
            this,
            &GUI_GRID_View::onRowExpanded
        );

        
        connect(
            r.get(),
            &GUI_GRID_Row::rowCollapsed,
            this,
            &GUI_GRID_View::onRowCollapsed
        );

        connect(
            r.get(),
            &GUI_GRID_Row::cellWidgetEvent,
            this,
            &GUI_GRID_View::cellWidgetEvent
        );

        // RLP_LOG_DEBUG(data->at(i)->values())

        r->setRowData(data->at(i));

        r->setPos(
            0,
            (i * 30)
        );

        r->updateCells(_colModel->colNames());


        _rowList.push_back(r);

    }

    updateRows();

    emit modelDataChanged();
}


void
GUI_GRID_View::setModelDataList(QVariantList data)
{
    // RLP_LOG_DEBUG("")

    CoreDs_ModelDataItemPtr modelData = CoreDs_ModelDataItem::makeItem();

    for (int i=0; i != data.size(); ++i) {

        CoreDs_ModelDataItemPtr row = CoreDs_ModelDataItem::makeItem();
        QVariantMap r = data.at(i).toMap();

        QVariantMap::iterator rit;
        for (rit = r.begin(); rit != r.end(); ++rit)
        {
            QVariant val = rit.value();
            row->insert(rit.key(), val);
        }

        modelData->append(row);

    }

    setModelData(modelData);

}


GUI_GRID_Row*
GUI_GRID_View::rowAt(int idx)
{
    // RLP_LOG_DEBUG(idx)

    if ((idx >= 0) && (idx < _rowList.size()))
    {
        GUI_GRID_RowPtr r = _rowList.at(idx);
        if (r)
        {
            return r.get();
        }
    }

    RLP_LOG_ERROR("No row at" << idx << "total rows:" << _rowList.size())
    return nullptr;
}


void
GUI_GRID_View::onColAdded(QString colName)
{
    // RLP_LOG_DEBUG(colName)

    GUI_GRID_ColType* col = _colModel->colType(colName);

    QVariantMap colData = col->toMap();
    QString displayName = colData.value("display_name").toString();

    GUI_MenuItem* item = qobject_cast<GUI_MenuPane*>(_toolbar->colSelector()->menu())->addItem(
        displayName, colData, true);
    item->setChecked(true);

    setWidth(col->xpos() + col->width());
}


void
GUI_GRID_View::onColToggled(QVariantMap sigData)
{
    QVariantMap colInfo = sigData.value("data").toMap();

    // RLP_LOG_INFO(colInfo)

    QString colName = colInfo.value("name").toString();

    // RLP_LOG_INFO(colName)

    GUI_GRID_ColType* col = _colModel->colType(colName);

    col->setVisible(!col->isVisible());

    _colModel->updateCols();
}


bool
GUI_GRID_View::setColVisible(QString colName, bool isVisible)
{
    GUI_GRID_ColType* col = _colModel->colType(colName);
    bool result = false;

    if (col != nullptr)
    {
        GUI_MenuPane* mp = qobject_cast<GUI_MenuPane*>(_toolbar->colSelector()->menu());
        GUI_MenuItem* mi = mp->item(col->displayName());
        if (mi != nullptr)
        {
            if (((isVisible) && (!mi->isChecked())) ||
                (!isVisible) && (mi->isChecked()))
            {
                mi->selectItem();
                result = true;
            }
            
        }
    }

    return result;
}


void
GUI_GRID_View::onRowExpanded(int idx)
{
    // RLP_LOG_DEBUG(idx);

    updateRows();
}


void
GUI_GRID_View::onRowCollapsed(int idx)
{
    // RLP_LOG_DEBUG(idx)

    updateRows();
}


CoreDs_ModelDataItemPtr
GUI_GRID_View::selection()
{
    RLP_LOG_WARN("DEPRECATED")
    return _selected;
}


QVariantMap
GUI_GRID_View::selectionValues()
{
    RLP_LOG_WARN("DEPRECATED")

    return _selected->values();
}


void
GUI_GRID_View::setSelection(CoreDs_ModelDataItemPtr data, int rowIdx)
{
    RLP_LOG_DEBUG(rowIdx)

    _selected = data;
    _selectedRow = rowIdx;

    updateSelection(data, rowIdx);

    update();

    emit rowSelected(data);

    QVariantList slist = {data->values()};
    emit selectionChanged("set", slist);
}


void
GUI_GRID_View::updateSelection(CoreDs_ModelDataItemPtr data, int rowIdx, bool emitSignal)
{
    // RLP_LOG_DEBUG(rowIdx)

    _fullSelection.clear();

    for (int i=0; i != _selectedRows.size(); ++i)
    {
        _selectedRows.at(i)->setSelected(false);
    }
    _selectedRows.clear();

    ModelItemList result;

    int startRow = _selectedRow;
    int endRow = rowIdx;

    if (startRow > endRow)
    {
        startRow = rowIdx;
        endRow = _selectedRow;
    }

    for (int i=startRow; i <= endRow; ++i)
    {
        GUI_GRID_Row* row = rowAt(i);
        if (row != nullptr)
        {
            row->setSelected(true);
            result.append(_data->at(i));
            _selectedRows.push_back(_rowList.at(i));
        } else
        {
            RLP_LOG_ERROR("Invalid row at" << i << "cannot add to selection")
        }
    }

    _fullSelection = result;

    if (emitSignal)
    {
        QVariantList rsel;
        for (int i=0; i != _fullSelection.size(); ++i)
        {
            rsel.append(_fullSelection.at(i)->values());
        }

        emit selectionChanged("set", rsel);
    }
}


void
GUI_GRID_View::removeSelection(CoreDs_ModelDataItemPtr data, int rowIdx)
{
    ModelItemList result;

    for (int i =0; i != _fullSelection.size(); ++i)
    {
        if (_fullSelection.at(i).get() != data.get())
        {
            result.append(_fullSelection.at(i));
        }
    }

    _fullSelection = result;

    GUI_GRID_Row* removeRow = rowAt(rowIdx);
    removeRow->setSelected(false);

    QList<GUI_GRID_RowPtr> selRows;

    for (int ri = 0; ri != _selectedRows.size(); ri++)
    {
        if (_selectedRows.at(ri).get() != removeRow)
        {
            selRows.append(_selectedRows.at(ri));
        }
    }

    _selectedRows = selRows;

    QVariantList remResult = {data->values()};
    emit selectionChanged("rem", remResult);
}


QVariantList
GUI_GRID_View::fullSelectionValues()
{
    QVariantList result;
    for (int i = 0; i != _fullSelection.size(); ++i)
    {
        result.push_back(_fullSelection.at(i)->values());
    }

    return result;
}


QVariantList
GUI_GRID_View::fullSelectionRows()
{
    QVariantList result;
    for (int i = 0; i != _selectedRows.size(); ++i)
    {
        result.push_back(_selectedRows.at(i)->rowIdx());
    }
    return result;
}


void
GUI_GRID_View::clearSelection()
{
    RLP_LOG_DEBUG("")

    _fullSelection.clear();
    _selectedRows.clear();

    for (int ri=0; ri != _rowList.size(); ++ri)
    {
        {
            _rowList.at(ri)->setSelected(false);
        }
    }

    update();

    QVariantList emptyList;
    emit selectionChanged("set", emptyList);
}


void
GUI_GRID_View::updateRows(bool force)
{
    _colModel->updateCols();

    qreal ypos = 0;
    
    for (int ri=0; ri != _rowList.size(); ++ri)
    {
        // RLP_LOG_DEBUG(ri << _rowList.at(ri).get())

        if (force || (_rowList.at(ri)->isVisible()))
        {
            _rowList.at(ri)->setPos(0, ypos);
            ypos += _rowList.at(ri)->rowHeight();
        }

        _rowList.at(ri)->updateRows();
    }

    _scrollArea->setMaxChildHeight(ypos);
}


void
GUI_GRID_View::updateHeader()
{
    // RLP_LOG_DEBUG("")

    _header->updateCells(_colModel->colNames());
    
}


void
GUI_GRID_View::clearData()
{
    // RLP_LOG_DEBUG("")

    for (int ri=0; ri != _rowList.size(); ++ri)
    {
        _rowList.at(ri)->setVisible(false);
        _rowList.at(ri)->deleteLater();
    }

    _rowList.clear();
    if (_data != nullptr)
    {
        _data->clear();
    }
    
    _selectedRows.clear();
}


bool
GUI_GRID_View::updateRowVisibilityForFilter(GUI_GRID_RowPtr row, QString filterStr, QVariantList colFilterList)
{
    bool filterFound = false;
    for (int cf=0; cf != colFilterList.size(); ++cf)
    {
        QString colName = colFilterList.at(cf).toString();
        QString val = row->modelValue(colName).toString();
        if ((filterStr == "") || (val.toLower().contains(filterStr.toLower())))
        {
            filterFound = true;
            break;
        }
    }

    if (filterFound)
    {
        row->setVisible(true);
        row->setFilteredOut(false);
    } else
    {
        row->setVisible(false);
        row->setFilteredOut(true);
    }

    return filterFound;
}


void
GUI_GRID_View::expandRowRecursive()
{
    // RLP_LOG_DEBUG("")

    for (int ri=0; ri!=_rowList.size(); ++ri)
    {
        _rowList.at(ri)->expandRowRecursive();
        _rowList.at(ri)->setVisible(true);
    }

    updateRows();
}


void
GUI_GRID_View::setFilterString(QString filterStr, QVariantList colFilterList)
{
    // RLP_LOG_DEBUG(filterStr << colFilterList)

    _currFilterStr = filterStr;
    _currColFilterList = colFilterList;

    bool rowFilterFound = false;
    for (int i=0; i != _rowList.size(); ++i)
    {
        GUI_GRID_RowPtr row = _rowList.at(i);
        rowFilterFound = updateRowVisibilityForFilter(row, filterStr, colFilterList);
        if (row->isExpanded())
        {
            bool subRowFilterFound = row->setFilterString(filterStr, colFilterList);
            if (subRowFilterFound)
            {
                rowFilterFound = true;
                row->setVisible(true);
            }
        }
    }

    updateRows();

}


void
GUI_GRID_View::onCellEditRequested(GUI_GRID_Cell* cell)
{
    // RLP_LOG_DEBUG(cell);
    QVariantMap cellEditInfo;
    QVariant cv;
    cv.setValue(cell);

    QPointF pos = cell->mapToItem(this, QPointF(0, 0));

    cellEditInfo.insert("cell", cv);
    cellEditInfo.insert("pos", pos);

    emit cellEditRequested(cellEditInfo);
}


void
GUI_GRID_View::onGridWidthChanged(qreal gridWidth)
{
    qreal parentWidth = parentItem()->width();

    // RLP_LOG_DEBUG("incoming:" << gridWidth << "parent:" << parentWidth << "colModel->colWidth():" << _colModel->colWidth())

    _scrollArea->setMaxChildWidth(_colModel->colWidth());

    if ((gridWidth + 30) > parentWidth)
    {
        // RLP_LOG_DEBUG("CLIPPING, too big:" << gridWidth)
        return;
    }

    // If toolbar is wider than incoming width, clip up to
    // toolbar width so toolbar doesn't clip
    qreal toolbarWidth = _toolbar->layout()->width();
    if (gridWidth < toolbarWidth)
    {
        gridWidth = toolbarWidth;
    }

    // RLP_LOG_DEBUG(gridWidth)

    _scrollArea->setWidth(gridWidth);
    setWidth(gridWidth);
}


void
GUI_GRID_View::onContentPosChanged(qreal xpos, qreal ypos)
{
    // RLP_LOG_DEBUG(xpos << ypos)

    _header->setPos(xpos, _header->pos().y());
}


void
GUI_GRID_View::onParentSizeChanged(qreal width, qreal height)
{
    if (_autoResize)
    {
        // RLP_LOG_DEBUG(width << height)

        setWidth(width);
        setHeight(height);
    }
}



void
GUI_GRID_View::onSizeChangedInternal()
{
    GUI_ItemBase::onSizeChangedInternal();

    _scrollArea->onParentSizeChanged(_colModel->colWidth(), height());
}

