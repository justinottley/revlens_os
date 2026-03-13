

#include "RlpGui/GRID/ReorderCell.h"
#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/View.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/FrameOverlay.h"

#include "RlpGui/WIDGET/Label.h"

#include "RlpCore/UTIL/Text.h"


const QString GUI_GRID_ReorderCell::COL_NAME = "_reorder";

RLP_SETUP_LOGGER(gui, GUI_GRID, ReorderHeaderCell)
RLP_SETUP_LOGGER(gui, GUI_GRID, ReorderCell)


GUI_GRID_ReorderHeaderCell::GUI_GRID_ReorderHeaderCell(GUI_GRID_Row* header):
    GUI_GRID_HeaderCell(header, GUI_GRID_ReorderCell::COL_NAME)
{
}


void
GUI_GRID_ReorderHeaderCell::paint(GUI_Painter* painter)
{
    painter->setPen(Qt::black);
    painter->setBrush(QBrush(GUI_Style::BgLoMidGrey));
    painter->drawRect(boundingRect());
}


GUI_GRID_ReorderCell::GUI_GRID_ReorderCell(GUI_GRID_Row* row):
    GUI_GRID_Cell(row, GUI_GRID_ReorderCell::COL_NAME),
    _inDragReorder(false)
{
    setZ(10);
}


void
GUI_GRID_ReorderCell::hoverEnterEvent(QHoverEvent* event)
{
    setCursor(Qt::OpenHandCursor);
}


void
GUI_GRID_ReorderCell::hoverLeaveEvent(QHoverEvent* event)
{
    setCursor(Qt::ArrowCursor);
}


void
GUI_GRID_ReorderCell::mouseMoveEvent(QMouseEvent* event)
{
    // RLP_LOG_DEBUG("")

    if (!dragStarted(event))
        return;


    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();

    // RLP_LOG_DEBUG("colname:" << _colName)

    mimeData->setData("text/grid.row_idx", QByteArray(QString::number(_row->rowIdx()).toUtf8()));
    drag->setMimeData(mimeData);

    // TODO FIXME: "name" hardcoded
    QString lbl = _row->modelValue("name").toString();
    int tw = UTIL_Text::getWidth(lbl);

    GUI_Scene* s = scene();
    GUI_FrameOverlay* di = new GUI_FrameOverlay(tw + 10, 30, nullptr);
    di->setColour(GUI_Style::BgDarkGrey);
    di->setOpacity(0.9);

    GUI_Label* l = new GUI_Label(di, lbl);

    // RLP_LOG_DEBUG(lbl)

    s->addItem(di);
    s->setDragItem(di);

    // di->setPos(event->globalPosition().x(), event->globalPosition().y());
    QPointF pos = event->position();
    QPointF gpos = mapToItem(s, pos);

    di->setPos(gpos.x(), gpos.y());

    Qt::DropAction dropAction = drag->exec(); // Qt::CopyAction | Qt::MoveAction);

    RLP_LOG_DEBUG("Drop action: " << dropAction);

    if (dropAction == Qt::MoveAction)
    {
        QVariantMap reorderInfo = di->metadataValue("grid.reorder_info").toMap();
        emit _row->view()->cellWidgetEvent("row.reorder_requested", reorderInfo);
    }

    s->clearDragItem();

    update();
}


void
GUI_GRID_ReorderCell::mouseReleaseEvent(QMouseEvent* event)
{

}


void
GUI_GRID_ReorderCell::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/grid.row_idx"))
    {
        event->setAccepted(true);
        _inDragReorder = true;

    } else
    {
        _inDragReorder = false;
    }

    update();

}


void
GUI_GRID_ReorderCell::dragLeaveEvent(QDragLeaveEvent* event)
{
    _inDragReorder = false;
    update();
}



void
GUI_GRID_ReorderCell::dropEvent(QDropEvent *event)
{
    _inDragReorder = false;

    if (event->mimeData()->hasFormat("text/grid.row_idx"))
    {
        int srcRowIdx = QString(event->mimeData()->data("text/grid.row_idx")).toInt();

        if (srcRowIdx == _row->rowIdx())
        {
            RLP_LOG_DEBUG("same position for reorder, doing nothing")
            return;
        }

        event->setAccepted(true);
        event->setDropAction(Qt::MoveAction);

        QVariantMap md;
        md.insert("src_idx", srcRowIdx);
        md.insert("dest_idx", _row->rowIdx());
        
        // There are some event loop object lifecycle issues happening that prevent
        // safely emitting a signal here. Return from the drag drop first, then emit signal
        GUI_ItemBase* di = scene()->dragItem();
        di->setMetadata("grid.reorder_info", md);
    }
}


void
GUI_GRID_ReorderCell::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    QPen op(Qt::black);
    painter->setPen(op);
    painter->strokeRect(boundingRect());


    int startYPos = (height() / 2) - 5;
    for (int i=0; i != 3; ++i)
    {
        int lineYPos = (i * 5) + startYPos;
        painter->drawLine(7, lineYPos, width() - 7, lineYPos);
    }

    if (_inDragReorder)
    {
        QRectF br(2, 2, width() - 4, 10);
        painter->setPen(GUI_Style::Highlight);
        painter->drawRect(br);
    }
}