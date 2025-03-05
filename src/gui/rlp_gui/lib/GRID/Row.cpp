
#include "RlpGui/BASE/Style.h"

#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/View.h"
#include "RlpGui/GRID/Cell.h"

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
        _selected(false),
        _inHover(false)
{
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
GUI_GRID_Row::setRowData(CoreDs_ModelDataItemPtr val)
{
    // RLP_LOG_DEBUG("")

    _rowData = val;
    connect(
        _rowData.get(),
        &CoreDs_ModelDataItem::populated,
        this,
        &GUI_GRID_Row::onDataPopulated
    );
    // RLP_LOG_DEBUG(_rowData->values())
}



bool
GUI_GRID_Row::hasChildren()
{
    if (_rowData == nullptr) {
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


    for (int i=0; i != colNames.size(); ++i) {
        
        QString colName = colNames.at(i);

        // RLP_LOG_DEBUG("makeCell:" << colName)
        GUI_GRID_Cell* c = makeCell(colName);
        
        _cells.insert(colName, c);
    }

    // RLP_LOG_DEBUG("Done")
}


GUI_GRID_Cell*
GUI_GRID_Row::makeCell(QString colName)
{
    // RLP_LOG_DEBUG(colName)

    GUI_GRID_Cell* cell = new GUI_GRID_Cell(this, colName);

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
    // qInfo() << "clear";

    _rowList.clear();
    _cells.clear();

    QList<QQuickItem*> citems = childItems();

    for (int i=0; i != citems.size(); ++i) {
        // qInfo() << "deleting child item";

        delete citems.at(i);
    }
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
    RLP_LOG_DEBUG("")

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

    emit rowExpanded(_rowIdx);
}


void
GUI_GRID_Row::expandRow()
{
    RLP_LOG_DEBUG("")

    _expanded = true;

    
    if (!isPopulated()) {

        _rowData->populate();
        _populated = true;

    } else {

        for (int ri=0; ri!=_rowList.size(); ++ri) {
            _rowList.at(ri)->setVisible(true);
        }
        
    }

    
    
    emit rowExpanded(_rowIdx);
}


void
GUI_GRID_Row::collapseRow()
{
    RLP_LOG_DEBUG("")

    for (int ri=0; ri != _rowList.size(); ++ri)
    {
        _rowList.at(ri)->setVisible(false);
    }

    _expanded = false;
    
    emit rowCollapsed(_rowIdx);
}



void
GUI_GRID_Row::onParentVisibleChanged()
{
    // RLP_LOG_DEBUG("")

    setVisible(_rowParent->isVisible());
}


qreal
GUI_GRID_Row::rowHeight()
{
    qreal heightTotal = height();

    if (_expanded) {
        for (int i=0; i != _rowList.size(); ++i) {
            heightTotal += _rowList.at(i)->rowHeight();
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
GUI_GRID_Row::updateRows()
{
    // RLP_LOG_DEBUG("")

    qreal ypos = pos().y() + height();

    if (_expanded) {
        for (int ri=0; ri != _rowList.size(); ++ri) {

            _rowList.at(ri)->setPos(0, ypos);
            _rowList.at(ri)->updateRows();

            ypos += _rowList.at(ri)->rowHeight();
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
    else if (selMode == GRID_SEL_APPEND)
    {
        _view->updateSelection(_rowData, _rowIdx);
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
    return QRectF(0, 0, width(), height()); // _view->colModel()->colWidth(), height());
}


void
GUI_GRID_Row::paint(GUI_Painter* painter)
{
    painter->setPen(Qt::transparent);

    if (_selected)
    {
        painter->setBrush(GUI_Style::BgMidGrey);
        painter->drawRect(boundingRect());
    }

    else if (_inHover)
    {
        painter->setBrush(GUI_Style::BgLoMidGrey);
        painter->drawRect(boundingRect());

    } else if (_evenRow)
    {
        painter->setBrush(GUI_Style::BgRowEven);
        painter->drawRect(boundingRect());
    }

}
