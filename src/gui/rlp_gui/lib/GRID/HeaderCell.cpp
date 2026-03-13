
#include "RlpGui/GRID/HeaderCell.h"

#include "RlpCore/UTIL/Text.h"

#include "RlpGui/BASE/Style.h"
#include "RlpGui/BASE/Scene.h"
#include "RlpGui/BASE/FrameOverlay.h"

#include "RlpGui/WIDGET/Label.h"

#include "RlpGui/GRID/Row.h"
#include "RlpGui/GRID/ColModel.h"
#include "RlpGui/GRID/View.h"


RLP_SETUP_LOGGER(gui, GUI_GRID, HeaderCell)

GUI_GRID_HeaderCell::GUI_GRID_HeaderCell(
    GUI_GRID_Row* header,
    QString colName):
        GUI_GRID_Cell(header, colName, CF_HEADER),
        _inDragReorder(false)
{
    _splitter = new GUI_GRID_ColSplitter(this);
    _splitter->setPos(width() - _splitter->width() - 1, 0);

    connect(
        _splitter,
        &GUI_GRID_ColSplitter::posChanged,
        this,
        &GUI_GRID_HeaderCell::onSplitterPosChanged
    );


}


void
GUI_GRID_HeaderCell::onSplitterPosChanged(int pos)
{
    // RLP_LOG_DEBUG(pos << " width: " << width() << " position: " <<position().x())

    int newWidth = pos + 5;

    // RLP_LOG_DEBUG("new width: " << newWidth)

    _col->setWidth(pos + 5);

}

    
void
GUI_GRID_HeaderCell::mouseMoveEvent(QMouseEvent* event)
{
    if (!dragStarted(event))
        return;


    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();

    // RLP_LOG_DEBUG("colname:" << _colName)

    mimeData->setData("text/grid.colname", QByteArray(_colName.toUtf8()));
    drag->setMimeData(mimeData);

    QString lbl = colType()->displayName();
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
    s->clearDragItem();

}


void
GUI_GRID_HeaderCell::dragEnterEvent(QDragEnterEvent* event)
{
    RLP_LOG_DEBUG("")

    if (event->mimeData()->hasFormat("text/grid.colname"))
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
GUI_GRID_HeaderCell::dragLeaveEvent(QDragLeaveEvent* event)
{
    _inDragReorder = false;
    update();
}


void
GUI_GRID_HeaderCell::dropEvent(QDropEvent *event)
{
    // RLP_LOG_DEBUG("")

    _inDragReorder = false;

    if (event->mimeData()->hasFormat("text/grid.colname"))
    {

        QString srcColName = QString(event->mimeData()->data("text/grid.colname"));
        QStringList colNames = _row->view()->colModel()->colNames();
        
        int srcIdx = colNames.indexOf(srcColName);
        int currIdx = colNames.indexOf(_colName);
        
        colNames.removeOne(srcColName);
        colNames.insert(currIdx, srcColName);

        _row->view()->colModel()->setColNames(colNames);
        _row->view()->colModel()->updateCols();

        event->setAccepted(true);
        event->setDropAction(Qt::MoveAction);

        update();

    } else {
        GUI_GRID_Cell::dropEvent(event);
    }
}


void
GUI_GRID_HeaderCell::paint(GUI_Painter* painter)
{

    if (_inHover)
    {
        painter->setBrush(GUI_Style::BgLightGrey);
    } else
    {
        painter->setBrush(GUI_Style::BgMidGrey);
    }

    GUI_GRID_Cell::paint(painter);

    if (_inDragReorder)
    {
        QRectF br(2, 2, 5, height() - 4);
        painter->setPen(GUI_Style::Highlight);
        painter->setBrush(QBrush(GUI_Style::Highlight));
        painter->drawRect(br);
    }
}
