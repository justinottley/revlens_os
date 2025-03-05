
#include "RlpGui/BASE/Scene.h"

#include "RlpGui/GRID/ColSplitter.h"
#include "RlpGui/GRID/Cell.h"



RLP_SETUP_LOGGER(gui, GUI_GRID, ColSplitter)

static const int COLSPLITTER_WIDTH = 10;
GUI_GRID_ColSplitter::GUI_GRID_ColSplitter(GUI_ItemBase* parent):
    _inDrag(false),
    _inHover(false),
    GUI_ItemBase(parent)
{
    setAcceptHoverEvents(true);
    setWidth(COLSPLITTER_WIDTH);
    setHeight(parent->height());
    _cell = qobject_cast<GUI_GRID_Cell*>(parent);
}


void
GUI_GRID_ColSplitter::mousePressEvent(QMouseEvent *event)
{
    RLP_LOG_DEBUG("");

    _inDrag = true;
    _dragPosX = event->globalPosition().x();
    _dragPos = pos().x();
}


void
GUI_GRID_ColSplitter::mouseMoveEvent(QMouseEvent *event)
{
    // RLP_LOG_DEBUG(event->pos().x());

    if (_inDrag) {

        // RLP_LOG_INFO(event->pos().x());
        
        int newPos = _dragPos + (event->globalPosition().x() - _dragPosX);
        setPos(newPos, 0);

        _cell->colType()->setWidth(newPos + COLSPLITTER_WIDTH);
    }
    
}


void
GUI_GRID_ColSplitter::mouseReleaseEvent(QMouseEvent *event)
{
    RLP_LOG_DEBUG("");

    _inDrag = false;
}


void
GUI_GRID_ColSplitter::hoverEnterEvent(QHoverEvent* event)
{
    RLP_LOG_DEBUG("")

    _inHover = true;

    update();
    // qobject_cast<GUI_Scene*>(scene())->requestUpdate();
}


void
GUI_GRID_ColSplitter::hoverLeaveEvent(QHoverEvent* event)
{
    RLP_LOG_DEBUG("")

    _inHover = false;

    update();

    // qobject_cast<GUI_Scene*>(scene())->requestUpdate();
}


QRectF
GUI_GRID_ColSplitter::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}


void
GUI_GRID_ColSplitter::paint(GUI_Painter* painter)
{
    // RLP_LOG_DEBUG("")

    if (_inHover) {
        painter->setPen(Qt::red);
        painter->setBrush(Qt::red);
        painter->drawRect(boundingRect());
    }


    
}
