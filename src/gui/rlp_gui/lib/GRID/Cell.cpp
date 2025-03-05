

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/GRID/Cell.h"
#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/ColModel.h"
#include "RlpGui/GRID/View.h"


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

    _col = _row->view()->colModel()->colType(colName);

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

        }
    }
    
    
    
    setWidth(_col->width());
    setHeight(30);

    // RLP_LOG_DEBUG("Width:" << width() << "height:" << height())

    if (_col == nullptr) {
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

    if (_row->hasModelValue(colName)) {

        // TODO FIXME TEMP FOR Entity
        //
        QVariant val = _row->modelValue(colName);

        // RLP_LOG_DEBUG("Got value" << colName << "=" << val)

        /*
        if (QString(val.typeName()) == "QUuid") {
            
            QVariant eInfo = _row->view()->getEntityByUuid(val.toString());
            _cellData = _cellData.insert(eInfo.toMap());
            
        }
        */
        _cellData.insert(
            "value",
            val
        );

        // RLP_LOG_DEBUG(_row)

        if (_row->hasModelMimeType(colName))
        {
            _cellData.insert(
                "mimeType",
                _row->modelMimeType(colName)
            );
        }
        


    } else
    {
        // RLP_LOG_WARN("Row has no value:" << colName)
    }
    
    
    

    // RLP_LOG_DEBUG(_cellData)

    
    // int xpos = _cellData.value("xpos").toInt();
    // qInfo() << "cell: " << colName << " " << xpos;
    
    setPos(_col->xpos(), 0);

    
    _fmt = _row->view()
               ->colModel()
               ->formatter(colName);

    

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
    if (_row->isExpanded()) {
        _row->collapseRow();

    } else {
        _row->expandRow();

    }
    
}


void
GUI_GRID_Cell::onHeightChanged()
{
    // RLP_LOG_DEBUG(height())

     _row->setBaseHeight(height());
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

    // RLP_LOG_DEBUG("DRAG STARTED")

    if (!_cellData.contains("mimeType"))
    {
        RLP_LOG_WARN("No mimetype, cannot start Drag and Drop")
        return;
    }

    QDrag* drag = new QDrag(this);

    GUI_ItemBase* di = _fmt->makeDragItem(this);

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

    // PY_Interp::acquireGIL();

    if (_inHoverEdit)
    {
        qreal xpos = event->position().x();
        qreal w = width();

        if (((w - xpos) < 50) && ((w - xpos) > 30))
        {
            // RLP_LOG_DEBUG("COPY TO CLIPBOARD")
            QClipboard* cb = QApplication::clipboard();
            cb->setText(_fmt->getValue(this));

        } else
        {
            _inEdit = true;
            update();

            emit editRequested(this);

        }

    } else
    {
        GridSelectionMode selMode = GRID_SEL_REPLACE;
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            selMode = GRID_SEL_APPEND;
        }
        else if (event->modifiers().testFlag(Qt::AltModifier))
        {
            selMode = GRID_SEL_REMOVE;
        }

        emit selected(_colName, selMode);
    }
    
    
    // for refresh (???)
    // _col->setWidth(_col->width());
}





void
GUI_GRID_Cell::mouseReleaseEvent(QMouseEvent* event)
{
}


void
GUI_GRID_Cell::hoverEnterEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG("")

    // PY_Interp::acquireGIL();

    _inHover = true;
    _row->setHover(true);

    update();
    _row->update();
}


void
GUI_GRID_Cell::hoverMoveEvent(QHoverEvent* event)
{
    // RLP_LOG_DEBUG(_inHover)
    _row->setHover(true);


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

    update();
    _row->update();
}


QRectF
GUI_GRID_Cell::boundingRect() const
{
    // return QRectF(1, 0, _col->width(), _row->height());
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

    // RLP_LOG_DEBUG("pos:" << position() << "size:" << width() << height())

    // QRectF br = boundingRect();
    //QRectF brOutline = br.adjusted(-1, 0, 1, 1);
    //painter->setClipRect(brOutline, Qt::IntersectClip);

    QPen op(Qt::black);
    // op.setWidth(1);
    painter->setPen(op);
    
    // painter->setBrush(Qt::red);
    painter->strokeRect(boundingRect());

    if (_cfType == CF_HEADER) {

        _fmt->paintHeaderCell(
            this,
            painter
        );

    } else if (_cfType == CF_DATA) {

        _fmt->paintCell(
            this,
            painter
        );

    }
}