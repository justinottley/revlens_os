

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/ColModel.h"
#include "RlpGui/GRID/View.h"

#include "RlpGui/GRID/CellWidgets.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, Cell)

GUI_GRID_Cell::GUI_GRID_Cell(
    GUI_GRID_Row* row,
    QString colName,
    ColFormatterType cfType):
        GUI_ItemBase::GUI_ItemBase(row),
        _row(row),
        _cfType(cfType),
        _colName(colName),
        _ei(nullptr),
        _inHover(false),
        _inHoverEdit(false),
        _inEdit(false),
        _doIndent(false)
{
    // RLP_LOG_DEBUG("")
    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemClipsChildrenToShape, true);
    setFlag(QQuickItem::ItemAcceptsDrops);

    connect(
        this,
        &GUI_GRID_Cell::heightChanged,
        this,
        &GUI_GRID_Cell::onHeightChanged
    );

    connect(
        this,
        &GUI_GRID_Cell::cellWidgetEvent,
        row,
        &GUI_GRID_Row::cellWidgetEvent
    );


    _col = _row->view()->colModel()->colType(colName);

    setWidth(_col->width());
    setHeight(30);

    if (_col->hasMetadataValue("expander_visible"))
    {
        _doIndent = true;
        if (row->hasChildren())
        {
            _doIndent = true;

            _ei = new GUI_GRID_RowExpanderItem(this);
            _ei->setPos(5 + (_row->treeDepth() * 15), 3);

            connect(
                _ei,
                &GUI_GRID_RowExpanderItem::triggered,
                this,
                &GUI_GRID_Cell::onRowExpanderTriggered
            );

            connect(
                _row,
                &GUI_GRID_Row::rowExpandRequested,
                _ei,
                &GUI_GRID_RowExpanderItem::onRowExpandRequested
            );

        }
    }


    // TODO FIXME: plugin style dispatch mechanism
    //
    if ((cfType == CF_DATA) && (_col->hasMetadataValue("widgets")))
    {
        QVariantMap colWidgetInfo = _col->metadata().value("widgets").toMap();
        QVariantMap::iterator it;
        for (it = colWidgetInfo.begin(); it != colWidgetInfo.end(); ++it)
        {
            QString widgetName = it.key();
            QVariantMap widgetInfo = it.value().toMap();

            if (widgetName == "choice")
            {
                GUI_GRID_CellChoiceWidget* ccw = new GUI_GRID_CellChoiceWidget(this, widgetInfo);

                // For type conversion through python binding
                QVariant v;
                v.setValue(ccw);
                _widgets.insert("choice", v);
            }

            else if (widgetName == "hover_button")
            {
                GUI_GRID_CellHoverButtonWidget* chbw = new GUI_GRID_CellHoverButtonWidget(this, widgetInfo);

                QVariantMap rowData = row->rowData()->values();
                chbw->button()->updateMetadata(rowData);
                
                if (widgetInfo.contains("tooltip"))
                {
                    chbw->button()->setToolTipText(widgetInfo.value("tooltip").toString());
                }
                connect(
                    chbw->button(),
                    &GUI_SvgButton::hoverChanged,
                    this,
                    &GUI_GRID_Cell::onHoverButtonHoverChanged
                );

                connect(
                    chbw->button(),
                    &GUI_SvgButton::buttonPressed,
                    this,
                    &GUI_GRID_Cell::onHoverButtonPressed
                );


                QVariant v;
                v.setValue(chbw);
                _widgets.insert("hover_button", v);
            }
        }
    }


    // RLP_LOG_DEBUG("Width:" << width() << "height:" << height())

    if (_col == nullptr)
    {
        RLP_LOG_ERROR("Could not get col")
        Q_ASSERT(false);
    }

    
    connect(
        _col,
        &GUI_GRID_ColType::xposChanged,
        this,
        &GUI_GRID_Cell::onColXPosChanged
    );

    connect(
        _col,
        &GUI_GRID_ColType::widthChanged,
        this,
        &GUI_GRID_Cell::onColWidthChanged
    );

    connect(
        _col,
        &GUI_GRID_ColType::visibleChanged,
        this,
        &GUI_GRID_Cell::onColVisibleChanged
    );

    // RLP_LOG_DEBUG(_row)

    if (_row->hasModelValue(colName))
    {
        QVariant val = _row->modelValue(colName);
        _cellData.insert(
            "value",
            val
        );

    } else
    {
        // RLP_LOG_WARN("Row has no value:" << colName)
    }
    
    if (_row->hasModelMimeType(colName))
    {
        _cellData.insert(
            "mimeType",
            _row->modelMimeType(colName)
        );
    }

    setPos(_col->xpos(), 0);

    _fmt = _row->view()
               ->colModel()
               ->formatter(colName);
}


GUI_GRID_Cell::~GUI_GRID_Cell()
{
    // RLP_LOG_DEBUG("~GUI_GRID_Cell()")
    _widgets.clear();
}


void
GUI_GRID_Cell::addWidget(QString name, GUI_ItemBase* widget)
{
    RLP_LOG_DEBUG(name << widget)
    // widget->setPos(0, 0);
    widget->setParentItem(this);

    if (widget->isPyMode())
    {
        // prevent garbage collection from python
        py::object pobj = py::cast(widget);
        pobj.inc_ref();
    }
    
    QVariant v;
    v.setValue(widget);

    _widgets.insert(name, v);

}


void
GUI_GRID_Cell::onColWidthChanged(QString colName, qreal width)
{
    // RLP_LOG_DEBUG(width)

    setWidth(width);
}



void
GUI_GRID_Cell::onColXPosChanged(QString colName, qreal xpos)
{
    // RLP_LOG_DEBUG(colName << " " << xpos)

    setPos(xpos, 0);
}


void
GUI_GRID_Cell::onColVisibleChanged(bool isVisible)
{
    // RLP_LOG_DEBUG(isVisible)

    setVisible(isVisible);
}


void
GUI_GRID_Cell::onRowExpanderTriggered()
{
    if (_row->isExpanded())
    {
        _row->collapseRow();

    } else
    {
        _row->expandRow();

    }
    
}


void
GUI_GRID_Cell::onHeightChanged()
{
    // RLP_LOG_DEBUG(height())

     _row->setBaseHeight(height());
}


void
GUI_GRID_Cell::onHoverButtonHoverChanged(bool val)
{
    // RLP_LOG_DEBUG(val)

    if (!val)
    {
        GUI_GRID_CellHoverButtonWidget* cw = _widgets.value("hover_button").value<GUI_GRID_CellHoverButtonWidget*>();
        if (!_inHover)
        {
            cw->button()->setVisible(false);
        }
    }
}


void
GUI_GRID_Cell::onHoverButtonPressed(QVariantMap md)
{
    // RLP_LOG_DEBUG(md)

    // Why doesn't the Cell widgetEvent signal work !??!?
    emit _row->view()->cellWidgetEvent("hover_button.button_pressed", md);
}


int
GUI_GRID_Cell::treeDepth()
{
    return _row->treeDepth();
}


bool
GUI_GRID_Cell::hasChildren()
{
    return _row->hasChildren();
}


bool
GUI_GRID_Cell::inHoverEdit()
{
    if (!_row->view()->cellsEditable())
    {
        return false;
    }

    return _inHoverEdit;
}


bool
GUI_GRID_Cell::inEdit()
{
    if (!_row->view()->cellsEditable())
    {
        return false;
    }

    return _inEdit;
}


void
GUI_GRID_Cell::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")
    if (!dragStarted(event))
        return;

    // We're in a drag - if this row is not selected,
    // the drag started without having the row selected first
    // replace the selection with this row before continuing
    //
    if (!_row->isSelected())
    {
        emit selected(_colName, GRID_SEL_REPLACE);
    }

    // RLP_LOG_DEBUG("DRAG STARTED")

    if (!_cellData.contains("mimeType"))
    {
        RLP_LOG_WARN("No mimetype, cannot start Drag and Drop")
        return;
    }

    QDrag* drag = new QDrag(this);

    GUI_ItemBase* di = _fmt->makeDragItem(this);
    if (di == nullptr)
    {
        RLP_LOG_ERROR("Invalid drag item")
        return;
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(
        _cellData.value("mimeType").toString(),
        di->metadataValue("mimeData").toByteArray()
    );

    // RLP_LOG_DEBUG(_cellData.value("mimeType").toString() << " --> " << QString(di->metadataValue("mimeData").toByteArray()))
    drag->setMimeData(mimeData);


    GUI_Scene* s = scene();
    s->addItem(di);
    s->setDragItem(di);

    QPointF pos = event->position();
    QPointF gpos = mapToItem(s, pos);

    di->setPos(gpos.x(), gpos.y());

    Qt::DropAction da = drag->exec();

    // RLP_LOG_DEBUG("Drop action: " << da);

    s->clearDragItem();

}


void
GUI_GRID_Cell::mousePressEvent(QMouseEvent *event)
{
    _pressPos = event->pos();

    if (_inHoverEdit)
    {
        qreal xpos = event->position().x();
        qreal w = width();

        if (((w - xpos) < 50) && ((w - xpos) > 30))
        {
            QClipboard* cb = QApplication::clipboard();
            cb->setText(_fmt->getValue(this));

        } else
        {
            _inEdit = true;
            update();

            emit editRequested(this);

        }

    } else if (_cfType == CF_DATA)
    {
        QPointF scenePos = event->scenePosition();
        QVariantMap md;

        md.insert("button", (int)event->button());
        md.insert("row_idx", _row->rowIdx());
        md.insert("col_name", _colName);
        md.insert("data", _cellData);
        md.insert("row_data", _row->rowData()->values());
        md.insert("scene_pos.x", scenePos.x());
        md.insert("scene_pos.y", scenePos.y());
        md.insert("in_item_select", _inItemSelect);

        _row->view()->cellWidgetEvent("cell.button_pressed", md);
    }
}


void
GUI_GRID_Cell::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return;
    }

    GridSelectionMode selMode = GRID_SEL_REPLACE;
    if (event->modifiers().testFlag(Qt::ShiftModifier))
    {
        selMode = GRID_SEL_APPEND_ALL;
    }
    else if (event->modifiers().testFlag(Qt::ControlModifier))
    {
        if (_row->isSelected())
        {
            selMode = GRID_SEL_REMOVE;
        } else
        {
            selMode = GRID_SEL_APPEND;
        }
    }

    emit selected(_colName, selMode);
}


void
GUI_GRID_Cell::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = true;
    _row->setHover(true);

    if (_widgets.contains("hover_button"))
    {
        GUI_GRID_CellHoverButtonWidget* cw = _widgets.value("hover_button").value<GUI_GRID_CellHoverButtonWidget*>();
        cw->cellHoverEnterEvent(event);
    }

    update();
    _row->update();
}


void
GUI_GRID_Cell::hoverMoveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(_inHover)
    _row->setHover(true);

    if (_fmt)
    {
        _fmt->hoverMoveEventInternal(this, event->position());
    }


    // NOTE: DISABLE THIS CHECK FOR NOW
    return;

    qreal xpos = event->position().x();
    qreal w = width();

    // RLP_LOG_DEBUG(xpos << w)

    bool newHE = false;
    if ((w - xpos) < 50)
    {
        newHE = true;
    }


    if (_inHoverEdit != newHE)
    {
        update();
    }

    _inHoverEdit = newHE;
    
}


void
GUI_GRID_Cell::hoverLeaveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    _inHover = false;
    _inHoverEdit = false;
    _row->setHover(false);

    if (_widgets.contains("hover_button"))
    {
        GUI_GRID_CellHoverButtonWidget* cw = _widgets.value("hover_button").value<GUI_GRID_CellHoverButtonWidget*>();
        cw->cellHoverLeaveEvent(event);
    }

    update();
    _row->update();
}


QRectF
GUI_GRID_Cell::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_GRID_Cell::paint(GUI_Painter* painter)
{

    if (!_fmt)
    {
        RLP_LOG_ERROR(_colName << " - NO FORMATTER")
        return;
    }


    QPen op(Qt::black);
    painter->setPen(op);
    painter->strokeRect(boundingRect());

    if (_cfType == CF_HEADER)
    {
        _fmt->paintHeaderCell(
            this,
            painter
        );

    } else if (_cfType == CF_DATA)
    {
        _fmt->paintCell(
            this,
            painter
        );

    }
}