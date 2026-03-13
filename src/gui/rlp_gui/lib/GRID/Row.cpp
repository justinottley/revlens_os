
#include "RlpGui/BASE/Style.h"

#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/View.h"
#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/ReorderCell.h"
#include "RlpGui/GRID/RowFactory.h"

RLP_SETUP_LOGGER(gui, GUI_GRID, Row)

GUI_GRID_Row::GUI_GRID_Row(
    GUI_ItemBase* parent,
    GUI_GRID_View* view,
    int rowIdx,
    GUI_GRID_Row* rowParent,
    int height):
        GUI_ItemBase(parent),
        _view(view),
        _rowParent(rowParent),
        _rowIdx(rowIdx),
        _evenRow(rowIdx % 2 == 0),
        _baseHeight(height),
        _expanded(false),
        _populated(false),
        _treeDepth(0),
        _rowData(nullptr),
        _mimeTypeKey(""),
        _selected(false),
        _inHover(false),
        _filteredOut(false),
        _bgCol(Qt::transparent),
        _highlightCol(Qt::transparent)
{
    _widgets.clear();

    connect(
        view,
        &GUI_GRID_View::rowSelected,
        this,
        &GUI_GRID_Row::onSiblingRowSelected
    );

    connect(
        view->colModel(),
        &GUI_GRID_ColModel::gridWidthChanged,
        this,
        &GUI_GRID_Row::onGridWidthChanged
    );

    setHeight(_baseHeight);
    setWidth(view->width());
}


GUI_GRID_Row::~GUI_GRID_Row()
{
    clear();
}


void
GUI_GRID_Row::addRowWidget(QString name, GUI_GRID_RowWidget* widget)
{
    widget->setParentItem(this);
    _widgets.insert(name, widget);
}


void
GUI_GRID_Row::setRowData(CoreDs_ModelDataItemPtr val)
{
    // RLP_LOG_DEBUG("")

    _rowData = val;
    _mimeTypeKey = val->mimeTypeKey();
    _mimeData = val->mimeData();

    // RLP_LOG_DEBUG(_mimeData)

    if (val->containsItem("__widgets__"))
    {
        CoreDs_ModelDataItemPtr widgets = val->item("__widgets__");
        QVariantMap allWidgets = widgets->values();
        
        GUI_GRID_RowFactory* rf = GUI_GRID_RowFactory::instance();

        QVariantMap::iterator wit;
        for (wit = allWidgets.begin(); wit != allWidgets.end(); ++wit)
        {
            QString widgetName = wit.key();
            QVariantMap widgetInfo = wit.value().toMap();

            rf->initRowWidget(this, widgetName, widgetInfo);
        }

    }


    connect(
        _rowData.get(),
        &CoreDs_ModelDataItem::populated,
        this,
        &GUI_GRID_Row::onDataPopulated
    );
    // RLP_LOG_DEBUG(_rowData->values())
}


bool
GUI_GRID_Row::setFilterString(QString filterStr, QVariantList colFilterList)
{
    // RLP_LOG_DEBUG(filterStr << colFilterList)

    bool rowFilterFound = false;
    for (int i=0; i != _rowList.size(); ++i)
    {
        GUI_GRID_RowPtr row = _rowList.at(i);

        bool riFilterFound = _view->updateRowVisibilityForFilter(row, filterStr, colFilterList);
        if (riFilterFound)
        {
            rowFilterFound = true;
        }
    }

    updateRows();

    return rowFilterFound;
}


void
GUI_GRID_Row::setPopulated(bool isPopulated)
{
    // RLP_LOG_DEBUG(isPopulated)

    _populated = isPopulated;
}


void
GUI_GRID_Row::setBgColour(QColor col)
{
    _bgCol = col; 
    _bgHoverCol = QColor(col.red() + 10, col.green() + 10, col.blue() + 10);
    update();
}


QString
GUI_GRID_Row::modelMimeType(QString key)
{
    if (_mimeTypeKey != "")
    {
        // RLP_LOG_DEBUG("using mime type key" << _mimeTypeKey)
        return _rowData->mimeType(_mimeTypeKey);
    }

    return _rowData->mimeType(key);
}


bool
GUI_GRID_Row::hasModelMimeType(QString key)
{
    // RLP_LOG_DEBUG(key << "mimeTypeKey:" << _mimeTypeKey)
    return (_mimeTypeKey != "" || _rowData->hasMimeType(key));
}


bool
GUI_GRID_Row::hasChildren()
{
    if (_rowData == nullptr)
    {
        return false;
    }

    // RLP_LOG_DEBUG(_rowData->value("name"))

    return (_rowData->containsItem("__children__"));
}


void
GUI_GRID_Row::updateCells(QStringList colNames)
{
    // RLP_LOG_DEBUG(colNames << colNames.size())

    clear();

    for (int i=0; i != colNames.size(); ++i)
    {
        QString colName = colNames.at(i);
        GUI_GRID_Cell* c = makeCell(colName);
        
        _cells.insert(colName, c);
    }

    // RLP_LOG_DEBUG("Done")
}


GUI_GRID_Cell*
GUI_GRID_Row::makeCell(QString colName)
{
    // RLP_LOG_DEBUG(colName)

    GUI_GRID_Cell* cell = nullptr;
    if (colName == GUI_GRID_ReorderCell::COL_NAME)
    {
        cell = new GUI_GRID_ReorderCell(this);
    } else
    {
        cell = new GUI_GRID_Cell(this, colName);
    }


    connect(
        cell,
        &GUI_GRID_Cell::selected,
        this,
        &GUI_GRID_Row::onCellSelected
    );

    connect(
        cell,
        &GUI_GRID_Cell::editRequested,
        _view,
        &GUI_GRID_View::onCellEditRequested
    );

    return cell;

}


GUI_GRID_Cell*
GUI_GRID_Row::cell(QString colName)
{
    return _cells.value(colName);
}


void
GUI_GRID_Row::clear()
{
    _rowList.clear();
    _cells.clear();

    // NOTE: CANNOT DELETE CHILDREN, ROW WIDGETS WILL GET DESTRUCTED?
    // QList<QQuickItem*> citems = childItems();

    // for (int i=0; i != citems.size(); ++i)
    // {
    //     delete citems.at(i);
    // }
}



GUI_GRID_ColType*
GUI_GRID_Row::colType(QString key)
{
    return _view->colModel()->colType(key);
}



// --------
// TreeGrid
// --------


void
GUI_GRID_Row::onDataPopulated()
{
    // RLP_LOG_DEBUG("")

    if (!_rowData->containsItem("__children__"))
    {
        return;
    }

    CoreDs_ModelDataItemPtr childModelData = _rowData->item("__children__");

    // RLP_LOG_DEBUG("POPULATE:" << childModelData->size())

    qreal ypos = pos().y();

    for (int i=0; i != childModelData->size(); ++i) {

        GUI_GRID_RowPtr r(new GUI_GRID_Row(
            _view->scrollArea()->content(),
            _view,
            i,
            this
        ));

        r->setRowData(childModelData->at(i));

        r->setTreeDepth(treeDepth() + 1);


        connect(
            r.get(),
            &GUI_GRID_Row::rowExpanded,
            _view,
            &GUI_GRID_View::onRowExpanded
        );

        connect(
            r.get(),
            &GUI_GRID_Row::rowCollapsed,
            _view,
            &GUI_GRID_View::onRowCollapsed
        );

        connect(
            this,
            &GUI_GRID_Row::visibleChanged,
            r.get(),
            &GUI_GRID_Row::onParentVisibleChanged
        );

        ypos += 30;

        r->setPos(
            0,
            ypos
        );

        r->updateCells(_view->colModel()->colNames());
        _rowList.append(r);
    }

    if (_view->currFilterStr() != "")
    {
        setFilterString(_view->currFilterStr(), _view->currColFilterList());
    }

    emit rowExpanded(_rowIdx);
}


void
GUI_GRID_Row::expandRow()
{
    RLP_LOG_DEBUG("")

    _expanded = true;

    if (!isPopulated())
    {

        _rowData->populate();
        _populated = true;

    } else
    {
        for (int ri=0; ri!=_rowList.size(); ++ri)
        {
            _rowList.at(ri)->setVisible(true);
        }
        
    }

    if (_view->currFilterStr() != "")
    {
        setFilterString(_view->currFilterStr(), _view->currColFilterList());
    }

    emit rowExpanded(_rowIdx);
}


void
GUI_GRID_Row::expandRowRecursive()
{
    // RLP_LOG_DEBUG("")

    if (!isPopulated())
    {
        _rowData->populate();
        _populated = true;
    }

    _expanded = true;

    for (int ri=0; ri!=_rowList.size(); ++ri)
    {
        _rowList.at(ri)->expandRowRecursive();
        _rowList.at(ri)->setVisible(true);
    }

    emit rowExpandRequested();
    emit rowExpanded(_rowIdx);
}


void
GUI_GRID_Row::collapseRow()
{
    // RLP_LOG_DEBUG("")
    collapseRowRecursive(rowListPtr());


    _expanded = false;

    emit rowCollapsed(_rowIdx);
}


void
GUI_GRID_Row::collapseRowRecursive(QList<GUI_GRID_RowPtr>* rowList)
{
    RLP_LOG_DEBUG("")

    for (int ri=0; ri != rowList->size(); ++ri)
    {
        if (rowList->at(ri)->isExpanded())
        {
            collapseRowRecursive(rowList->at(ri)->rowListPtr());
        }
        rowList->at(ri)->setVisible(false);
    }
}


void
GUI_GRID_Row::onParentVisibleChanged()
{
    // RLP_LOG_DEBUG("")

    if (!_filteredOut)
    {
        setVisible(_rowParent->isVisible());
    }
}


qreal
GUI_GRID_Row::rowHeight()
{
    qreal heightTotal = height();

    if (_expanded)
    {
        for (int i=0; i != _rowList.size(); ++i)
        {
            if (_rowList.at(i)->isVisible())
            {
                heightTotal += _rowList.at(i)->rowHeight();
            }
        }
    }
    
    // RLP_LOG_DEBUG(_rowData.value("name") << " " << heightTotal);

    return heightTotal;
}


void
GUI_GRID_Row::setItemHeight(qreal height)
{
    setHeight(height);

    CellIterator it;
    for (it = _cells.begin(); it != _cells.end(); ++it)
    {
        it.value()->setHeight(height);
    }
}


void
GUI_GRID_Row::setRowSelected(bool selected, int selMode)
{
    setSelected(selected);

    GridSelectionMode sMode = (GridSelectionMode)selMode;
    onCellSelected("", sMode);
}


void
GUI_GRID_Row::updateRows()
{
    // RLP_LOG_DEBUG("")

    qreal ypos = pos().y() + height();

    if (_expanded)
    {
        bool isEvenRow = true;
        for (int ri=0; ri != _rowList.size(); ++ri)
        {
            GUI_GRID_RowPtr childRow = _rowList.at(ri);
            if (childRow->isVisible())
            {
                // RLP_LOG_DEBUG(childRow->modelValue("path").toString() << "IS VISIBLE")

                childRow->setPos(0, ypos);
                childRow->setEvenRow(isEvenRow);
                ypos += childRow->rowHeight();
                isEvenRow = !isEvenRow;
            }
            _rowList.at(ri)->updateRows();
        }
    } else
    {
        for (int ri=0; ri != _rowList.size(); ++ri)
        {
            _rowList.at(ri)->setVisible(false);
        }
    }
}


void
GUI_GRID_Row::onCellSelected(QString colName, GridSelectionMode selMode)
{
    // RLP_LOG_DEBUG(colName << (int)selMode)

    _selected = true;
    if (selMode == GRID_SEL_REPLACE)
    {
        _view->setSelection(_rowData, _rowIdx);
    }
    else if ((selMode == GRID_SEL_APPEND) || (selMode == GRID_SEL_APPEND_ALL))
    {
        _view->updateSelection(_rowData, _rowIdx, selMode, /* emitSignal = */ true);
    }
    else if (selMode == GRID_SEL_REMOVE)
    {
        _view->removeSelection(_rowData, _rowIdx);
    }

    update();
}



void
GUI_GRID_Row::onGridWidthChanged(qreal width)
{
    setWidth(width);
}


void
GUI_GRID_Row::onSiblingRowSelected(CoreDs_ModelDataItemPtr data)
{
    
    if (data != _rowData)
    {
        _selected = false;
    }
    update();
}


void
GUI_GRID_Row::mousePressEvent(QMouseEvent* event)
{
    RLP_LOG_DEBUG("")
}


QRectF
GUI_GRID_Row::boundingRect() const
{
    return QRectF(0, 0, _view->colModel()->colWidth(), height());
}


void
GUI_GRID_Row::paint(GUI_Painter* painter)
{
    painter->setPen(Qt::transparent);

    if (_bgCol != Qt::transparent)
    {
        if (_inHover)
        {
            painter->setBrush(_bgHoverCol);
        } else
        {
            painter->setBrush(_bgCol);
        }

        painter->drawRect(boundingRect());
        return;
    }


    if (_selected)
    {
        painter->setBrush(GUI_Style::BgGrey);
        painter->drawRect(boundingRect());
    }

    else if (_inHover)
    {
        painter->setBrush(GUI_Style::BgLoMidGrey);
        painter->drawRect(boundingRect());

    } else
    {
        if (_evenRow)
        {
            painter->setBrush(GUI_Style::BgRowEven);
        } else
        {
            painter->setBrush(GUI_Style::BgRowOdd);
        }

        painter->drawRect(boundingRect());
    }


    if (_highlightCol != Qt::transparent)
    {
        QPen p(_highlightCol);
        p.setWidth(6);
        painter->setPen(p);
        painter->strokeRect(boundingRect());
    }
}
