
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
        GUI_GRID_Cell(header, colName, CF_HEADER)
{
    setAcceptHoverEvents(true);

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
    RLP_LOG_DEBUG(pos << " width: " << width() << " position: " <<position().x())

    int newWidth = pos + 5;

    RLP_LOG_DEBUG("new width: " << newWidth)

    _col->setWidth(pos + 5);

}

    
void
GUI_GRID_HeaderCell::mouseMoveEvent(QMouseEvent* event)
{
    RLP_LOG_DEBUG("")

    if (!dragStarted(event))
        return;

    /*
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if ((event->pos() - _pressPos).manhattanLength()
         < QApplication::startDragDistance())
        return;
    */

    // RLP_LOG_INFO("IN DRAG");
    // RLP_LOG_INFO(_cellData);

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();

    RLP_LOG_DEBUG(_colName)

    mimeData->setData("text/grid.colname", QByteArray(_colName.toUtf8()));
    drag->setMimeData(mimeData);

    QString lbl = colType()->displayName();
    int tw = UTIL_Text::getWidth(lbl);

    GUI_Scene* s = scene();
    GUI_FrameOverlay* di = new GUI_FrameOverlay(tw + 10, 30, nullptr);
    di->setColour(GUI_Style::BgDarkGrey);
    di->setOpacity(0.9);

    GUI_Label* l = new GUI_Label(di, lbl);

    s->addItem(di);
    s->setDragItem(di);

    di->setPos(event->globalPosition().x(), event->globalPosition().y());

    Qt::DropAction dropAction = drag->exec(); // Qt::CopyAction | Qt::MoveAction);

    RLP_LOG_DEBUG("Drop action: " << dropAction);
    s->clearDragItem();

}


/*

void
GUI_GRID_HeaderCell::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/grid.colname"))
    {
        event->setAccepted(true);

        RLP_LOG_DEBUG(event->mimeData()->text() << _colName)

    }
}


void
GUI_GRID_HeaderCell::dragMoveEvent(QDragMoveEvent* event)
{
    RLP_LOG_DEBUG(event->source())
    if (event->mimeData()->hasFormat("text/grid.colname"))
    {
        event->setAccepted(true);

        GUI_Scene* s = scene();
        GUI_ItemBase* di = s->dragItem();
        if (di != nullptr)
        {
            QPointF pos = event->position();
            QPointF gpos = mapToItem(s, pos);

            di->setPos(gpos.x(), gpos.y());
        }
        
    }
}
*/

void
GUI_GRID_HeaderCell::dropEvent(QDropEvent *event)
{
    RLP_LOG_DEBUG("")

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

    } else {
        GUI_GRID_HeaderCell::dropEvent(event);
    }
}


void
GUI_GRID_HeaderCell::paint(GUI_Painter* painter)
{
    
    if (_inHover) {
        painter->setBrush(GUI_Style::BgLightGrey);

    } else {
        painter->setBrush(GUI_Style::BgMidGrey);
    }

    GUI_GRID_Cell::paint(painter);
}
    